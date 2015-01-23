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

namespace triagens {
  namespace mvcc {

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief transaction id
/// a transaction id is a 64 bit unsigned integer that is internally split into
/// two components: 
/// - the upper 56 bits contain the top-level transaction id
/// - the lower 8 bits contain the sub-transaction id within the top-level
///   transaction
/// the 56 bits should be enough to run approx. 114M transactions per second 
/// about 20 years: (1 << (64 - 8)) / (20 * 365 * 24 * 60 * 60)
////////////////////////////////////////////////////////////////////////////////

    struct TransactionId {
      typedef uint64_t IdType;

////////////////////////////////////////////////////////////////////////////////
/// @brief create a transaction id from an existing integer value
////////////////////////////////////////////////////////////////////////////////

      explicit TransactionId (IdType id)
        : _id(id) {
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief create a transaction id from two integer components
/// this will throw if the sub-transaction value is too high
////////////////////////////////////////////////////////////////////////////////
      
      TransactionId (IdType mainPart,
                     IdType sequencePart)
        : TransactionId(MainPart(mainPart) | SequencePart(sequencePart)) {
        // check if the sub-transaction part got too high
        if (sequencePart > SequenceMask) {
          THROW_ARANGO_EXCEPTION(TRI_ERROR_TRANSACTION_ID_OVERFLOW);
        }
      }

      IdType operator () () const {
        return _id;
      }
      
      inline IdType mainPart () const {
        return MainPart(_id);
      }
      
      inline IdType sequencePart () const {
        return SequencePart(_id);
      }
      
      inline bool isSameTransaction (IdType other) const {
        return isSameTransaction(_id, other);
      }

      static inline IdType MainPart (IdType id) {
        return (id & MainMask);
      }

      static inline IdType SequencePart (IdType id) {
        return (id & SequenceMask);
      }
      
      static inline bool isSameTransaction (IdType lhs, IdType rhs) {
        return (MainPart(lhs) == MainPart(rhs));
      }

      friend std::ostream& operator<< (std::ostream&, TransactionId const*);
    
      friend std::ostream& operator<< (std::ostream&, TransactionId const&);

      IdType const _id;

      static IdType const MainMask      = 0xffffffffffffff00ULL;
      static IdType const SequenceMask  = 0x00000000000000ffULL;
    };

  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hash function for TransactionId objects
////////////////////////////////////////////////////////////////////////////////

namespace std {
  template<> struct hash<triagens::mvcc::TransactionId> {
    size_t operator () (triagens::mvcc::TransactionId const& value) const {
      return std::hash<triagens::mvcc::TransactionId::IdType>()(value._id);
    }
  };

  template<> struct equal_to<triagens::mvcc::TransactionId> {
    bool operator () (triagens::mvcc::TransactionId const& lhs,
                      triagens::mvcc::TransactionId const& rhs) const {
      return lhs._id == rhs._id;
    }
  };

}

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
