////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC transaction id struct
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

#ifndef ARANGODB_MVCC_TRANSACTION_ID_H
#define ARANGODB_MVCC_TRANSACTION_ID_H 1

#include "Basics/Common.h"
#include "Utils/Exception.h"
#include "VocBase/voc-types.h"

namespace triagens {
  namespace mvcc {

    struct TransactionId {

      typedef TRI_voc_tid_t InternalType;

////////////////////////////////////////////////////////////////////////////////
/// @brief create a transaction id from two integer components
////////////////////////////////////////////////////////////////////////////////
      
      TransactionId () 
        : ownTransactionId(0),
          topTransactionId(0) {
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief create a transaction id from two integer components
////////////////////////////////////////////////////////////////////////////////
      
      TransactionId (InternalType ownTransactionId,
                     InternalType topTransactionId)
        : ownTransactionId(ownTransactionId),
          topTransactionId(topTransactionId) {
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief create a transaction id from another
////////////////////////////////////////////////////////////////////////////////
      
      TransactionId (TransactionId const& other) 
        : ownTransactionId(other.ownTransactionId),
          topTransactionId(other.topTransactionId) {
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief create a transaction id from another
////////////////////////////////////////////////////////////////////////////////
      
      TransactionId& operator= (TransactionId const& other) {
        ownTransactionId = other.ownTransactionId;
        topTransactionId = other.topTransactionId;
        return *this;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief return the own transaction id
////////////////////////////////////////////////////////////////////////////////

      inline InternalType own () const {
        return ownTransactionId;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief return the top transaction id
////////////////////////////////////////////////////////////////////////////////
      
      inline InternalType top () const {
        return topTransactionId;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief reset the ids
////////////////////////////////////////////////////////////////////////////////

      inline void reset () {
        ownTransactionId = 0;
        topTransactionId = 0;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief get a string representation of the id
////////////////////////////////////////////////////////////////////////////////

      std::string toString () const {
        return std::to_string(ownTransactionId) + " (" + std::to_string(topTransactionId) + ")";
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief append the transaction id to an output stream
////////////////////////////////////////////////////////////////////////////////

      friend std::ostream& operator<< (std::ostream&, TransactionId const*);
    
      friend std::ostream& operator<< (std::ostream&, TransactionId const&);

      InternalType mutable ownTransactionId;
      InternalType mutable topTransactionId;
    };

  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hash function for TransactionId objects
////////////////////////////////////////////////////////////////////////////////
/*
namespace std {
  template<> struct hash<triagens::mvcc::TransactionId> {
    size_t operator () (triagens::mvcc::TransactionId const& id) const {
      return std::hash<TRI_voc_tid_t>()(id.ownTransactionId);
    }
  };

  template<> struct equal_to<triagens::mvcc::TransactionId> {
    bool operator () (triagens::mvcc::TransactionId const& lhs,
                      triagens::mvcc::TransactionId const& rhs) const {
      return (lhs.ownTransactionId == rhs.ownTransactionId);
    }
  };

}
*/

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
