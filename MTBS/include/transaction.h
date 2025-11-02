#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <string>
#include <memory>
#include <functional>
#include <chrono>
#include <atomic>
#include <condition_variable>
#include <fstream>
#include "account.h"

// Forward declarations
class Bank;
class Account;

/**
 * @brief Transaction processor that handles individual banking operations
 * 
 * Each transaction is processed in its own thread, demonstrating
 * concurrent execution while maintaining data integrity.
 */
class TransactionProcessor {
public:
    // Transaction result structure
    struct TransactionResult {
        bool success;
        std::string message;
        std::string transactionId;
        double newBalance;
        std::chrono::system_clock::time_point timestamp;
        
        TransactionResult(bool s, const std::string& msg, const std::string& id, 
                         double balance, std::chrono::system_clock::time_point time);
    };
    
    // Constructor and destructor
    TransactionProcessor();
    ~TransactionProcessor();
    
    // Main transaction processing methods
    TransactionResult processDeposit(Account& account, double amount, 
                                   const std::string& description = "");
    TransactionResult processWithdraw(Account& account, double amount, 
                                    const std::string& description = "");
    TransactionResult processTransfer(Account& fromAccount, Account& toAccount, 
                                    double amount, const std::string& description = "");
    TransactionResult processBalanceCheck(const Account& account);
    
    // Transaction validation
    bool validateTransaction(const Account& account, double amount, 
                           const std::string& type);
    
    // Utility methods
    std::string generateTransactionId();
    std::string getCurrentTimestamp();
    void logTransaction(const Transaction& transaction);

private:
    // Transaction ID counter (atomic for thread safety)
    static std::atomic<unsigned long long> transactionCounter;
    
    // Validation helpers
    bool isValidAmount(double amount);
    bool hasSufficientFunds(const Account& account, double amount);
    bool isAccountActive(const Account& account);
};

/**
 * @brief Transaction queue manager for handling concurrent transactions
 * 
 * Manages a queue of pending transactions and distributes them
 * to available worker threads. Demonstrates producer-consumer pattern.
 */
class TransactionQueue {
public:
    // Transaction task structure
    struct TransactionTask {
        std::function<TransactionProcessor::TransactionResult()> task;
        std::string description;
        std::chrono::system_clock::time_point queuedAt;
        
        TransactionTask(const std::function<TransactionProcessor::TransactionResult()>& t,
                       const std::string& desc);
    };
    
    // Constructor and destructor
    TransactionQueue(size_t maxQueueSize = 1000);
    ~TransactionQueue();
    
    // Queue management
    bool enqueueTransaction(const TransactionTask& task);
    bool dequeueTransaction(TransactionTask& task);
    
    // Queue status
    size_t getQueueSize() const;
    bool isEmpty() const;
    bool isFull() const;
    
    // Queue monitoring
    void clearQueue();
    std::vector<std::string> getQueueStatus() const;

private:
    std::vector<TransactionTask> taskQueue;
    mutable std::mutex queueMutex;
    std::condition_variable queueCondition;
    size_t maxSize;
    
    // Queue statistics
    std::atomic<size_t> totalProcessed;
    std::atomic<size_t> totalFailed;
};

/**
 * @brief Transaction logger for recording all banking operations
 * 
 * Thread-safe logging system that maintains a complete audit trail
 * of all transactions for compliance and debugging purposes.
 */
class TransactionLogger {
public:
    // Log levels
    enum class LogLevel {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };
    
    // Constructor and destructor
    TransactionLogger(const std::string& logFilePath = "bank_transactions.log");
    ~TransactionLogger();
    
    // Logging methods
    void logTransaction(const Transaction& transaction);
    void logMessage(LogLevel level, const std::string& message);
    void logError(const std::string& error, const std::string& context = "");
    
    // Log management
    void setLogLevel(LogLevel level);
    void flushLogs();
    void rotateLogFile();
    
    // Log retrieval
    std::vector<std::string> getRecentTransactions(size_t count = 100);
    std::vector<std::string> searchTransactions(const std::string& searchTerm);
    
    // Statistics
    size_t getTotalTransactions() const;
    size_t getSuccessfulTransactions() const;
    size_t getFailedTransactions() const;

private:
    std::string logFile;
    std::ofstream logStream;
    LogLevel currentLevel;
    mutable std::mutex loggerMutex;
    
    // Log statistics
    std::atomic<size_t> totalLogged;
    std::atomic<size_t> successfulLogged;
    std::atomic<size_t> failedLogged;
    
    // Helper methods
    std::string levelToString(LogLevel level);
    std::string formatLogEntry(LogLevel level, const std::string& message);
    void writeToFile(const std::string& entry);
};

// Utility functions for transaction processing
namespace TransactionUtils {
    // Generate unique transaction IDs
    std::string generateUniqueId();
    
    // Format currency amounts
    std::string formatCurrency(double amount);
    
    // Validate account numbers
    bool isValidAccountNumber(const std::string& accountNumber);
    
    // Calculate transaction fees
    double calculateTransactionFee(double amount, const std::string& type);
    
    // Generate transaction summaries
    std::string generateTransactionSummary(const std::vector<Transaction>& transactions);
}

#endif // TRANSACTION_H

