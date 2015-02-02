////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC transactions stack
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

#ifndef ARANGODB_MVCC_TRANSACTION_STACK_ACCESSOR_H
#define ARANGODB_MVCC_TRANSACTION_STACK_ACCESSOR_H 1

#include "Basics/Common.h"
#include "Mvcc/Transaction.h"

namespace triagens {
  namespace mvcc {

    class Transaction;

// -----------------------------------------------------------------------------
// --SECTION--                                    class TransactionStackAccessor
// -----------------------------------------------------------------------------

    class TransactionStackAccessor {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

        TransactionStackAccessor (TransactionStackAccessor const&) = delete;
        TransactionStackAccessor& operator= (TransactionStackAccessor const&) = delete;
      
////////////////////////////////////////////////////////////////////////////////
/// @brief create a stack accessor instance
////////////////////////////////////////////////////////////////////////////////

        TransactionStackAccessor ();

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the stack accessor
////////////////////////////////////////////////////////////////////////////////

        ~TransactionStackAccessor ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief check if the stack is empty
////////////////////////////////////////////////////////////////////////////////

        inline bool isEmpty () const {
          return _threadTransactions->empty();
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief return the top of stack
////////////////////////////////////////////////////////////////////////////////

        inline Transaction* peek () const {
          if (isEmpty()) {
            return nullptr;
          }

          return _threadTransactions->back();
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief push a transaction on the stack
////////////////////////////////////////////////////////////////////////////////

        void push (Transaction* transaction) {
          _threadTransactions->push_back(transaction);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief pop a transaction from the stack
////////////////////////////////////////////////////////////////////////////////

        Transaction* pop () {
          if (isEmpty()) {
            return nullptr;
          }

          auto result = _threadTransactions->back();
          _threadTransactions->pop_back();
          return result;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief check if a transaction is present on the stack
////////////////////////////////////////////////////////////////////////////////

        bool isOnStack (TransactionId::IdType id) const {
          size_t i = _threadTransactions->size();
          while (i > 0) {
            --i;
            if ((* _threadTransactions)[i]->id()() == id) {
              return true;
            }
          }
          return false;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief check if the stack is empty
////////////////////////////////////////////////////////////////////////////////

        static inline bool IsEmpty () {
          if (_threadTransactions == nullptr) {
            return true;
          }

          return _threadTransactions->empty();
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief thread-local vector of started transactions
////////////////////////////////////////////////////////////////////////////////
    
        static thread_local std::vector<Transaction*>* _threadTransactions;

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
