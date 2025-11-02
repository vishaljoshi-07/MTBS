#ifndef BANK_H
#define BANK_H

#include <map>
#include <vector>
#include <memory>
#include <mutex>
#include <string>
#include <atomic>
#include "account.h"
#include "transaction.h"
#include "thread_manager.h"

// Forward declarations
class TransactionProcessor;
class TransactionLogger;

/**
 * @brief Main banking system class that coordinates all operations
 * 
 * This class demonstrates the central coordination of multiple threads
 * and shared resources in a banking environment.
 */
class Bank {
public:
    // Bank configuration
    struct Config {
        std::string bankName;
        std::string bankCode;
        size_t maxAccounts;
        size_t maxConcurrentTransactions;
        bool enableAuditLogging;
        
        Config(const std::string& name = "MTBS Bank", 
               const std::string& code = "MTBS001",
               size_t maxAcc = 1000,
               size_t maxTrans = 100,
               bool audit = true);
    };
    
    // Constructor and destructor
    explicit Bank(const Config& config = Config());
    ~Bank();
    
    // Account management (THREAD-SAFE)
    std::string createAccount(const std::string& holderName, double initialBalance = 0.0);
    bool closeAccount(const std::string& accountNumber);
    std::shared_ptr<Account> getAccount(const std::string& accountNumber);
    std::vector<std::shared_ptr<Account>> getAllAccounts();
    
    // Transaction processing (THREAD-SAFE)
    bool processDeposit(const std::string& accountNumber, double amount, 
                       const std::string& description = "");
    bool processWithdraw(const std::string& accountNumber, double amount, 
                        const std::string& description = "");
    bool processTransfer(const std::string& fromAccount, const std::string& toAccount, 
                        double amount, const std::string& description = "");
    
    // Banking operations
    double getAccountBalance(const std::string& accountNumber);
    std::vector<Transaction> getAccountTransactions(const std::string& accountNumber);
    std::string getAccountStatus(const std::string& accountNumber);
    
    // System management
    void startBankingSystem();
    void stopBankingSystem();
    bool isSystemRunning() const;
    
    // Statistics and monitoring
    size_t getTotalAccounts() const;
    size_t getActiveAccounts() const;
    size_t getTotalTransactions() const;
    size_t getSuccessfulTransactions() const;
    size_t getFailedTransactions() const;
    
    // System information
    std::string getBankName() const;
    std::string getBankCode() const;
    std::string getSystemStatus() const;
    std::string getPerformanceReport() const;
    
    // Utility methods
    void generateSampleData(size_t accountCount = 5);
    void clearAllData();
    void exportTransactionLog(const std::string& filename);
    void importTransactionLog(const std::string& filename);

private:
    // Bank configuration
    Config config;
    
    // Account storage (SHARED RESOURCE - PROTECTED BY MUTEX)
    std::map<std::string, std::shared_ptr<Account>> accounts;
    mutable std::mutex accountsMutex;
    
    // Transaction processing components
    std::unique_ptr<TransactionProcessor> transactionProcessor;
    std::unique_ptr<TransactionLogger> transactionLogger;
    
    // Thread management
    std::unique_ptr<ThreadPool> threadPool;
    std::unique_ptr<WorkQueue> workQueue;
    std::unique_ptr<ThreadMonitor> threadMonitor;
    
    // System state
    std::atomic<bool> systemRunning;
    std::atomic<size_t> totalTransactions;
    std::atomic<size_t> successfulTransactions;
    std::atomic<size_t> failedTransactions;
    
    // Account counter (atomic for thread safety)
    std::atomic<unsigned long long> accountCounter;
    
    // Private helper methods
    std::string generateAccountNumber();
    bool validateAccountNumber(const std::string& accountNumber);
    bool validateTransaction(const std::string& accountNumber, double amount, 
                           const std::string& type);
    void logTransaction(const Transaction& transaction);
    void updateStatistics(bool success);
    
    // Thread safety helpers
    std::shared_ptr<Account> findAccountUnsafe(const std::string& accountNumber);
    void addAccountUnsafe(const std::string& accountNumber, std::shared_ptr<Account> account);
    void removeAccountUnsafe(const std::string& accountNumber);
    
    // Internal transaction processing
    void processTransactionAsync(const std::function<void()>& transactionTask, 
                               const std::string& description);
};

/**
 * @brief Banking system exception class
 * 
 * Custom exception for banking-specific errors with detailed information.
 */
class BankException : public std::exception {
public:
    enum class ErrorType {
        ACCOUNT_NOT_FOUND,
        INSUFFICIENT_FUNDS,
        INVALID_AMOUNT,
        ACCOUNT_CLOSED,
        TRANSACTION_FAILED,
        SYSTEM_ERROR,
        INVALID_ACCOUNT_NUMBER,
        DUPLICATE_ACCOUNT
    };
    
    BankException(ErrorType type, const std::string& message, 
                  const std::string& accountNumber = "");
    
    const char* what() const noexcept override;
    ErrorType getErrorType() const;
    std::string getAccountNumber() const;
    std::string getErrorMessage() const;

private:
    ErrorType errorType;
    std::string errorMessage;
    std::string accountNumber;
    std::string fullMessage;
};

// Utility functions for banking operations
namespace BankUtils {
    // Validate banking data
    bool isValidBankCode(const std::string& code);
    bool isValidAccountHolderName(const std::string& name);
    bool isValidInitialBalance(double balance);
    
    // Format banking information
    std::string formatAccountNumber(const std::string& number);
    std::string formatBankCode(const std::string& code);
    std::string formatCurrency(double amount);
    
    // Generate banking identifiers
    std::string generateBankCode();
    std::string generateAccountNumber();
    std::string generateTransactionId();
    
    // Banking calculations
    double calculateInterest(double principal, double rate, int months);
    double calculateTransactionFee(double amount, const std::string& type);
    double calculateMinimumBalance(const std::string& accountType);
}

#endif // BANK_H

