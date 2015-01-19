////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC fulltext index
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
/// @author Jan Steemann
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "FulltextIndex.h"
#include "Basics/json.h"
#include "Basics/utf8-helper.h"
#include "FulltextIndex/fulltext-index.h"
#include "FulltextIndex/fulltext-wordlist.h"
#include "Utils/Exception.h"
#include "VocBase/document-collection.h"
#include "VocBase/voc-shaper.h"

using namespace triagens::basics;
using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                               class FulltextIndex
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new fulltext index
////////////////////////////////////////////////////////////////////////////////

FulltextIndex::FulltextIndex (TRI_idx_iid_t id,
                              TRI_document_collection_t* collection,
                              std::vector<std::string> const& fields,
                              int minWordLength) 
  : Index(id, collection, fields),
    _fulltextIndex(nullptr),
    _attribute(0),
    _minWordLength(minWordLength > 0 ? minWordLength : 1) {
  
  // look up the attribute
  TRI_shaper_t* shaper = _collection->getShaper();  // ONLY IN INDEX, PROTECTED by RUNTIME
  _attribute = shaper->findOrCreateAttributePathByName(shaper, _fields[0].c_str());

  if (_attribute == 0) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }

  _fulltextIndex = TRI_CreateFtsIndex(2048, 1, 1);
  
  if (_fulltextIndex == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the fulltext index
////////////////////////////////////////////////////////////////////////////////

FulltextIndex::~FulltextIndex () {
  if (_fulltextIndex != nullptr) {
    TRI_FreeFtsIndex(_fulltextIndex);
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                            public virtual methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document into the index
////////////////////////////////////////////////////////////////////////////////
        
void FulltextIndex::insert (TransactionCollection*,
                            TRI_doc_mptr_t const* doc) {
  TRI_fulltext_wordlist_t* wordlist = getWordlist(doc);

  if (wordlist == nullptr) {
    // TODO: distinguish the cases "empty wordlist" and "out of memory"
    return;
  }

  int res = TRI_ERROR_NO_ERROR;

  if (wordlist->_numWords > 0) {
    // TODO: use status codes
    if (! TRI_InsertWordsFulltextIndex(_fulltextIndex, (TRI_fulltext_doc_t) ((uintptr_t) doc), wordlist)) {
      res = TRI_ERROR_INTERNAL;
    }
  }

  TRI_FreeWordlistFulltextIndex(wordlist);

  if (res != TRI_ERROR_NO_ERROR) {
    THROW_ARANGO_EXCEPTION(res);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document from the index
////////////////////////////////////////////////////////////////////////////////

void FulltextIndex::remove (TransactionCollection*, 
                            TRI_doc_mptr_t const* doc) {
  TRI_DeleteDocumentFulltextIndex(_fulltextIndex, (TRI_fulltext_doc_t) ((uintptr_t) doc));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief post-insert operation (does nothing for this index type)
////////////////////////////////////////////////////////////////////////////////

void FulltextIndex::preCommit (TransactionCollection*) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief clean up the index
////////////////////////////////////////////////////////////////////////////////

void FulltextIndex::cleanup () {
  // check whether we should do a cleanup at all
  if (! TRI_CompactFulltextIndex(_fulltextIndex)) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get the memory usage of the index
////////////////////////////////////////////////////////////////////////////////
  
size_t FulltextIndex::memory () {
  return TRI_MemoryFulltextIndex(_fulltextIndex);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get the memory usage of the index
////////////////////////////////////////////////////////////////////////////////

Json FulltextIndex::toJson (TRI_memory_zone_t* zone) const {
  Json json(zone, Json::Object, 2);
  Json fields(zone, Json::Array, _fields.size());
  for (auto& field : _fields) {
    fields.add(Json(zone, field));
  }
  json("fields", fields)
      ("minLength", Json(zone, static_cast<double>(_minWordLength)));
  return json;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief callback function called by the fulltext index to determine the
/// words to index for a specific document
////////////////////////////////////////////////////////////////////////////////

TRI_fulltext_wordlist_t* FulltextIndex::getWordlist (TRI_doc_mptr_t const* document) {
  // extract the shape
  TRI_shape_t const* shape;
  TRI_shaped_json_t shaped;
  TRI_shaped_json_t shapedJson;
  TRI_EXTRACT_SHAPED_JSON_MARKER(shaped, document->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  bool ok = TRI_ExtractShapedJsonVocShaper(_collection->getShaper(), &shaped, 0, _attribute, &shapedJson, &shape);  // ONLY IN INDEX, PROTECTED by RUNTIME

  if (! ok || shape == nullptr) {
    return nullptr;
  }

  // extract the string value for the indexed attribute
  char* text;
  size_t textLength;
  ok = TRI_StringValueShapedJson(shape, shapedJson._data.data, &text, &textLength);

  if (! ok) {
    return nullptr;
  }

  // parse the document text
  TRI_vector_string_t* words = TRI_get_words(text, textLength, (size_t) _minWordLength, (size_t) TRI_FULLTEXT_MAX_WORD_LENGTH, true);

  if (words == nullptr) {
    return nullptr;
  }

  TRI_fulltext_wordlist_t* wordlist = TRI_CreateWordlistFulltextIndex(words->_buffer, words->_length);

  if (wordlist == nullptr) {
    TRI_FreeVectorString(TRI_UNKNOWN_MEM_ZONE, words);
    return nullptr;
  }

  // this really is a hack, but it works well:
  // make the word list vector think it's empty and free it
  // this does not free the word list, that we have already over the result
  words->_length = 0;
  words->_buffer = nullptr;
  TRI_FreeVectorString(TRI_UNKNOWN_MEM_ZONE, words);

  return wordlist;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
