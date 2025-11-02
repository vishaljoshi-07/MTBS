#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <string>
#include <vector>
#include <mutex>
#include <chrono>
#include <atomic>

// Forward declaration
struct Transaction;

/**
 * @brief Represents a bank account with thread-safe operations
 * 
 * This class demonstrates critical section protection using mutex locks.
 * All balance modifications are protected to prevent race conditions.
 */
class Account {
private:
    std::string accountNumber;      // Unique account identifier
    std::string accountHolderName;  // Name of account holder
    double balance;                 // Current account balance
    std::vector<Transaction> transactionHistory; // Transaction log
    
    // Mutex for protecting shared resources (CRITICAL SECTION)
    mutable std::mutex accountMutex;
    
    // Timestamp for account creation
    std::chrono::system_clock::time_point createdAt;

public:
    // Constructor and destructor
    Account(const std::string& number, const std::string& holderName, double initialBalance = 0.0);
    ~Account() = default;
    
    // Copy constructor and assignment operator (disabled for thread safety)
    Account(const Account&) = delete;
    Account& operator=(const Account&) = delete;
    
    // Getters (const methods, use shared_lock for better performance)
    std::string getAccountNumber() const;
    std::string getAccountHolderName() const;
    double getBalance() const;
    std::vector<Transaction> getTransactionHistory() const;
    std::chrono::system_clock::time_point getCreatedAt() const;
    
    // Thread-safe balance operations (CRITICAL SECTIONS)
    bool deposit(double amount, const std::string& description = "");
    bool withdraw(double amount, const std::string& description = "");
    bool transfer(Account& targetAccount, double amount, const std::string& description = "");
    
    // Transaction history management
    void addTransaction(const Transaction& transaction);
    void clearTransactionHistory();
    
    // Account status
    bool isActive() const;
    std::string getStatus() const;
    
    // Utility methods
    std::string toString() const;
    void printDetails() const;
};

/**
 * @brief Transaction record structure
 * 
 * Immutable structure representing a single banking transaction.
 * Once created, transaction details cannot be modified.
 */
struct Transaction {
    const std::string transactionId;        // Unique transaction identifier
    const std::string fromAccount;          // Source account number
    const std::string toAccount;            // Destination account number
    const std::string type;                 // Transaction type (DEPOSIT, WITHDRAW, TRANSFER)
    const double amount;                    // Transaction amount
    const std::string description;          // Transaction description
    const std::chrono::system_clock::time_point timestamp; // When transaction occurred
    mutable std::string status;             // Transaction status (SUCCESS, FAILED, PENDING)
    
    // Static counter for generating unique transaction IDs
    static std::atomic<unsigned long long> transactionCounter;
    
    // Constructor
    Transaction(const std::string& id, const std::string& from, const std::string& to,
                const std::string& t, double amt, const std::string& desc);
    
    // Utility methods
    std::string toString() const;
    std::string getFormattedTimestamp() const;
    bool isSuccessful() const;
};

// Transaction types constants
namespace TransactionType {
    const std::string DEPOSIT = "DEPOSIT";
    const std::string WITHDRAW = "WITHDRAW";
    const std::string TRANSFER = "TRANSFER";
    const std::string BALANCE_CHECK = "BALANCE_CHECK";
}

// Transaction status constants
namespace TransactionStatus {
    const std::string SUCCESS = "SUCCESS";
    const std::string FAILED = "FAILED";
    const std::string PENDING = "PENDING";
    const std::string INSUFFICIENT_FUNDS = "INSUFFICIENT_FUNDS";
    const std::string INVALID_ACCOUNT = "INVALID_ACCOUNT";
}

#endif // ACCOUNT_H

