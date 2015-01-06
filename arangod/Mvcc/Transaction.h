////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC transaction class
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

#ifndef ARANGODB_MVCC_TRANSACTION_H
#define ARANGODB_MVCC_TRANSACTION_H 1

#include "Basics/Common.h"

struct TRI_vocbase_s;

namespace triagens {
  namespace mvcc {

    class TransactionManager;

// -----------------------------------------------------------------------------
// --SECTION--                                                 class Transaction
// -----------------------------------------------------------------------------

    class Transaction {

      public:

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief transaction id data type
////////////////////////////////////////////////////////////////////////////////

        typedef uint64_t IdType;

////////////////////////////////////////////////////////////////////////////////
/// @brief transaction id
////////////////////////////////////////////////////////////////////////////////

        struct TransactionId {
          explicit TransactionId (IdType id) 
            : _id(id) {
          }

          inline IdType id () const {
            return _id; // return the full id
          }

          inline IdType idPart () const {
            return _id & (0xfffffffffff00000ULL); // return only the upper 44 bits
          }
          
          inline IdType sequencePart () const {
            return _id & (0x00000000000fffffULL); // return only the lower 20 bits
          }

          std::string toString () const {
            std::string result(std::to_string(id()));
            return result;
          }
        
          private:

            IdType const _id;
        };

////////////////////////////////////////////////////////////////////////////////
/// @brief statuses that a transaction can have
////////////////////////////////////////////////////////////////////////////////

        enum class StatusType {
          ONGOING      = 1,
          COMMITTED    = 2,
          ROLLED_BACK  = 3
        };

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

        Transaction (Transaction const&) = delete;
        Transaction& operator= (Transaction const&) = delete;
      
      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new transaction, with id provided by the transaction
/// manager. creating a transaction is only allowed by the transaction manager
////////////////////////////////////////////////////////////////////////////////

        Transaction (TransactionManager*,
                     TransactionId const&,
                     struct TRI_vocbase_s*);

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction
////////////////////////////////////////////////////////////////////////////////

        virtual ~Transaction ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief return the transaction manager
////////////////////////////////////////////////////////////////////////////////

        inline TransactionManager* transactionManager () const {
          return _transactionManager;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief return the transaction status
////////////////////////////////////////////////////////////////////////////////

        inline Transaction::StatusType status () const {
          return _status;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the transaction is currently ongoing
////////////////////////////////////////////////////////////////////////////////

        inline bool isOngoing () const {
          return (_status == Transaction::StatusType::ONGOING);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the transaction has committed
////////////////////////////////////////////////////////////////////////////////

        inline bool isCommitted () const {
          return (_status == Transaction::StatusType::COMMITTED);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the transaction has rolled back
////////////////////////////////////////////////////////////////////////////////

        inline bool isRolledBack () const {
          return (_status == Transaction::StatusType::ROLLED_BACK);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief return the transaction id
////////////////////////////////////////////////////////////////////////////////

        inline TransactionId const& id () const {
          return _id;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief return the database used by the transaction
////////////////////////////////////////////////////////////////////////////////

        inline struct TRI_vocbase_s* vocbase () const {
          return _vocbase;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief commit the transaction
////////////////////////////////////////////////////////////////////////////////

        int commit ();

////////////////////////////////////////////////////////////////////////////////
/// @brief roll back the transaction
////////////////////////////////////////////////////////////////////////////////

        int rollback ();

////////////////////////////////////////////////////////////////////////////////
/// @brief get the transaction's parent transaction
////////////////////////////////////////////////////////////////////////////////

        virtual Transaction* parentTransaction () = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the transaction is a top-level transaction
////////////////////////////////////////////////////////////////////////////////

        virtual bool isTopLevel () const = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief get a string representation of the transaction
////////////////////////////////////////////////////////////////////////////////

        virtual std::string toString () const = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief append the transaction to an output stream
////////////////////////////////////////////////////////////////////////////////
    
        friend std::ostream& operator<< (std::ostream&, Transaction const*);

// -----------------------------------------------------------------------------
// --SECTION--                                               protected variables
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief the transaction manager
////////////////////////////////////////////////////////////////////////////////

        TransactionManager* _transactionManager;

////////////////////////////////////////////////////////////////////////////////
/// @brief the transaction's id
////////////////////////////////////////////////////////////////////////////////

        TransactionId const _id;

////////////////////////////////////////////////////////////////////////////////
/// @brief the database used in the transaction
////////////////////////////////////////////////////////////////////////////////

        struct TRI_vocbase_s* const _vocbase;

////////////////////////////////////////////////////////////////////////////////
/// @brief the transaction's status
////////////////////////////////////////////////////////////////////////////////

        StatusType _status;
    };

  }
}

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
