////////////////////////////////////////////////////////////////////////////////
/// @brief datafiles
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2014 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Dr. Frank Celler
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#include "Basics/win-utils.h"
#endif

#include "datafile.h"

#include "Basics/conversions.h"
#include "Basics/hashes.h"
#include "Basics/logging.h"
#include "Basics/memory-map.h"
#include "Basics/tri-strings.h"
#include "Basics/files.h"
#include "VocBase/server.h"


// #define DEBUG_DATAFILE 1

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief return whether the datafile is a physical file (true) or an
/// anonymous mapped region (false)
////////////////////////////////////////////////////////////////////////////////

static bool IsPhysicalDatafile (const TRI_datafile_t* const datafile) {
  return datafile->_filename != nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the name of a datafile
////////////////////////////////////////////////////////////////////////////////

static const char* GetNameDatafile (const TRI_datafile_t* const datafile) {
  if (datafile->_filename == nullptr) {
    // anonymous regions do not have a filename
    return "anonymous region";
  }

  // return name of the physical file
  return datafile->_filename;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief close a datafile
////////////////////////////////////////////////////////////////////////////////

static void CloseDatafile (TRI_datafile_t* const datafile) {
  TRI_ASSERT(datafile->_state != TRI_DF_STATE_CLOSED);

  if (datafile->isPhysical(datafile)) {
    TRI_CLOSE(datafile->_fd);
  }

  datafile->_state = TRI_DF_STATE_CLOSED;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy a datafile
////////////////////////////////////////////////////////////////////////////////

static void DestroyDatafile (TRI_datafile_t* const datafile) {
  if (datafile->_filename != nullptr) {
    TRI_FreeString(TRI_CORE_MEM_ZONE, datafile->_filename);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief sync the data of a datafile
////////////////////////////////////////////////////////////////////////////////

static bool SyncDatafile (const TRI_datafile_t* const datafile,
                          char const* begin,
                          char const* end) {
  if (datafile->_filename == nullptr) {
    // anonymous regions do not need to be synced
    return true;
  }

  TRI_ASSERT(datafile->_fd >= 0);

  if (begin == end) {
    // no need to sync
    return true;
  }

  return TRI_MSync(datafile->_fd, begin, end);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief truncate the datafile to a specific length
////////////////////////////////////////////////////////////////////////////////

static int TruncateDatafile (TRI_datafile_t* const datafile, const off_t length) {
  if (datafile->isPhysical(datafile)) {
    // only physical files can be truncated
    return ftruncate(datafile->_fd, length);
  }

  // for anonymous regions, this is a non-op
  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief calculates the actual CRC of a marker, without bounds checks
////////////////////////////////////////////////////////////////////////////////

static TRI_voc_crc_t CalculateCrcValue (TRI_df_marker_t const* marker) {
  TRI_voc_size_t zero = 0;
  off_t o = offsetof(TRI_df_marker_t, _crc);
  size_t n = sizeof(TRI_voc_crc_t);

  char const* ptr = (char const*) marker;

  TRI_voc_crc_t crc = TRI_InitialCrc32();

  crc = TRI_BlockCrc32(crc, ptr, o);
  crc = TRI_BlockCrc32(crc, (char*) &zero, n);
  crc = TRI_BlockCrc32(crc, ptr + o + n, marker->_size - o - n);

  crc = TRI_FinalCrc32(crc);

  return crc;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief diagnoses a marker
////////////////////////////////////////////////////////////////////////////////

static std::string DiagnoseMarker (TRI_df_marker_t const* marker,
                                   char const* end) {
  std::ostringstream result;

  if (marker == nullptr) {
    result << "marker is undefined. should not happen";
    return result.str();
  }
  
  // check marker type
  TRI_df_marker_type_t type = marker->_type;
  if (type <= (TRI_df_marker_type_t) TRI_MARKER_MIN) {
    // marker type is less than minimum allowed type value
    result << "marker type value (" << static_cast<int>(type) << ") is wrong. expecting value higher than " << TRI_MARKER_MIN;
    return result.str();
  }

  if (type >= (TRI_df_marker_type_t) TRI_MARKER_MAX) {
    // marker type is greater than maximum allowed type value
    result << "marker type value (" << static_cast<int>(type) << ") is wrong. expecting value less than " << TRI_MARKER_MAX;
    return result.str();
  }

  if (marker->_size >= (TRI_voc_size_t) TRI_MARKER_MAXIMAL_SIZE) {
    // a single marker bigger than 256 MB seems unreasonable
    // note: this is an arbitrary limit
    result << "marker size value (" << marker->_size << ") is wrong. expecting value less than " << TRI_MARKER_MAXIMAL_SIZE;
    return result.str();
  }

  if (marker->_size < sizeof(TRI_df_marker_t)) {
    result << "marker size is too small (" << marker->_size << "). expecting at least " << sizeof(TRI_df_marker_t) << " bytes";
    return result.str();
  }
  
  if (reinterpret_cast<char const*>(marker) + marker->_size > end) {
    result << "marker size is beyond end of datafile";
    return result.str();
  }

  TRI_voc_crc_t crc = CalculateCrcValue(marker);
    
  if (marker->_crc == crc) {
    result << "crc checksum is correct";
  }
  else {
    result << "crc checksum (hex " << std::hex << marker->_crc << ") is wrong. expecting (hex " << std::hex << crc << ")";
  }

  return result.str();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checks a CRC of a marker, with bounds checks
////////////////////////////////////////////////////////////////////////////////

static bool CheckCrcMarker (TRI_df_marker_t const* marker,
                            char const* end) {
  if (marker->_size < sizeof(TRI_df_marker_t)) {
    return false;
  }
  
  if (reinterpret_cast<char const*>(marker) + marker->_size > end) {
    return false;
  }

  auto expected = CalculateCrcValue(marker);
  return marker->_crc == expected;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a new sparse datafile
///
/// returns the file descriptor or -1 if the file cannot be created
////////////////////////////////////////////////////////////////////////////////

static int CreateSparseFile (char const* filename,
                             const TRI_voc_size_t maximalSize) {
  TRI_ERRORBUF;
  TRI_lseek_t offset;
  char zero;
  ssize_t res;
  int fd;

  // open the file
  fd = TRI_CREATE(filename, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);

  if (fd < 0) {
    TRI_SYSTEM_ERROR();

    TRI_set_errno(TRI_ERROR_SYS_ERROR);

    LOG_ERROR("cannot create datafile '%s': %s", filename, TRI_GET_ERRORBUF);
    return -1;
  }

  // create sparse file
  offset = TRI_LSEEK(fd, (TRI_lseek_t) (maximalSize - 1), SEEK_SET);

  if (offset == (TRI_lseek_t) -1) {
    TRI_SYSTEM_ERROR();
    TRI_set_errno(TRI_ERROR_SYS_ERROR);
    TRI_CLOSE(fd);

    // remove empty file
    TRI_UnlinkFile(filename);

    LOG_ERROR("cannot seek in datafile '%s': '%s'", filename, TRI_GET_ERRORBUF);
    return -1;
  }

  zero = 0;
  res = TRI_WRITE(fd, &zero, 1);

  if (res < 0) {
    TRI_SYSTEM_ERROR();
    TRI_set_errno(TRI_ERROR_SYS_ERROR);
    TRI_CLOSE(fd);

    // remove empty file
    TRI_UnlinkFile(filename);

    LOG_ERROR("cannot create sparse datafile '%s': '%s'", filename, TRI_GET_ERRORBUF);
    return -1;
  }

  return fd;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief initialises a datafile
////////////////////////////////////////////////////////////////////////////////

static void InitDatafile (TRI_datafile_t* datafile,
                          char* filename,
                          int fd,
                          void* mmHandle,
                          TRI_voc_size_t maximalSize,
                          TRI_voc_size_t currentSize,
                          TRI_voc_fid_t fid,
                          char* data) {

  // filename is a string for physical datafiles, and NULL for anonymous regions
  // fd is a positive value for physical datafiles, and -1 for anonymous regions
  if (filename == nullptr) {
    TRI_ASSERT(fd == -1);
  }
  else {
    TRI_ASSERT(fd >= 0);
  }

  datafile->_state       = TRI_DF_STATE_READ;
  datafile->_fid         = fid;

  datafile->_filename    = filename;
  datafile->_fd          = fd;
  datafile->_mmHandle    = mmHandle;

  datafile->_maximalSize = maximalSize;
  datafile->_currentSize = currentSize;
  datafile->_footerSize  = sizeof(TRI_df_footer_marker_t);

  datafile->_isSealed    = false;
  datafile->_lastError   = TRI_ERROR_NO_ERROR;

  datafile->_full        = false;

  datafile->_data        = data;
  datafile->_next        = data + currentSize;

  datafile->_synced      = data;
  datafile->_written     = nullptr;

  // reset tick aggregates
  datafile->_tickMin     = 0;
  datafile->_tickMax     = 0;
  datafile->_dataMin     = 0;
  datafile->_dataMax     = 0;

  // initialise function pointers
  datafile->isPhysical   = &IsPhysicalDatafile;
  datafile->getName      = &GetNameDatafile;
  datafile->close        = &CloseDatafile;
  datafile->destroy      = &DestroyDatafile;
  datafile->sync         = &SyncDatafile;
  datafile->truncate     = &TruncateDatafile;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief truncates a datafile
///
/// Create a truncated datafile, seal it and rename the old.
////////////////////////////////////////////////////////////////////////////////

static int TruncateAndSealDatafile (TRI_datafile_t* datafile,
                                    TRI_voc_size_t vocSize) {
  TRI_ERRORBUF;
  char* oldname;
  char zero;
  int res;
  void* data;
  void* mmHandle;

  // this function must not be called for non-physical datafiles
  TRI_ASSERT(datafile->isPhysical(datafile));

  // use multiples of page-size
  size_t maximalSize = ((vocSize + sizeof(TRI_df_footer_marker_t) + PageSize - 1) / PageSize) * PageSize;

  // sanity check
  if (sizeof(TRI_df_header_marker_t) + sizeof(TRI_df_footer_marker_t) > maximalSize) {
    LOG_ERROR("cannot create datafile '%s', maximal size '%u' is too small", datafile->getName(datafile), (unsigned int) maximalSize);
    return TRI_set_errno(TRI_ERROR_ARANGO_MAXIMAL_SIZE_TOO_SMALL);
  }

  // open the file
  char* filename = TRI_Concatenate2String(datafile->_filename, ".new");

  int fd = TRI_CREATE(filename, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);

  if (fd < 0) {
    TRI_SYSTEM_ERROR();
    LOG_ERROR("cannot create new datafile '%s': '%s'", filename, TRI_GET_ERRORBUF);
    TRI_FreeString(TRI_CORE_MEM_ZONE, filename);

    return TRI_set_errno(TRI_ERROR_SYS_ERROR);
  }

  // create sparse file
  TRI_lseek_t offset = TRI_LSEEK(fd, (TRI_lseek_t) (maximalSize - 1), SEEK_SET);

  if (offset == (TRI_lseek_t) -1) {
    TRI_SYSTEM_ERROR();
    TRI_set_errno(TRI_ERROR_SYS_ERROR);
    TRI_CLOSE(fd);

    // remove empty file
    TRI_UnlinkFile(filename);

    LOG_ERROR("cannot seek in new datafile '%s': '%s'", filename, TRI_GET_ERRORBUF);
    TRI_FreeString(TRI_CORE_MEM_ZONE, filename);

    return TRI_ERROR_SYS_ERROR;
  }

  zero = 0;
  res = TRI_WRITE(fd, &zero, 1);

  if (res < 0) {
    TRI_SYSTEM_ERROR();
    TRI_set_errno(TRI_ERROR_SYS_ERROR);
    TRI_CLOSE(fd);

    // remove empty file
    TRI_UnlinkFile(filename);

    LOG_ERROR("cannot create sparse datafile '%s': '%s'", filename, TRI_GET_ERRORBUF);
    TRI_FreeString(TRI_CORE_MEM_ZONE, filename);

    return TRI_ERROR_SYS_ERROR;
  }

  // memory map the data
  res = TRI_MMFile(0, maximalSize, PROT_WRITE | PROT_READ, MAP_SHARED, fd, &mmHandle, 0, &data);

  if (res != TRI_ERROR_NO_ERROR) {
    TRI_SYSTEM_ERROR();
    TRI_set_errno(res);
    TRI_CLOSE(fd);

    // remove empty file
    TRI_UnlinkFile(filename);

    LOG_ERROR("cannot memory map file '%s': '%s'", filename, TRI_GET_ERRORBUF);
    TRI_FreeString(TRI_CORE_MEM_ZONE, filename);

    return TRI_errno();
  }

  // copy the data
  memcpy(data, datafile->_data, vocSize);

  // patch the datafile structure
  res = TRI_UNMMFile(datafile->_data, datafile->_maximalSize, datafile->_fd, &datafile->_mmHandle);

  if (res < 0) {
    TRI_CLOSE(datafile->_fd);
    TRI_FreeString(TRI_CORE_MEM_ZONE, filename);

    LOG_ERROR("munmap failed with: %d", res);
    return res;
  }

  // .............................................................................................
  // For windows: Mem mapped files use handles
  // the datafile->_mmHandle handle object has been closed in the underlying
  // TRI_UNMMFile(...) call above so we do not need to close it for the associated file below
  // .............................................................................................

  TRI_CLOSE(datafile->_fd);

  datafile->_data = static_cast<char*>(data);
  datafile->_next = (char*)(data) + vocSize;
  datafile->_currentSize = vocSize;
  datafile->_maximalSize = static_cast<TRI_voc_size_t>(maximalSize);
  datafile->_fd = fd;
  datafile->_mmHandle = mmHandle;
  datafile->_state = TRI_DF_STATE_CLOSED;
  datafile->_full = false;
  datafile->_isSealed = false;
  datafile->_synced = static_cast<char*>(data);
  datafile->_written = datafile->_next;

  // rename files
  oldname = TRI_Concatenate2String(datafile->_filename, ".corrupted");

  res = TRI_RenameFile(datafile->_filename, oldname);

  if (res != TRI_ERROR_NO_ERROR) {
    TRI_FreeString(TRI_CORE_MEM_ZONE, filename);
    TRI_FreeString(TRI_CORE_MEM_ZONE, oldname);

    return res;
  }

  res = TRI_RenameFile(filename, datafile->_filename);

  if (res != TRI_ERROR_NO_ERROR) {
    TRI_FreeString(TRI_CORE_MEM_ZONE, filename);
    TRI_FreeString(TRI_CORE_MEM_ZONE, oldname);

    return res;
  }

  TRI_FreeString(TRI_CORE_MEM_ZONE, filename);
  TRI_FreeString(TRI_CORE_MEM_ZONE, oldname);

  // need to reset the datafile state here to write, otherwise the following call will return an error
  datafile->_state = TRI_DF_STATE_WRITE;

  return TRI_SealDatafile(datafile);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief scans a datafile
////////////////////////////////////////////////////////////////////////////////

static TRI_df_scan_t ScanDatafile (TRI_datafile_t const* datafile) {
  TRI_df_scan_t scan;
  TRI_df_scan_entry_t entry;
  TRI_voc_size_t currentSize;
  char* end;
  char* ptr;

  // this function must not be called for non-physical datafiles
  TRI_ASSERT(datafile->isPhysical(datafile));

  ptr = datafile->_data;
  end = datafile->_data + datafile->_currentSize;
  currentSize = 0;

  TRI_InitVector2(&scan._entries, TRI_CORE_MEM_ZONE, sizeof(TRI_df_scan_entry_t), 1024);

  scan._currentSize = datafile->_currentSize;
  scan._maximalSize = datafile->_maximalSize;
  scan._numberMarkers = 0;
  scan._status = 1;
  scan._isSealed = false; // assume false

  if (datafile->_currentSize == 0) {
    end = datafile->_data + datafile->_maximalSize;
  }

  while (ptr < end) {
    TRI_df_marker_t* marker = (TRI_df_marker_t*) ptr;
    bool ok;
    size_t size;

    memset(&entry, 0, sizeof(entry));

    entry._position   = (TRI_voc_size_t) (ptr - datafile->_data);
    entry._size       = marker->_size;
    entry._realSize   = TRI_DF_ALIGN_BLOCK(marker->_size);
    entry._tick       = marker->_tick;
    entry._type       = marker->_type;
    entry._status     = 1;
    entry._diagnosis  = nullptr;

    if (marker->_size == 0 && marker->_crc == 0 && marker->_type == 0 && marker->_tick == 0) {
      entry._status = 2;

      scan._endPosition = currentSize;

      TRI_PushBackVector(&scan._entries, &entry);
      return scan;
    }

    ++scan._numberMarkers;

    if (marker->_size == 0) {
      entry._status = 3;

      scan._status = 2;
      scan._endPosition = currentSize;

      TRI_PushBackVector(&scan._entries, &entry);
      return scan;
    }

    if (marker->_size < sizeof(TRI_df_marker_t)) {
      entry._status = 4;

      auto&& diagnosis = DiagnoseMarker(marker, end);
      entry._diagnosis = TRI_DuplicateString2Z(TRI_UNKNOWN_MEM_ZONE, diagnosis.c_str(), diagnosis.size());

      scan._endPosition = currentSize;
      scan._status = 3;

      TRI_PushBackVector(&scan._entries, &entry);
      return scan;
    }

    if (! TRI_IsValidMarkerDatafile(marker)) {
      entry._status = 4;

      auto&& diagnosis = DiagnoseMarker(marker, end);
      entry._diagnosis = TRI_DuplicateString2Z(TRI_UNKNOWN_MEM_ZONE, diagnosis.c_str(), diagnosis.size());

      scan._endPosition = currentSize;
      scan._status = 3;

      TRI_PushBackVector(&scan._entries, &entry);
      return scan;
    }

    ok = CheckCrcMarker(marker, end);

    if (! ok) {
      entry._status = 5;
      
      auto&& diagnosis = DiagnoseMarker(marker, end);
      entry._diagnosis = TRI_DuplicateString2Z(TRI_UNKNOWN_MEM_ZONE, diagnosis.c_str(), diagnosis.size());
      
      scan._status = 4;
    }

    TRI_PushBackVector(&scan._entries, &entry);

    size = TRI_DF_ALIGN_BLOCK(marker->_size);
    currentSize += (TRI_voc_size_t) size;

    if (marker->_type == TRI_DF_MARKER_FOOTER) {
      scan._endPosition = currentSize;
      scan._isSealed = true;
      return scan;
    }

    ptr += size;
  }

  return scan;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief tries to repair a datafile
////////////////////////////////////////////////////////////////////////////////

static bool TryRepairDatafile (TRI_datafile_t* datafile) {
  // this function must not be called for non-physical datafiles
  TRI_ASSERT(datafile->isPhysical(datafile));

  char* ptr = datafile->_data;
  char* end = datafile->_data + datafile->_currentSize;

  if (datafile->_currentSize == 0) {
    end = datafile->_data + datafile->_maximalSize;
  }
  
  TRI_voc_size_t currentSize = 0;

  while (ptr < end) {
    TRI_df_marker_t* marker = reinterpret_cast<TRI_df_marker_t*>(ptr);

    if (marker->_size == 0) {
      // reached end
      return true;
    }

    if (marker->_size < sizeof(TRI_df_marker_t) ||
        ptr + marker->_size > end) {
      // marker too small or too big
      return false;
    }

    if (! TRI_IsValidMarkerDatafile(marker)) {
      // unknown marker type
      return false;
    }

    if (marker->_type != 0) {
      if (! CheckCrcMarker(marker, end)) {
        // CRC mismatch!
        auto next = reinterpret_cast<char const*>(marker) + marker->_size;
        auto p = next;

        if (p < end) {
          // check if the rest of the datafile is only followed by NULL bytes
          bool isFollowedByNullBytes = true;
          while (p < end) {
            if (*p != '\0') {
              isFollowedByNullBytes = false;
              break;
            }
            ++p;
          }

          if (isFollowedByNullBytes) {
            // only last marker in datafile was corrupt. fix the datafile in place
            LOG_INFO("truncating datafile '%s' at position %lu",
                     datafile->getName(datafile),
                     (unsigned long) currentSize);
            int res = TruncateAndSealDatafile(datafile, currentSize);
            return (res == TRI_ERROR_NO_ERROR);
          }
            
          // there is some other stuff following. now inspect it...
          TRI_ASSERT(next <= end);
      
          if (next < end) {
            // there is a next marker
            auto nextMarker = reinterpret_cast<TRI_df_marker_t const*>(next);

            if (nextMarker->_type != 0 && 
                nextMarker->_size >= sizeof(TRI_df_marker_t) &&
                next + nextMarker->_size <= end &&
                TRI_IsValidMarkerDatafile(nextMarker) &&
                CheckCrcMarker(nextMarker, end)) {
              // next marker looks good.

              // create a temporary buffer
              auto buffer = TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, marker->_size, false);

              if (buffer == nullptr) {
                return false;
              }

              // create a new marker in the temporary buffer
              auto temp = reinterpret_cast<TRI_df_marker_t*>(buffer);
              TRI_InitMarkerDatafile(static_cast<char*>(buffer), TRI_DF_MARKER_BLANK, static_cast<TRI_voc_size_t>(marker->_size));
              temp->_crc = CalculateCrcValue(temp);

              // all done. now copy back the marker into the file
              memcpy(static_cast<void*>(ptr), buffer, static_cast<size_t>(marker->_size));

              TRI_Free(TRI_UNKNOWN_MEM_ZONE, buffer);
            
              bool ok = datafile->sync(datafile, ptr, (ptr + marker->_size));

              if (ok) {
                LOG_INFO("zeroed single invalid marker in datafile '%s' at position %lu",
                         datafile->getName(datafile),
                         (unsigned long) currentSize);
              }
              else {
                LOG_ERROR("could not zero single invalid marker in datafile '%s' at position %lu",
                           datafile->getName(datafile),
                           (unsigned long) currentSize);
                return false;
              }
            }
            else {
              // next marker looks broken, too.
              int res = TruncateAndSealDatafile(datafile, currentSize);
              return (res == TRI_ERROR_NO_ERROR);
            }
          }
        }

      }
    }

    size_t size = TRI_DF_ALIGN_BLOCK(marker->_size);
    currentSize += (TRI_voc_size_t) size;

    if (marker->_type == TRI_DF_MARKER_FOOTER) {
      return true;
    }

    ptr += size;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief fixes a corrupted datafile
////////////////////////////////////////////////////////////////////////////////

static bool FixDatafile (TRI_datafile_t* datafile,
                         TRI_voc_size_t currentSize) {
  LOG_WARNING("datafile '%s' is corrupted at position %llu", 
              datafile->getName(datafile), 
              (unsigned long long) currentSize);
  
  LOG_WARNING("setting datafile '%s' to read-only and ignoring all data from this file beyond this position",
              datafile->getName(datafile));

  datafile->_currentSize = currentSize;
  datafile->_maximalSize = static_cast<TRI_voc_size_t>(currentSize);
  datafile->_next        = datafile->_data + datafile->_currentSize;
  datafile->_full        = true;
  datafile->_state       = TRI_DF_STATE_READ;
  datafile->_isSealed    = true;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checks a datafile
////////////////////////////////////////////////////////////////////////////////

static bool CheckDatafile (TRI_datafile_t* datafile,
                           bool ignoreFailures) {
  // this function must not be called for non-physical datafiles
  TRI_ASSERT(datafile->isPhysical(datafile));

  char* ptr = datafile->_data;
  char* end = datafile->_data + datafile->_currentSize;
  TRI_voc_size_t currentSize = 0;

  if (datafile->_currentSize == 0) {
    LOG_WARNING("current size is 0 in read-only datafile '%s', trying to fix", datafile->getName(datafile));

    end = datafile->_data + datafile->_maximalSize;
  }

  TRI_voc_tick_t maxTick = 0;
  
  auto updateTick = [] (TRI_voc_tick_t maxTick) -> void {
    TRI_UpdateTickServer(maxTick);
  };

  while (ptr < end) {
    TRI_df_marker_t* marker = reinterpret_cast<TRI_df_marker_t*>(ptr);

#ifdef DEBUG_DATAFILE
    LOG_TRACE("MARKER: size %lu, tick %lx, crc %lx, type %u",
              (unsigned long) marker->_size,
              (unsigned long long) marker->_tick,
              (unsigned long) marker->_crc,
              (unsigned int) marker->_type);
#endif

    if (marker->_size == 0) {
      LOG_DEBUG("reached end of datafile '%s' data, current size %lu",
                datafile->getName(datafile),
                (unsigned long) currentSize);

      datafile->_currentSize = currentSize;
      datafile->_next = datafile->_data + datafile->_currentSize;

      updateTick(maxTick);

      return true;
    }

    if (marker->_size < sizeof(TRI_df_marker_t)) {
      if (ignoreFailures) {
        return FixDatafile(datafile, currentSize);
      }
      else {
        datafile->_lastError = TRI_set_errno(TRI_ERROR_ARANGO_CORRUPTED_DATAFILE);
        datafile->_currentSize = currentSize;
        datafile->_next = datafile->_data + datafile->_currentSize;
        datafile->_state = TRI_DF_STATE_OPEN_ERROR;

        LOG_WARNING("marker in datafile '%s' too small, size %lu, should be at least %lu",
                    datafile->getName(datafile),
                    (unsigned long) marker->_size,
                    (unsigned long) sizeof(TRI_df_marker_t));
      
        updateTick(maxTick);

        return false;
      }
    }

    // prevent reading over the end of the file
    if (ptr + marker->_size > end) {
      if (ignoreFailures) {
        return FixDatafile(datafile, currentSize);
      }
      else {
        datafile->_lastError = TRI_set_errno(TRI_ERROR_ARANGO_CORRUPTED_DATAFILE);
        datafile->_currentSize = currentSize;
        datafile->_next = datafile->_data + datafile->_currentSize;
        datafile->_state = TRI_DF_STATE_OPEN_ERROR;

        LOG_WARNING("marker in datafile '%s' points with size %lu beyond end of file",
                    datafile->getName(datafile),
                    (unsigned long) marker->_size);
      
        updateTick(maxTick);

        return false;
      }
    }

    // the following sanity check offers some, but not 100% crash-protection when reading
    // totally corrupted datafiles
    if (! TRI_IsValidMarkerDatafile(marker)) {
      if (marker->_type == 0 && marker->_size < 128) {
        // ignore markers with type 0 and a small size
        LOG_WARNING("ignoring suspicious marker in datafile '%s': type: %d, size: %lu",
                    datafile->getName(datafile),
                    (int) marker->_type,
                    (unsigned long) marker->_size);
      }
      else {
        if (ignoreFailures) {
          return FixDatafile(datafile, currentSize);
        }
        else {
          datafile->_lastError = TRI_set_errno(TRI_ERROR_ARANGO_CORRUPTED_DATAFILE);
          datafile->_currentSize = currentSize;
          datafile->_next = datafile->_data + datafile->_currentSize;
          datafile->_state = TRI_DF_STATE_OPEN_ERROR;

          LOG_WARNING("marker in datafile '%s' is corrupt: type: %d, size: %lu",
                      datafile->getName(datafile),
                      (int) marker->_type,
                      (unsigned long) marker->_size);
      
          updateTick(maxTick);

          return false;
        }
      }
    }

    if (marker->_type != 0) {
      bool ok = CheckCrcMarker(marker, end);

      if (! ok) {
        // CRC mismatch!
        bool nextMarkerOk = false;

        if (marker->_size > 0) {
          auto next = reinterpret_cast<char const*>(marker) + marker->_size;
          auto p = next;

          if (p < end) {
            // check if the rest of the datafile is only followed by NULL bytes
            bool isFollowedByNullBytes = true;
            while (p < end) {
              if (*p != '\0') {
                isFollowedByNullBytes = false;
                break;
              }
              ++p;
            }

            if (isFollowedByNullBytes) {
              // only last marker in datafile was corrupt. fix the datafile in place
              LOG_WARNING("datafile '%s' automatically truncated at last marker", datafile->getName(datafile));
              ignoreFailures = true;
            }
            else {
              // there is some other stuff following. now inspect it...
              TRI_ASSERT(next <= end);
        
              if (next < end) {
                // there is a next marker
                auto nextMarker = reinterpret_cast<TRI_df_marker_t const*>(next);

                if (nextMarker->_type != 0 && 
                    nextMarker->_size >= sizeof(TRI_df_marker_t) &&
                    next + nextMarker->_size <= end &&
                    TRI_IsValidMarkerDatafile(nextMarker) &&
                    CheckCrcMarker(nextMarker, end)) {
                  // next marker looks good.
                  nextMarkerOk = true;
                }
              }
              else {
                // EOF
                nextMarkerOk = true;
              }
            }
          }
        }

        if (ignoreFailures) {
          return FixDatafile(datafile, currentSize);
        }
        else {
          datafile->_lastError = TRI_set_errno(TRI_ERROR_ARANGO_CORRUPTED_DATAFILE);
          datafile->_currentSize = currentSize;
          datafile->_next = datafile->_data + datafile->_currentSize;
          datafile->_state = TRI_DF_STATE_OPEN_ERROR;

          LOG_WARNING("crc mismatch found in datafile '%s' at position %lu. expected crc: %x, actual crc: %x", 
                      datafile->getName(datafile),
                      (unsigned long) currentSize,
                      CalculateCrcValue(marker),
                      marker->_crc);
          
          if (nextMarkerOk) {
            LOG_INFO("data directly following this marker looks ok so repairing the marker may recover it");
          }
          else {
            LOG_WARNING("data directly following this marker cannot be analyzed");
          }

          updateTick(maxTick);

          return false;
        }
      }
    }

    if (marker->_tick > maxTick) {
      maxTick = marker->_tick;
    }

    size_t size = TRI_DF_ALIGN_BLOCK(marker->_size);
    currentSize += (TRI_voc_size_t) size;

    if (marker->_type == TRI_DF_MARKER_FOOTER) {
      LOG_DEBUG("found footer, reached end of datafile '%s', current size %lu",
                datafile->getName(datafile),
                (unsigned long) currentSize);

      datafile->_isSealed = true;
      datafile->_currentSize = currentSize;
      datafile->_next = datafile->_data + datafile->_currentSize;

      updateTick(maxTick);
      return true;
    }

    ptr += size;
  }

  updateTick(maxTick);
  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief extract the numeric part from a filename
/// the filename must look like this: /.*type-abc\.ending$/, where abc is
/// a number, and type and ending are arbitrary letters
////////////////////////////////////////////////////////////////////////////////

static uint64_t GetNumericFilenamePart (const char* filename) {
  char const* pos1 = strrchr(filename, '.');

  if (pos1 == nullptr) {
    return 0;
  }

  char const* pos2 = strrchr(filename, '-');

  if (pos2 == nullptr || pos2 > pos1) {
    return 0;
  }

  return TRI_UInt64String2(pos2 + 1, pos1 - pos2 - 1);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create the initial datafile header marker
////////////////////////////////////////////////////////////////////////////////

static int WriteInitialHeaderMarker (TRI_datafile_t* datafile,
                                     TRI_voc_fid_t fid,
                                     TRI_voc_size_t maximalSize) {
  // create the header
  TRI_df_header_marker_t header;
  TRI_InitMarkerDatafile((char*) &header, TRI_DF_MARKER_HEADER, sizeof(TRI_df_header_marker_t));
  header.base._tick = (TRI_voc_tick_t) fid;

  header._version     = TRI_DF_VERSION;
  header._maximalSize = maximalSize;
  header._fid         = fid;

  // reserve space and write header to file
  TRI_df_marker_t* position;
  int res = TRI_ReserveElementDatafile(datafile, header.base._size, &position, 0);

  if (res == TRI_ERROR_NO_ERROR) {
    TRI_ASSERT_EXPENSIVE(position != nullptr);
    res = TRI_WriteCrcElementDatafile(datafile, position, &header.base, false);
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief opens a datafile
////////////////////////////////////////////////////////////////////////////////

static TRI_datafile_t* OpenDatafile (char const* filename,
                                     bool ignoreErrors) {
  TRI_ERRORBUF;
  TRI_voc_size_t size;
  TRI_voc_fid_t fid;
  bool ok;
  void* data;
  char* ptr;
  int fd;
  int res;
  ssize_t len;
  TRI_stat_t status;
  TRI_df_header_marker_t header;
  void* mmHandle;

  // this function must not be called for non-physical datafiles
  TRI_ASSERT(filename != nullptr);

  fid = GetNumericFilenamePart(filename);

  // ..........................................................................
  // attempt to open a datafile file
  // ..........................................................................

  fd = TRI_OPEN(filename, O_RDWR);

  if (fd < 0) {
    TRI_SYSTEM_ERROR();
    TRI_set_errno(TRI_ERROR_SYS_ERROR);

    LOG_ERROR("cannot open datafile '%s': '%s'", filename, TRI_GET_ERRORBUF);

    return nullptr;
  }

  // compute the size of the file
  res = TRI_FSTAT(fd, &status);

  if (res < 0) {
    TRI_SYSTEM_ERROR();
    TRI_set_errno(TRI_ERROR_SYS_ERROR);
    TRI_CLOSE(fd);

    LOG_ERROR("cannot get status of datafile '%s': %s", filename, TRI_GET_ERRORBUF);

    return nullptr;
  }

  // check that file is not too small
  size = static_cast<TRI_voc_size_t>(status.st_size);

  if (size < sizeof(TRI_df_header_marker_t) + sizeof(TRI_df_footer_marker_t)) {
    TRI_set_errno(TRI_ERROR_ARANGO_CORRUPTED_DATAFILE);
    TRI_CLOSE(fd);

    LOG_ERROR("datafile '%s' is corrupt, size is only %u", filename, (unsigned int) size);

    return nullptr;
  }

  // read header from file
  ptr = (char*) &header;
  len = sizeof(TRI_df_header_marker_t);

  ok = TRI_ReadPointer(fd, ptr, len);

  if (! ok) {
    LOG_ERROR("cannot read datafile header from '%s': %s", filename, TRI_last_error());

    TRI_CLOSE(fd);
    return nullptr;
  }
  
  char const* end = static_cast<char const*>(ptr) + len;

  // check CRC
  ok = CheckCrcMarker(&header.base, end);

  if (! ok) {
    TRI_set_errno(TRI_ERROR_ARANGO_CORRUPTED_DATAFILE);

    LOG_ERROR("corrupted datafile header read from '%s'", filename);

    if (! ignoreErrors) {
      TRI_CLOSE(fd);
      return nullptr;
    }
  }

  // check the datafile version
  if (ok) {
    if (header._version != TRI_DF_VERSION) {
      TRI_set_errno(TRI_ERROR_ARANGO_CORRUPTED_DATAFILE);

      LOG_ERROR("unknown datafile version '%u' in datafile '%s'",
                (unsigned int) header._version,
                filename);

      if (! ignoreErrors) {
        TRI_CLOSE(fd);
        return nullptr;
      }
    }
  }

  // check the maximal size
  if (size > header._maximalSize) {
    LOG_DEBUG("datafile '%s' has size '%u', but maximal size is '%u'",
              filename,
              (unsigned int) size,
              (unsigned int) header._maximalSize);
  }

  // map datafile into memory
  res = TRI_MMFile(0, size, PROT_READ, MAP_SHARED, fd, &mmHandle, 0, &data);

  if (res != TRI_ERROR_NO_ERROR) {
    TRI_set_errno(res);
    TRI_CLOSE(fd);

    LOG_ERROR("cannot memory map datafile '%s': %s", filename, TRI_errno_string(res));
    return nullptr;
  }

  // create datafile structure
  TRI_datafile_t* datafile = static_cast<TRI_datafile_t*>(TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_datafile_t), false));

  if (datafile == nullptr) {
    TRI_UNMMFile(data, size, fd, &mmHandle);
    TRI_CLOSE(fd);

    return nullptr;
  }

  InitDatafile(datafile,
               TRI_DuplicateString(filename),
               fd,
               mmHandle,
               size,
               size,
               fid,
               static_cast<char*>(data));

  return datafile;
}

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief creates either an anonymous or a physical datafile
////////////////////////////////////////////////////////////////////////////////

TRI_datafile_t* TRI_CreateDatafile (char const* filename,
                                    TRI_voc_fid_t fid,
                                    TRI_voc_size_t maximalSize,
                                    bool withInitialMarkers) {
  TRI_datafile_t* datafile;

  TRI_ASSERT(PageSize >= 256);

  // use multiples of page-size
  maximalSize = (TRI_voc_size_t) (((maximalSize + PageSize - 1) / PageSize) * PageSize);

  // sanity check maximal size
  if (sizeof(TRI_df_header_marker_t) + sizeof(TRI_df_footer_marker_t) > maximalSize) {
    LOG_ERROR("cannot create datafile, maximal size '%u' is too small", (unsigned int) maximalSize);
    TRI_set_errno(TRI_ERROR_ARANGO_MAXIMAL_SIZE_TOO_SMALL);

    return nullptr;
  }

  // create either an anonymous or a physical datafile
  if (filename == nullptr) {
#ifdef TRI_HAVE_ANONYMOUS_MMAP
    datafile = TRI_CreateAnonymousDatafile(fid, maximalSize);
#else
    // system does not support anonymous mmap
    return nullptr;
#endif
  }
  else {
    datafile = TRI_CreatePhysicalDatafile(filename, fid, maximalSize);
  }

  if (datafile == nullptr) {
    // an error occurred during creation
    return nullptr;
  }


  datafile->_state = TRI_DF_STATE_WRITE;

  if (withInitialMarkers) {
    int res = WriteInitialHeaderMarker(datafile, fid, maximalSize);

    if (res != TRI_ERROR_NO_ERROR) {
      LOG_ERROR("cannot write header to datafile '%s'", datafile->getName(datafile));
      TRI_UNMMFile(datafile->_data, datafile->_maximalSize, datafile->_fd, &datafile->_mmHandle);

      datafile->close(datafile);
      datafile->destroy(datafile);
      TRI_Free(TRI_UNKNOWN_MEM_ZONE, datafile);

      return nullptr;
    }
  }

  LOG_DEBUG("created datafile '%s' of size %u and page-size %u",
            datafile->getName(datafile),
            (unsigned int) maximalSize,
            (unsigned int) PageSize);

  return datafile;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a new anonymous datafile
///
/// this is only supported on certain platforms (Linux, MacOS)
////////////////////////////////////////////////////////////////////////////////

#ifdef TRI_HAVE_ANONYMOUS_MMAP

TRI_datafile_t* TRI_CreateAnonymousDatafile (TRI_voc_fid_t fid,
                                             TRI_voc_size_t maximalSize) {
  TRI_datafile_t* datafile;
  ssize_t res;
  void* data;
  void* mmHandle;
  int flags;
  int fd;

#ifdef TRI_MMAP_ANONYMOUS
  // fd -1 is required for "real" anonymous regions
  fd = -1;
  flags = TRI_MMAP_ANONYMOUS | MAP_SHARED;
#else
  // ugly workaround if MAP_ANONYMOUS is not available
  // TODO: make this more portable
  // TODO: find a good workaround for Windows
  fd = TRI_OPEN("/dev/zero", O_RDWR);
  if (fd == -1) {
    return nullptr;
  }

  flags = MAP_PRIVATE;
#endif

  // memory map the data
  res = TRI_MMFile(nullptr, maximalSize, PROT_WRITE | PROT_READ, flags, fd, &mmHandle, 0, &data);

#ifdef MAP_ANONYMOUS
  // nothing to do
#else
  // close auxilliary file
  TRI_CLOSE(fd);
  fd = -1;
#endif

  if (res != TRI_ERROR_NO_ERROR) {
    TRI_set_errno(res);

    LOG_ERROR("cannot memory map anonymous region: %s", TRI_last_error());
    return nullptr;
  }

  // create datafile structure
  datafile = static_cast<TRI_datafile_t*>(TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_datafile_t), false));

  if (datafile == nullptr) {
    TRI_set_errno(TRI_ERROR_OUT_OF_MEMORY);

    LOG_ERROR("out of memory");
    return nullptr;
  }

  InitDatafile(datafile,
               nullptr,
               fd,
               mmHandle,
               maximalSize,
               0,
               fid,
               static_cast<char*>(data));

  return datafile;
}

#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a new physical datafile
////////////////////////////////////////////////////////////////////////////////

TRI_datafile_t* TRI_CreatePhysicalDatafile (char const* filename,
                                            TRI_voc_fid_t fid,
                                            TRI_voc_size_t maximalSize) {
  void* data;
  void* mmHandle;

  TRI_ASSERT(filename != nullptr);

  int fd = CreateSparseFile(filename, maximalSize);

  if (fd < 0) {
    // an error occurred
    return nullptr;
  }

  // memory map the data
  ssize_t res = TRI_MMFile(0, maximalSize, PROT_WRITE | PROT_READ, MAP_SHARED, fd, &mmHandle, 0, &data);

  if (res != TRI_ERROR_NO_ERROR) {
    TRI_set_errno(res);
    TRI_CLOSE(fd);

    // remove empty file
    TRI_UnlinkFile(filename);

    LOG_ERROR("cannot memory map file '%s': '%s'", filename, TRI_errno_string((int) res));
    return nullptr;
  }

  // create datafile structure
  auto datafile = static_cast<TRI_datafile_t*>(TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_datafile_t), false));

  if (datafile == nullptr) {
    TRI_set_errno(TRI_ERROR_OUT_OF_MEMORY);
    TRI_CLOSE(fd);

    LOG_ERROR("out of memory");
    return nullptr;
  }

  InitDatafile(datafile,
               TRI_DuplicateString(filename),
               fd,
               mmHandle,
               maximalSize,
               0,
               fid,
               static_cast<char*>(data));

  // Advise OS that sequential access is going to happen:
  TRI_MMFileAdvise(datafile->_data, datafile->_maximalSize,
                   TRI_MADVISE_SEQUENTIAL);

  return datafile;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief frees the memory allocated, but does not free the pointer
////////////////////////////////////////////////////////////////////////////////

void TRI_DestroyDatafile (TRI_datafile_t* datafile) {
  datafile->destroy(datafile);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief frees the memory allocated and but frees the pointer
////////////////////////////////////////////////////////////////////////////////

void TRI_FreeDatafile (TRI_datafile_t* datafile) {
  TRI_DestroyDatafile(datafile);
  TRI_Free(TRI_UNKNOWN_MEM_ZONE, datafile);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the name for a marker
////////////////////////////////////////////////////////////////////////////////

char const* TRI_NameMarkerDatafile (TRI_df_marker_t const* marker) {
  switch (marker->_type) {
    // general markers
    case TRI_DF_MARKER_HEADER:
    case TRI_DF_MARKER_BLANK:
      return "blank marker (used when repairing datafiles)";
    case TRI_COL_MARKER_HEADER:
      return "header";
    case TRI_DF_MARKER_FOOTER:
      return "footer";

    // datafile markers
    case TRI_DOC_MARKER_KEY_DOCUMENT:
      return "document (df)";
    case TRI_DOC_MARKER_KEY_EDGE:
      return "edge (df)";
    case TRI_DOC_MARKER_KEY_DELETION:
      return "deletion (df)";
    case TRI_DOC_MARKER_BEGIN_TRANSACTION:
      return "begin transaction (df)";
    case TRI_DOC_MARKER_COMMIT_TRANSACTION:
      return "commit transaction (df)";
    case TRI_DOC_MARKER_ABORT_TRANSACTION:
      return "abort transaction (df)";
    case TRI_DOC_MARKER_PREPARE_TRANSACTION:
      return "prepare transaction (df)";
    case TRI_DF_MARKER_ATTRIBUTE:
      return "attribute (df)";
    case TRI_DF_MARKER_SHAPE:
      return "shape (df)";

    // wal markers
    case TRI_WAL_MARKER_ATTRIBUTE:
      return "attribute (wal)";
    case TRI_WAL_MARKER_SHAPE:
      return "shape (wal)";
    case TRI_WAL_MARKER_DOCUMENT:
      return "document (wal)";
    case TRI_WAL_MARKER_EDGE:
      return "edge (wal)";
    case TRI_WAL_MARKER_REMOVE:
      return "deletion (wal)";
    case TRI_WAL_MARKER_BEGIN_TRANSACTION:
      return "begin transaction (wal)";
    case TRI_WAL_MARKER_COMMIT_TRANSACTION:
      return "commit transaction (wal)";
    case TRI_WAL_MARKER_ABORT_TRANSACTION:
      return "abort transaction (wal)";
    case TRI_WAL_MARKER_BEGIN_REMOTE_TRANSACTION:
      return "begin remote transaction (wal)";
    case TRI_WAL_MARKER_COMMIT_REMOTE_TRANSACTION:
      return "commit remote transaction (wal)";
    case TRI_WAL_MARKER_ABORT_REMOTE_TRANSACTION:
      return "abort remote transaction (wal)";
    case TRI_WAL_MARKER_CREATE_COLLECTION:
      return "create collection (wal)";
    case TRI_WAL_MARKER_DROP_COLLECTION:
      return "drop collection (wal)";
    case TRI_WAL_MARKER_RENAME_COLLECTION:
      return "rename collection (wal)";
    case TRI_WAL_MARKER_CHANGE_COLLECTION:
      return "change collection (wal)";
    case TRI_WAL_MARKER_CREATE_INDEX:
      return "create index (wal)";
    case TRI_WAL_MARKER_DROP_INDEX:
      return "drop index (wal)";
    case TRI_WAL_MARKER_CREATE_DATABASE:
      return "create database (wal)";
    case TRI_WAL_MARKER_DROP_DATABASE:
      return "drop database (wal)";

    default:
      return "unused/unknown";
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief initialises a marker with the most basic information
////////////////////////////////////////////////////////////////////////////////

void TRI_InitMarkerDatafile (char* marker,
                             TRI_df_marker_type_e type,
                             TRI_voc_size_t size) {

  TRI_ASSERT(marker != nullptr);
  TRI_ASSERT(type > TRI_MARKER_MIN && type < TRI_MARKER_MAX);
  TRI_ASSERT(size > 0);

  // initialise the basic bytes
  memset(marker, 0, size);

  TRI_df_marker_t* df = reinterpret_cast<TRI_df_marker_t*>(marker);
  df->_size = size;
  df->_type = type;
  // not needed because of memset above
  // marker->_crc  = 0;
  // marker->_tick = 0;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checks whether a marker is valid
////////////////////////////////////////////////////////////////////////////////

bool TRI_IsValidMarkerDatafile (TRI_df_marker_t const* marker) {
  if (marker == nullptr) {
    return false;
  }

  // check marker type
  TRI_df_marker_type_t type = marker->_type;
  if (type <= (TRI_df_marker_type_t) TRI_MARKER_MIN) {
    // marker type is less than minimum allowed type value
    return false;
  }

  if (type >= (TRI_df_marker_type_t) TRI_MARKER_MAX) {
    // marker type is greater than maximum allowed type value
    return false;
  }

  if (marker->_size >= (TRI_voc_size_t) TRI_MARKER_MAXIMAL_SIZE) {
    // a single marker bigger than 256 MB seems unreasonable
    // note: this is an arbitrary limit
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief reserves room for an element, advances the pointer
///
/// note: maximalJournalSize is the collection's maximalJournalSize property,
/// which may be different from the size of the current datafile
/// some callers do not set the value of maximalJournalSize
////////////////////////////////////////////////////////////////////////////////

int TRI_ReserveElementDatafile (TRI_datafile_t* datafile,
                                TRI_voc_size_t size,
                                TRI_df_marker_t** position,
                                TRI_voc_size_t maximalJournalSize) {
  *position = nullptr;
  size = TRI_DF_ALIGN_BLOCK(size);

  if (datafile->_state != TRI_DF_STATE_WRITE) {
    if (datafile->_state == TRI_DF_STATE_READ) {
      LOG_ERROR("cannot reserve marker, datafile is read-only");

      return TRI_set_errno(TRI_ERROR_ARANGO_READ_ONLY);
    }

    return TRI_set_errno(TRI_ERROR_ARANGO_ILLEGAL_STATE);
  }

  // check the maximal size
  if (size + TRI_JOURNAL_OVERHEAD > datafile->_maximalSize) {
    // marker is bigger than journal size.
    // adding the marker to this datafile will not work

    if (maximalJournalSize <= datafile->_maximalSize) {
      // the collection property 'maximalJournalSize' is equal to
      // or smaller than the size of this datafile
      // creating a new file and writing the marker into it will not work either
      return TRI_set_errno(TRI_ERROR_ARANGO_DOCUMENT_TOO_LARGE);
    }

    // if we get here, the collection's 'maximalJournalSize' property is
    // higher than the size of this datafile.
    // maybe the marker will fit into a new datafile with the bigger size?
    if (size + TRI_JOURNAL_OVERHEAD > maximalJournalSize) {
      // marker still won't fit
      return TRI_set_errno(TRI_ERROR_ARANGO_DOCUMENT_TOO_LARGE);
    }

    // fall-through intentional
  }

  // add the marker, leave enough room for the footer
  if (datafile->_currentSize + size + datafile->_footerSize > datafile->_maximalSize) {
    datafile->_lastError = TRI_set_errno(TRI_ERROR_ARANGO_DATAFILE_FULL);
    datafile->_full = true;

    LOG_TRACE("cannot write marker, not enough space");

    return datafile->_lastError;
  }

  *position = (TRI_df_marker_t*) datafile->_next;

  TRI_ASSERT_EXPENSIVE(*position != nullptr);

  datafile->_next += size;
  datafile->_currentSize += size;

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief writes a marker to the datafile
/// this function will write the marker as-is, without any CRC or tick updates
////////////////////////////////////////////////////////////////////////////////

int TRI_WriteElementDatafile (TRI_datafile_t* datafile,
                              void* position,
                              TRI_df_marker_t const* marker,
                              bool forceSync) {
  TRI_ASSERT(marker->_tick > 0);
  TRI_ASSERT(marker->_size > 0);

  TRI_UpdateTicksDatafile(datafile, marker);

  if (datafile->_state != TRI_DF_STATE_WRITE) {
    if (datafile->_state == TRI_DF_STATE_READ) {
      LOG_ERROR("cannot write marker, datafile is read-only");

      return TRI_set_errno(TRI_ERROR_ARANGO_READ_ONLY);
    }

    return TRI_set_errno(TRI_ERROR_ARANGO_ILLEGAL_STATE);
  }
    
  TRI_ASSERT_EXPENSIVE(position != nullptr);

  // out of bounds check for writing into a datafile
  if (position == nullptr ||
      position < (void*) datafile->_data ||
      position >= (void*) (datafile->_data + datafile->_maximalSize)) {

    LOG_ERROR("logic error. writing out of bounds of datafile '%s'", datafile->getName(datafile));
    return TRI_set_errno(TRI_ERROR_ARANGO_ILLEGAL_STATE);
  }

  memcpy(position, marker, static_cast<size_t>(marker->_size));


  if (forceSync) {
    bool ok = datafile->sync(datafile, static_cast<char const*>(position), ((char*) position) + marker->_size);

    if (! ok) {
      datafile->_state = TRI_DF_STATE_WRITE_ERROR;

      if (errno == ENOSPC) {
        datafile->_lastError = TRI_set_errno(TRI_ERROR_ARANGO_FILESYSTEM_FULL);
      }
      else {
        datafile->_lastError = TRI_set_errno(TRI_ERROR_SYS_ERROR);
      }

      LOG_ERROR("msync failed with: %s", TRI_last_error());

      return datafile->_lastError;
    }
    else {
      LOG_TRACE("msync succeeded %p, size %lu", position, (unsigned long) marker->_size);
    }
  }

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief update tick values for a datafile
////////////////////////////////////////////////////////////////////////////////

void TRI_UpdateTicksDatafile (TRI_datafile_t* datafile,
                              TRI_df_marker_t const* marker) {
  TRI_df_marker_type_e type = (TRI_df_marker_type_e) marker->_type;
  
  if (type != TRI_DF_MARKER_HEADER && 
      type != TRI_DF_MARKER_FOOTER &&
      type != TRI_COL_MARKER_HEADER) {
    // every marker but headers / footers counts

    TRI_voc_tick_t tick = marker->_tick;

    if (datafile->_tickMin == 0) {
      datafile->_tickMin = tick;
    }

    if (datafile->_tickMax < marker->_tick) {
      datafile->_tickMax = tick;
    }

    if (type != TRI_DF_MARKER_ATTRIBUTE &&
        type != TRI_DF_MARKER_SHAPE &&
        type != TRI_WAL_MARKER_ATTRIBUTE &&
        type != TRI_WAL_MARKER_SHAPE) {
      if (datafile->_dataMin == 0) {
        datafile->_dataMin = tick;
      }

      if (datafile->_dataMax < tick) {
        datafile->_dataMax = tick;
      }
    }
  }

// TODO: check whether the following code can be removed safely
/*
  if (type != TRI_DF_MARKER_HEADER &&
      type != TRI_DF_MARKER_FOOTER &&
      type != TRI_COL_MARKER_HEADER &&
      type != TRI_DF_MARKER_ATTRIBUTE &&
      type != TRI_DF_MARKER_SHAPE) {

#ifdef TRI_ENABLE_MAINTAINER_MODE
    // check _tick value of marker and set min/max tick values for datafile
    if (marker->_tick < datafile->_tickMin) {
      LOG_FATAL_AND_EXIT("logic error. invalid tick value %llu encountered when writing marker of type %d into datafile '%s'. "
          "expected tick value >= tickMin %llu",
          (unsigned long long) tick,
          (int) marker->_type,
          datafile->getName(datafile),
          (unsigned long long) datafile->_tickMin);
    }

    if (tick < datafile->_tickMax) {
      LOG_FATAL_AND_EXIT("logic error. invalid tick value %llu encountered when writing marker of type %d into datafile '%s'. "
          "expected tick value >= tickMax %llu",
          (unsigned long long) tick,
          (int) marker->_type,
          datafile->getName(datafile),
          (unsigned long long) datafile->_tickMax);
    }

    if (tick < static_cast<TRI_voc_tick_t>(datafile->_fid)) {
      LOG_FATAL_AND_EXIT("logic error. invalid tick value %llu encountered when writing marker of type %d into datafile '%s'. "
          "expected tick value >= fid %llu",
          (unsigned long long) tick,
          (int) marker->_type,
          datafile->getName(datafile),
          (unsigned long long) datafile->_fid);
    }
#endif

    if (type == TRI_DOC_MARKER_KEY_DOCUMENT ||
        type == TRI_DOC_MARKER_KEY_EDGE) {
      if (datafile->_dataMin == 0) {
        datafile->_dataMin = tick;
      }

      if (datafile->_dataMax < tick) {
        datafile->_dataMax = tick;
      }
    }
  }

  if (type != TRI_DF_MARKER_ATTRIBUTE &&
      type != TRI_DF_MARKER_SHAPE) {

    if (datafile->_tickMin == 0) {
      datafile->_tickMin = tick;
    }

    if (datafile->_tickMax < marker->_tick) {
      datafile->_tickMax = tick;
    }
  }
  */
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checksums and writes a marker to the datafile
////////////////////////////////////////////////////////////////////////////////

int TRI_WriteCrcElementDatafile (TRI_datafile_t* datafile,
                                 void* position,
                                 TRI_df_marker_t* marker,
                                 bool forceSync) {
  TRI_ASSERT(marker->_tick != 0);

  if (datafile->isPhysical(datafile)) {
    TRI_voc_crc_t crc = TRI_InitialCrc32();

    crc = TRI_BlockCrc32(crc, (char const*) marker, marker->_size);
    marker->_crc = TRI_FinalCrc32(crc);
  }

  return TRI_WriteElementDatafile(datafile, position, marker, forceSync);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief iterates over a datafile
/// also may set datafile's min/max tick values
////////////////////////////////////////////////////////////////////////////////

bool TRI_IterateDatafile (TRI_datafile_t* datafile,
                          bool (*iterator)(TRI_df_marker_t const*, void*, TRI_datafile_t*),
                          void* data) {
  TRI_ASSERT(iterator != nullptr);

  LOG_TRACE("iterating over datafile '%s', fid: %llu",
            datafile->getName(datafile),
            (unsigned long long) datafile->_fid);

  char const* ptr = datafile->_data;
  char const* end = datafile->_data + datafile->_currentSize;

  if (datafile->_state != TRI_DF_STATE_READ && datafile->_state != TRI_DF_STATE_WRITE) {
    TRI_set_errno(TRI_ERROR_ARANGO_ILLEGAL_STATE);
    return false;
  }

  while (ptr < end) {
    TRI_df_marker_t const* marker = reinterpret_cast<TRI_df_marker_t const*>(ptr);

    if (marker->_size == 0) {
      return true;
    }

    // update the tick statistics
    TRI_UpdateTicksDatafile(datafile, marker);

    if (! iterator(marker, data, datafile)) {
      return false;
    }

    size_t size = TRI_DF_ALIGN_BLOCK(marker->_size);
    ptr += size;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief opens an existing datafile
///
/// The datafile will be opened read-only if a footer is found
////////////////////////////////////////////////////////////////////////////////

TRI_datafile_t* TRI_OpenDatafile (char const* filename,
                                  bool ignoreFailures) {
  // this function must not be called for non-physical datafiles
  TRI_ASSERT(filename != nullptr);

  TRI_datafile_t* datafile = OpenDatafile(filename, false);

  if (datafile == nullptr) {
    return nullptr;
  }

  // check the datafile by scanning markers
  bool ok = CheckDatafile(datafile, ignoreFailures);

  if (! ok) {
    TRI_UNMMFile(datafile->_data, datafile->_maximalSize, datafile->_fd, &datafile->_mmHandle);
    TRI_CLOSE(datafile->_fd);

    LOG_ERROR("datafile '%s' is corrupt", datafile->getName(datafile));
    // must free datafile here
    TRI_FreeDatafile(datafile);

    return nullptr;
  }

  // change to read-write if no footer has been found
  if (! datafile->_isSealed) {
    datafile->_state = TRI_DF_STATE_WRITE;
    TRI_ProtectMMFile(datafile->_data, datafile->_maximalSize, PROT_READ | PROT_WRITE, datafile->_fd, &datafile->_mmHandle);
  }

  // Advise on sequential use:
  TRI_MMFileAdvise(datafile->_data, datafile->_maximalSize,
                   TRI_MADVISE_SEQUENTIAL);
  TRI_MMFileAdvise(datafile->_data, datafile->_maximalSize,
                   TRI_MADVISE_WILLNEED);

  return datafile;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief closes a datafile and all memory regions
////////////////////////////////////////////////////////////////////////////////

bool TRI_CloseDatafile (TRI_datafile_t* datafile) {
  if (datafile->_state == TRI_DF_STATE_READ || datafile->_state == TRI_DF_STATE_WRITE) {
    int res;

    res = TRI_UNMMFile(datafile->_data, datafile->_maximalSize, datafile->_fd, &datafile->_mmHandle);

    if (res != TRI_ERROR_NO_ERROR) {
      LOG_ERROR("munmap failed with: %d", res);
      datafile->_state = TRI_DF_STATE_WRITE_ERROR;
      datafile->_lastError = res;
      return false;
    }

    else {
      datafile->close(datafile);
      datafile->_data = 0;
      datafile->_next = 0;
      datafile->_fd = -1;

      return true;
    }
  }
  else if (datafile->_state == TRI_DF_STATE_CLOSED) {
    LOG_WARNING("closing an already closed datafile '%s'", datafile->getName(datafile));
    return true;
  }
  else {
    TRI_set_errno(TRI_ERROR_ARANGO_ILLEGAL_STATE);
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief renames a datafile
////////////////////////////////////////////////////////////////////////////////

bool TRI_RenameDatafile (TRI_datafile_t* datafile, char const* filename) {
  // this function must not be called for non-physical datafiles
  TRI_ASSERT(datafile->isPhysical(datafile));
  TRI_ASSERT(filename != nullptr);

  if (TRI_ExistsFile(filename)) {
    LOG_ERROR("cannot overwrite datafile '%s'", filename);

    datafile->_lastError = TRI_set_errno(TRI_ERROR_ARANGO_DATAFILE_ALREADY_EXISTS);
    return false;
  }

  int res = TRI_RenameFile(datafile->_filename, filename);

  if (res != TRI_ERROR_NO_ERROR) {
    datafile->_state = TRI_DF_STATE_RENAME_ERROR;
    datafile->_lastError = TRI_set_errno(TRI_ERROR_SYS_ERROR);

    return false;
  }

  TRI_FreeString(TRI_CORE_MEM_ZONE, datafile->_filename);
  datafile->_filename = TRI_DuplicateString(filename);

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief seals a datafile, writes a footer, sets it to read-only
////////////////////////////////////////////////////////////////////////////////

int TRI_SealDatafile (TRI_datafile_t* datafile) {
  TRI_df_footer_marker_t footer;
  TRI_df_marker_t* position;

  if (datafile->_state == TRI_DF_STATE_READ) {
    return TRI_set_errno(TRI_ERROR_ARANGO_READ_ONLY);
  }

  if (datafile->_state != TRI_DF_STATE_WRITE) {
    return TRI_set_errno(TRI_ERROR_ARANGO_ILLEGAL_STATE);
  }

  if (datafile->_isSealed) {
    return TRI_set_errno(TRI_ERROR_ARANGO_DATAFILE_SEALED);
  }


  // create the footer
  TRI_InitMarkerDatafile((char*) &footer, TRI_DF_MARKER_FOOTER, sizeof(TRI_df_footer_marker_t));
  // set a proper tick value
  if (datafile->_tickMax == 0) {
    datafile->_tickMax = TRI_NewTickServer();
  }
  footer.base._tick = datafile->_tickMax;

  // reserve space and write footer to file
  datafile->_footerSize = 0;

  int res = TRI_ReserveElementDatafile(datafile, footer.base._size, &position, 0);

  if (res == TRI_ERROR_NO_ERROR) {
    TRI_ASSERT_EXPENSIVE(position != nullptr);
    res = TRI_WriteCrcElementDatafile(datafile, position, &footer.base, false);
  }

  if (res != TRI_ERROR_NO_ERROR) {
    return res;
  }

  // sync file
  bool ok = datafile->sync(datafile, datafile->_synced, ((char*) datafile->_data) + datafile->_currentSize);

  if (! ok) {
    datafile->_state = TRI_DF_STATE_WRITE_ERROR;

    if (errno == ENOSPC) {
      datafile->_lastError = TRI_set_errno(TRI_ERROR_ARANGO_FILESYSTEM_FULL);
    }
    else {
      datafile->_lastError = TRI_errno();
    }

    LOG_ERROR("msync failed with: %s", TRI_last_error());
  }

  // everything is now synced
  datafile->_synced = datafile->_written;

  /*
    TODO: do we have to unmap file? That is, release the memory which has been allocated for
          this file? At the moment the windows of function TRI_ProtectMMFile does nothing.
  */
  TRI_ProtectMMFile(datafile->_data, datafile->_maximalSize, PROT_READ, datafile->_fd, &datafile->_mmHandle);

  // truncate datafile
  if (ok) {
    #ifdef _WIN32
      res = 0;
      /*
      res = ftruncate(datafile->_fd, datafile->_currentSize);
      Linux centric problems:
        Under windows can not reduce size of the memory mapped file without unmapping it!
        However, apparently we may have users
      */
    #else
      res = datafile->truncate(datafile, datafile->_currentSize);
    #endif

    if (res < 0) {
      LOG_ERROR("cannot truncate datafile '%s': %s", datafile->getName(datafile), TRI_last_error());
      datafile->_lastError = TRI_set_errno(TRI_ERROR_SYS_ERROR);
      datafile->_state = TRI_DF_STATE_WRITE_ERROR;
      ok = false;
    }

    datafile->_isSealed = true;
    datafile->_state = TRI_DF_STATE_READ;
    datafile->_maximalSize = datafile->_currentSize;
  }

  if (! ok) {
    return datafile->_lastError;
  }

  if (datafile->isPhysical(datafile)) {
    // From now on we predict random access (until collection or compaction):
    TRI_MMFileAdvise(datafile->_data, datafile->_maximalSize,
                     TRI_MADVISE_RANDOM);
  }

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief truncates a datafile and seals it
/// this is called from the recovery procedure only
////////////////////////////////////////////////////////////////////////////////

int TRI_TruncateDatafile (char const* path,
                          TRI_voc_size_t position) {
  // this function must not be called for non-physical datafiles
  TRI_ASSERT(path != nullptr);

  TRI_datafile_t* datafile = OpenDatafile(path, true);

  if (datafile == nullptr) {
    return TRI_ERROR_ARANGO_DATAFILE_UNREADABLE;
  }

  int res = TruncateAndSealDatafile(datafile, position);
  TRI_CloseDatafile(datafile);
  TRI_FreeDatafile(datafile);

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief try to repair a datafile
////////////////////////////////////////////////////////////////////////////////

bool TRI_TryRepairDatafile (char const* path) {
  // this function must not be called for non-physical datafiles
  TRI_ASSERT(path != nullptr);

  TRI_datafile_t* datafile = OpenDatafile(path, true);

  if (datafile == nullptr) {
    return false;
  }
     
  // set to read/write access 
  TRI_ProtectMMFile(datafile->_data, datafile->_maximalSize, PROT_READ | PROT_WRITE, datafile->_fd, &datafile->_mmHandle);

  bool result = TryRepairDatafile(datafile);
  TRI_CloseDatafile(datafile);
  TRI_FreeDatafile(datafile);

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns information about the datafile
////////////////////////////////////////////////////////////////////////////////

TRI_df_scan_t TRI_ScanDatafile (char const* path) {
  TRI_df_scan_t scan;

  // this function must not be called for non-physical datafiles
  TRI_ASSERT(path != nullptr);

  TRI_datafile_t* datafile = OpenDatafile(path, true);

  if (datafile != nullptr) {
    scan = ScanDatafile(datafile);
    TRI_CloseDatafile(datafile);
    TRI_FreeDatafile(datafile);
  }
  else {
    scan._currentSize = 0;
    scan._maximalSize = 0;
    scan._endPosition = 0;
    scan._numberMarkers = 0;

    TRI_InitVector(&scan._entries, TRI_CORE_MEM_ZONE, sizeof(TRI_df_scan_entry_t));

    scan._status   = 5;
    scan._isSealed = false;
  }

  return scan;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys information about the datafile
////////////////////////////////////////////////////////////////////////////////

void TRI_DestroyDatafileScan (TRI_df_scan_t* scan) {
  size_t const n = TRI_LengthVector(&scan->_entries);

  for (size_t i = 0; i < n; ++i) {
    auto entry = static_cast<TRI_df_scan_entry_t*>(TRI_AtVector(&scan->_entries, i));

    if (entry != nullptr && entry->_diagnosis != nullptr) {
      TRI_FreeString(TRI_UNKNOWN_MEM_ZONE, entry->_diagnosis);
    }
  }

  TRI_DestroyVector(&scan->_entries);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
