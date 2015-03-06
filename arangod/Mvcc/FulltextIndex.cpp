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
#include "Basics/ReadLocker.h"
#include "Basics/WriteLocker.h"
#include "FulltextIndex/fulltext-index.h"
#include "FulltextIndex/fulltext-query.h"
#include "FulltextIndex/fulltext-result.h"
#include "FulltextIndex/fulltext-wordlist.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionId.h"
#include "Utils/Exception.h"
#include "VocBase/document-collection.h"
#include "VocBase/voc-shaper.h"

using namespace triagens::basics;
using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                               class FulltextIndex
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief default minimum word length
////////////////////////////////////////////////////////////////////////////////
  
int const FulltextIndex::DefaultMinWordLength = 2;

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
  : Index(id, fields),
    _collection(collection),
    _lock(),
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
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief query the fulltext index
////////////////////////////////////////////////////////////////////////////////

std::vector<TRI_doc_mptr_t*>* FulltextIndex::query (Transaction* transaction,
                                                    std::string const& queryString) {
  
  TRI_fulltext_query_t* query = TRI_CreateQueryFulltextIndex(TRI_FULLTEXT_SEARCH_MAX_WORDS);

  if (query == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }
  
  int res = TRI_ParseQueryFulltextIndex(query, queryString.c_str());

  if (res != TRI_ERROR_NO_ERROR) {
    TRI_FreeQueryFulltextIndex(query);
    THROW_ARANGO_EXCEPTION(res);
  }
  
  TRI_fulltext_result_t* queryResult = nullptr;
  {
    READ_LOCKER(_lock);
    // note: TRI_QueryFulltextIndex will free its query argument!
    queryResult = TRI_QueryFulltextIndex(_fulltextIndex, query);
  }
    
  if (queryResult == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }

  std::unique_ptr<std::vector<TRI_doc_mptr_t*>> theResult(new std::vector<TRI_doc_mptr_t*>);
  try {
    theResult->reserve(queryResult->_numDocuments);

    for (size_t i = 0; i < queryResult->_numDocuments; ++i) {
      TRI_doc_mptr_t* document = reinterpret_cast<TRI_doc_mptr_t*>(queryResult->_documents[i]);

      TransactionId from = document->from();
      TransactionId to = document->to();
      if (transaction->isVisibleForRead(from, to)) {
        theResult->emplace_back(document);
      }
    }
  }
  catch (...) {
    TRI_FreeResultFulltextIndex(queryResult);
    throw;
  }
    
  TRI_FreeResultFulltextIndex(queryResult);

  return theResult.release();
}

// -----------------------------------------------------------------------------
// --SECTION--                                            public virtual methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document when loading a collection
////////////////////////////////////////////////////////////////////////////////

void FulltextIndex::insert (TRI_doc_mptr_t* doc) {
  insert(nullptr, nullptr, doc);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document into the index
////////////////////////////////////////////////////////////////////////////////
        
void FulltextIndex::insert (TransactionCollection*,
                            Transaction*,
                            TRI_doc_mptr_t* doc) {
  TRI_fulltext_wordlist_t* wordlist = getWordlist(doc);

  if (wordlist == nullptr) {
    // TODO: distinguish the cases "empty wordlist" and "out of memory"
    return;
  }
    
  if (wordlist->_numWords == 0) {
    TRI_FreeWordlistFulltextIndex(wordlist);
    return;
  }

  try {
    int res = TRI_ERROR_NO_ERROR;
    {
      WRITE_LOCKER(_lock);

      if (! TRI_InsertWordsFulltextIndex(_fulltextIndex, (TRI_fulltext_doc_t) ((uintptr_t) doc), wordlist)) {
        res = TRI_ERROR_INTERNAL;
      }
    }

    TRI_FreeWordlistFulltextIndex(wordlist);

    if (res != TRI_ERROR_NO_ERROR) {
      THROW_ARANGO_EXCEPTION(res);
    }
  }
  catch (...) {
    TRI_FreeWordlistFulltextIndex(wordlist);
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document from the index
////////////////////////////////////////////////////////////////////////////////

TRI_doc_mptr_t* FulltextIndex::remove (TransactionCollection*,
                                       Transaction*,
                                       std::string const&,
                                       TRI_doc_mptr_t* doc) {
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief forget a document in the index
////////////////////////////////////////////////////////////////////////////////

void FulltextIndex::forget (TransactionCollection*, 
                            Transaction*,
                            TRI_doc_mptr_t* doc) {
  WRITE_LOCKER(_lock);
  TRI_DeleteDocumentFulltextIndex(_fulltextIndex, (TRI_fulltext_doc_t) ((uintptr_t) doc));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief post-insert operation (does nothing for this index type)
////////////////////////////////////////////////////////////////////////////////

void FulltextIndex::preCommit (TransactionCollection*,
                               Transaction*) {
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
  READ_LOCKER(_lock);
  return TRI_MemoryFulltextIndex(_fulltextIndex);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get the memory usage of the index
////////////////////////////////////////////////////////////////////////////////

Json FulltextIndex::toJson (TRI_memory_zone_t* zone) const {
  Json json = Index::toJson(zone);
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
