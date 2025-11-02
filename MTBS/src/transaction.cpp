#include "../include/transaction.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <queue>
#include <condition_variable>
#include <fstream>

// ============================================================================
// STATIC MEMBER INITIALIZATION
// ============================================================================

std::atomic<unsigned long long> TransactionProcessor::transactionCounter(0);

// ============================================================================
// TRANSACTION RESULT IMPLEMENTATION
// ============================================================================

TransactionProcessor::TransactionResult::TransactionResult(bool s, const std::string& msg, 
                                                         const std::string& id, double balance, 
                                                         std::chrono::system_clock::time_point time)
    : success(s), message(msg), transactionId(id), newBalance(balance), timestamp(time) {
}

// ============================================================================
// TRANSACTION PROCESSOR IMPLEMENTATION
// ============================================================================

TransactionProcessor::TransactionProcessor() {
    // Initialize transaction counter if needed
}

TransactionProcessor::~TransactionProcessor() = default;

TransactionProcessor::TransactionResult TransactionProcessor::processDeposit(Account& account, 
                                                                          double amount, 
                                                                          const std::string& description) {
    // Validate transaction
    if (!validateTransaction(account, amount, "DEPOSIT")) {
        return TransactionResult(false, "Invalid deposit transaction", 
                               generateTransactionId(), account.getBalance(), 
                               std::chrono::system_clock::now());
    }
    
    // Process deposit
    bool success = account.deposit(amount, description);
    double newBalance = account.getBalance();
    
    if (success) {
        return TransactionResult(true, "Deposit successful", 
                               generateTransactionId(), newBalance, 
                               std::chrono::system_clock::now());
    } else {
        return TransactionResult(false, "Deposit failed", 
                               generateTransactionId(), newBalance, 
                               std::chrono::system_clock::now());
    }
}

TransactionProcessor::TransactionResult TransactionProcessor::processWithdraw(Account& account, 
                                                                           double amount, 
                                                                           const std::string& description) {
    // Validate transaction
    if (!validateTransaction(account, amount, "WITHDRAW")) {
        return TransactionResult(false, "Invalid withdrawal transaction", 
                               generateTransactionId(), account.getBalance(), 
                               std::chrono::system_clock::now());
    }
    
    // Process withdrawal
    bool success = account.withdraw(amount, description);
    double newBalance = account.getBalance();
    
    if (success) {
        return TransactionResult(true, "Withdrawal successful", 
                               generateTransactionId(), newBalance, 
                               std::chrono::system_clock::now());
    } else {
        return TransactionResult(false, "Insufficient funds for withdrawal", 
                               generateTransactionId(), newBalance, 
                               std::chrono::system_clock::now());
    }
}

TransactionProcessor::TransactionResult TransactionProcessor::processTransfer(Account& fromAccount, 
                                                                            Account& toAccount, 
                                                                            double amount, 
                                                                            const std::string& description) {
    // Validate transaction
    if (!validateTransaction(fromAccount, amount, "TRANSFER")) {
        return TransactionResult(false, "Invalid transfer transaction", 
                               generateTransactionId(), fromAccount.getBalance(), 
                               std::chrono::system_clock::now());
    }
    
    // Process transfer
    bool success = fromAccount.transfer(toAccount, amount, description);
    double newBalance = fromAccount.getBalance();
    
    if (success) {
        return TransactionResult(true, "Transfer successful", 
                               generateTransactionId(), newBalance, 
                               std::chrono::system_clock::now());
    } else {
        return TransactionResult(false, "Transfer failed - insufficient funds", 
                               generateTransactionId(), newBalance, 
                               std::chrono::system_clock::now());
    }
}

TransactionProcessor::TransactionResult TransactionProcessor::processBalanceCheck(const Account& account) {
    double balance = account.getBalance();
    return TransactionResult(true, "Balance check successful", 
                           generateTransactionId(), balance, 
                           std::chrono::system_clock::now());
}

bool TransactionProcessor::validateTransaction(const Account& account, double amount, 
                                             const std::string& type) {
    // Check if account is active
    if (!isAccountActive(account)) {
        return false;
    }
    
    // Validate amount
    if (!isValidAmount(amount)) {
        return false;
    }
    
    // Check sufficient funds for withdrawals and transfers
    if ((type == "WITHDRAW" || type == "TRANSFER") && !hasSufficientFunds(account, amount)) {
        return false;
    }
    
    return true;
}

std::string TransactionProcessor::generateTransactionId() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::ostringstream oss;
    oss << "TXN_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S")
        << "_" << std::setfill('0') << std::setw(3) << ms.count()
        << "_" << ++transactionCounter;
    
    return oss.str();
}

std::string TransactionProcessor::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void TransactionProcessor::logTransaction(const Transaction& transaction) {
    std::cout << "[" << getCurrentTimestamp() << "] " 
              << "Transaction: " << transaction.transactionId 
              << " - " << transaction.type 
              << " $" << std::fixed << std::setprecision(2) << transaction.amount
              << " - Status: " << transaction.status << std::endl;
}

bool TransactionProcessor::isValidAmount(double amount) {
    return amount > 0 && amount < 1000000000; // Reasonable banking limits
}

bool TransactionProcessor::hasSufficientFunds(const Account& account, double amount) {
    return account.getBalance() >= amount;
}

bool TransactionProcessor::isAccountActive(const Account& account) {
    return account.isActive();
}

// ============================================================================
// TRANSACTION QUEUE IMPLEMENTATION
// ============================================================================

TransactionQueue::TransactionTask::TransactionTask(
    const std::function<TransactionProcessor::TransactionResult()>& t,
    const std::string& desc)
    : task(t), description(desc), queuedAt(std::chrono::system_clock::now()) {
}

TransactionQueue::TransactionQueue(size_t maxQueueSize) 
    : maxSize(maxQueueSize), totalProcessed(0), totalFailed(0) {
}

TransactionQueue::~TransactionQueue() = default;

bool TransactionQueue::enqueueTransaction(const TransactionTask& task) {
    std::lock_guard<std::mutex> lock(queueMutex);
    
    if (isFull()) {
        return false; // Queue is full
    }
    
    taskQueue.push_back(task);
    queueCondition.notify_one();
    return true;
}

bool TransactionQueue::dequeueTransaction(TransactionTask& task) {
    std::lock_guard<std::mutex> lock(queueMutex);
    
    if (isEmpty()) {
        return false; // Queue is empty
    }
    
    task = taskQueue.front();
    taskQueue.erase(taskQueue.begin());
    return true;
}

size_t TransactionQueue::getQueueSize() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return taskQueue.size();
}

bool TransactionQueue::isEmpty() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return taskQueue.empty();
}

bool TransactionQueue::isFull() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    return taskQueue.size() >= maxSize;
}

void TransactionQueue::clearQueue() {
    std::lock_guard<std::mutex> lock(queueMutex);
    while (!taskQueue.empty()) {
        taskQueue.erase(taskQueue.begin());
    }
}

std::vector<std::string> TransactionQueue::getQueueStatus() const {
    std::lock_guard<std::mutex> lock(queueMutex);
    
    std::vector<std::string> status;
    status.push_back("Queue Size: " + std::to_string(taskQueue.size()));
    status.push_back("Max Size: " + std::to_string(maxSize));
    status.push_back("Is Empty: " + std::string(isEmpty() ? "Yes" : "No"));
    status.push_back("Is Full: " + std::string(isFull() ? "Yes" : "No"));
    
    return status;
}

// ============================================================================
// TRANSACTION LOGGER IMPLEMENTATION
// ============================================================================

TransactionLogger::TransactionLogger(const std::string& logFilePath)
    : logFile(logFilePath), currentLevel(LogLevel::INFO), 
      totalLogged(0), successfulLogged(0), failedLogged(0) {
    logStream.open(logFile, std::ios::app);
}

TransactionLogger::~TransactionLogger() {
    if (logStream.is_open()) {
        logStream.close();
    }
}

void TransactionLogger::logTransaction(const Transaction& transaction) {
    std::string message = "Transaction: " + transaction.transactionId + 
                          " - " + transaction.type + 
                          " $" + std::to_string(transaction.amount) + 
                          " - Status: " + transaction.status;
    
    logMessage(LogLevel::INFO, message);
    
    // Update statistics
    totalLogged++;
    if (transaction.status == TransactionStatus::SUCCESS) {
        successfulLogged++;
    } else {
        failedLogged++;
    }
}

void TransactionLogger::logMessage(LogLevel level, const std::string& message) {
    if (level < currentLevel) {
        return; // Skip logging if level is too low
    }
    
    std::string entry = formatLogEntry(level, message);
    writeToFile(entry);
}

void TransactionLogger::logError(const std::string& error, const std::string& context) {
    std::string message = "ERROR: " + error;
    if (!context.empty()) {
        message += " (Context: " + context + ")";
    }
    logMessage(LogLevel::ERROR, message);
}

void TransactionLogger::setLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(loggerMutex);
    currentLevel = level;
}

void TransactionLogger::flushLogs() {
    std::lock_guard<std::mutex> lock(loggerMutex);
    if (logStream.is_open()) {
        logStream.flush();
    }
}

void TransactionLogger::rotateLogFile() {
    std::lock_guard<std::mutex> lock(loggerMutex);
    
    if (logStream.is_open()) {
        logStream.close();
    }
    
    // Create new log file with timestamp
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::ostringstream oss;
    oss << logFile << "." << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S");
    
    logFile = oss.str();
    logStream.open(logFile, std::ios::app);
}

std::vector<std::string> TransactionLogger::getRecentTransactions(size_t count) {
    // This is a simplified implementation
    // In a real system, you'd read from the log file
    std::vector<std::string> recent;
    recent.push_back("Recent transactions would be loaded from log file");
    return recent;
}

std::vector<std::string> TransactionLogger::searchTransactions(const std::string& searchTerm) {
    // This is a simplified implementation
    // In a real system, you'd search through the log file
    std::vector<std::string> results;
    results.push_back("Search results for: " + searchTerm);
    return results;
}

size_t TransactionLogger::getTotalTransactions() const {
    return totalLogged;
}

size_t TransactionLogger::getSuccessfulTransactions() const {
    return successfulLogged;
}

size_t TransactionLogger::getFailedTransactions() const {
    return failedLogged;
}

std::string TransactionLogger::levelToString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

std::string TransactionLogger::formatLogEntry(LogLevel level, const std::string& message) {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << "[" << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S") << "] "
        << "[" << levelToString(level) << "] "
        << message;
    
    return oss.str();
}

void TransactionLogger::writeToFile(const std::string& entry) {
    std::lock_guard<std::mutex> lock(loggerMutex);
    if (logStream.is_open()) {
        logStream << entry << std::endl;
        logStream.flush();
    }
}

// ============================================================================
// TRANSACTION UTILS IMPLEMENTATION
// ============================================================================

namespace TransactionUtils {
    std::string generateUniqueId() {
        static std::atomic<unsigned long long> counter(0);
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::ostringstream oss;
        oss << "UTIL_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S")
            << "_" << ++counter;
        
        return oss.str();
    }
    
    std::string formatCurrency(double amount) {
        std::ostringstream oss;
        oss << "$" << std::fixed << std::setprecision(2) << amount;
        return oss.str();
    }
    
    bool isValidAccountNumber(const std::string& accountNumber) {
        return !accountNumber.empty() && accountNumber.length() >= 8;
    }
    
    double calculateTransactionFee(double amount, const std::string& type) {
        if (type == TransactionType::TRANSFER) {
            return std::min(amount * 0.01, 10.0); // 1% fee, max $10
        } else if (type == TransactionType::WITHDRAW) {
            return 2.0; // Fixed $2 fee
        }
        return 0.0; // No fee for deposits
    }
    
    std::string generateTransactionSummary(const std::vector<Transaction>& transactions) {
        if (transactions.empty()) {
            return "No transactions to summarize";
        }
        
        double totalAmount = 0.0;
        size_t successfulCount = 0;
        size_t failedCount = 0;
        
        for (const auto& txn : transactions) {
            if (txn.isSuccessful()) {
                totalAmount += txn.amount;
                successfulCount++;
            } else {
                failedCount++;
            }
        }
        
        std::ostringstream oss;
        oss << "Transaction Summary:\n"
            << "Total Transactions: " << transactions.size() << "\n"
            << "Successful: " << successfulCount << "\n"
            << "Failed: " << failedCount << "\n"
            << "Total Amount: " << formatCurrency(totalAmount);
        
        return oss.str();
    }
}
