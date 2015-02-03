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
#include "Basics/FlagsType.h"
#include "Mvcc/CollectionStats.h"
#include "Mvcc/TransactionId.h"
#include "VocBase/voc-types.h"

struct TRI_vocbase_s;

namespace triagens {
  namespace mvcc {

    class TopLevelTransaction;
    class TransactionCollection;
    class TransactionManager;

// -----------------------------------------------------------------------------
// --SECTION--                                            struct TransactionInfo
// -----------------------------------------------------------------------------

    struct TransactionInfo {
      TransactionInfo (TransactionId const& id,
                       double startTime) 
        : id(id),
          startTime(startTime) {
      }
      
      TransactionInfo (TransactionInfo const& other) 
        : id(other.id),
          startTime(other.startTime) {
      }
      
      TransactionInfo& operator= (TransactionInfo const& other) {
        id = other.id;
        startTime = other.startTime;
        return *this;
      }

      TransactionId id;
      double        startTime;
    };

// -----------------------------------------------------------------------------
// --SECTION--                                                 class Transaction
// -----------------------------------------------------------------------------

    class Transaction {

      friend class TransactionManager;
      friend class LocalTransactionManager;
      friend class SubTransaction;

      public:

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief statuses that a transaction can have
////////////////////////////////////////////////////////////////////////////////

        enum class StatusType : uint16_t {
          ONGOING      = 1,
          COMMITTED    = 2,
          ROLLED_BACK  = 3
        };

////////////////////////////////////////////////////////////////////////////////
/// @brief statuses that a transaction can have
////////////////////////////////////////////////////////////////////////////////

        enum class VisibilityType : uint16_t {
          INVISIBLE  = 1,
          CONCURRENT = 2,
          VISIBLE    = 3
        };

////////////////////////////////////////////////////////////////////////////////
/// @brief transaction flags
////////////////////////////////////////////////////////////////////////////////

        struct TransactionFlags : triagens::basics::FlagsType<uint32_t> {
          TransactionFlags ()
            : FlagsType() {
          }
          
          inline bool initialized () const {
            return hasFlag(Initialized);
          }

          inline bool beginMarkerWritten () const {
            return hasFlag(BeginMarkerWritten);
          }

          inline bool dataMarkerWritten () const {
            return hasFlag(DataMarkerWritten);
          }
          
          inline bool endMarkerWritten () const {
            return hasFlag(EndMarkerWritten);
          }

          inline bool pushedOnThreadStack () const {
            return hasFlag(PushedOnThreadStack);
          }
          
          void initialized (bool value) {
            TRI_ASSERT(! hasFlag(Initialized));
            setFlag(Initialized);
          }

          void beginMarkerWritten (bool value) {
            TRI_ASSERT(! hasFlag(BeginMarkerWritten));
            TRI_ASSERT(! hasFlag(DataMarkerWritten));
            TRI_ASSERT(! hasFlag(EndMarkerWritten));

            setFlag(BeginMarkerWritten);
          }
          
          void dataMarkerWritten (bool value) {
            setFlag(DataMarkerWritten);
          }
          
          void endMarkerWritten (bool value) {
            TRI_ASSERT(hasFlag(BeginMarkerWritten));
            TRI_ASSERT(! hasFlag(EndMarkerWritten));

            setFlag(EndMarkerWritten);
          }
          
          void pushedOnThreadStack (bool value) {
            setFlag(PushedOnThreadStack, value);
          }

          static uint32_t const Initialized          = 0x01;
          static uint32_t const BeginMarkerWritten   = 0x02;
          static uint32_t const DataMarkerWritten    = 0x04;
          static uint32_t const EndMarkerWritten     = 0x08;
          static uint32_t const PushedOnThreadStack  = 0x10;
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
/// @brief return the transaction start time
////////////////////////////////////////////////////////////////////////////////

        inline double startTime () const {
          return _startTime;
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
/// @brief update the number of inserted documents
////////////////////////////////////////////////////////////////////////////////

        void incNumInserted (TransactionCollection const*,
                             bool);

////////////////////////////////////////////////////////////////////////////////
/// @brief update the number of removed documents
////////////////////////////////////////////////////////////////////////////////

        void incNumRemoved (TransactionCollection const*,
                            bool);

////////////////////////////////////////////////////////////////////////////////
/// @brief return the transaction status
////////////////////////////////////////////////////////////////////////////////

        Transaction::StatusType status () const; 

////////////////////////////////////////////////////////////////////////////////
/// @brief commit the transaction
////////////////////////////////////////////////////////////////////////////////

        virtual int commit () = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief roll back the transaction
////////////////////////////////////////////////////////////////////////////////

        virtual int rollback () = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief returns aggregated transaction statistics
////////////////////////////////////////////////////////////////////////////////

        virtual CollectionStats aggregatedStats (TRI_voc_cid_t) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a collection used in the transaction
/// this registers the collection in the transaction if not yet present
////////////////////////////////////////////////////////////////////////////////
        
        virtual TransactionCollection* collection (std::string const&) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a collection used in the transaction
/// this registers the collection in the transaction if not yet present
////////////////////////////////////////////////////////////////////////////////
        
        virtual TransactionCollection* collection (TRI_voc_cid_t) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief get the transaction's top-level transaction
////////////////////////////////////////////////////////////////////////////////

        virtual TopLevelTransaction* topLevelTransaction () = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief get the transaction's direct parent transaction
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
    
        friend std::ostream& operator<< (std::ostream&, Transaction const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief visibility, this implements the MVCC logic of what this transaction
/// can see, returns the visibility of the other transaction for this one.
/// The result can be INVISIBLE, INCONFLICT or VISIBLE. We guarantee
/// INVISIBLE < INCONFLICT < VISIBLE, such that one can do things like
/// "visibility(other) < VISIBLE" legally.
////////////////////////////////////////////////////////////////////////////////
    
         VisibilityType visibility (TransactionId const& other);

         bool isVisibleForRead (TransactionId const& from,
                                TransactionId const& to);

////////////////////////////////////////////////////////////////////////////////
/// @brief wasOngoingAtStart, check whether or not another transaction was
/// ongoing when this one started
////////////////////////////////////////////////////////////////////////////////

         bool wasOngoingAtStart (TransactionId const& other);

////////////////////////////////////////////////////////////////////////////////
/// @brief isNotAborted, this is a pure optimisation method. It checks
/// whether another transaction was aborted. It is only legal to call
/// this on transactions that were already committed or aborted before
/// this transaction has started. If the method returns true, then other
/// is guaranteed to be a transaction that has committed before this
/// transaction was started. The method may return false even if the
/// transaction with id other has committed. This is a method of the
/// Transaction class to allow for clever optimisations using a bloom
/// filter.
////////////////////////////////////////////////////////////////////////////////

         bool isNotAborted(TransactionId const& other) {
           return false;  // Optimise later
         }

////////////////////////////////////////////////////////////////////////////////
/// @brief set the killed flag
////////////////////////////////////////////////////////////////////////////////

        void killed (bool);

////////////////////////////////////////////////////////////////////////////////
/// @brief check the killed flag
////////////////////////////////////////////////////////////////////////////////

        bool killed ();

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief initialize and fetch the current state from the transaction manager
////////////////////////////////////////////////////////////////////////////////

        void initializeState ();

////////////////////////////////////////////////////////////////////////////////
/// @brief this is called when a subtransaction is finished
////////////////////////////////////////////////////////////////////////////////

        void updateSubTransaction (Transaction*);

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
/// @brief the transaction's start time
////////////////////////////////////////////////////////////////////////////////

        double const _startTime;

////////////////////////////////////////////////////////////////////////////////
/// @brief the transaction's status
////////////////////////////////////////////////////////////////////////////////

        StatusType _status;

////////////////////////////////////////////////////////////////////////////////
/// @brief various flags
////////////////////////////////////////////////////////////////////////////////

        TransactionFlags _flags;

////////////////////////////////////////////////////////////////////////////////
/// @brief collection modification statistics
////////////////////////////////////////////////////////////////////////////////

        std::unordered_map<TRI_voc_cid_t, CollectionStats> _stats;

////////////////////////////////////////////////////////////////////////////////
/// @brief list of all sub-transactions with their statuses
////////////////////////////////////////////////////////////////////////////////

        std::vector<std::pair<TransactionId::InternalType, StatusType>> _subTransactions; 

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the transaction was killed by another thread
////////////////////////////////////////////////////////////////////////////////

        bool _killed;

    };
  }
}

#endif
