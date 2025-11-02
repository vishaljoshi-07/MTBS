#include "../include/bank.h"
#include "../include/bank_utils.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <random>

// ============================================================================
// BANK CONFIG IMPLEMENTATION
// ============================================================================

Bank::Config::Config(const std::string& name, const std::string& code,
                     size_t maxAcc, size_t maxTrans, bool audit)
    : bankName(name), bankCode(code), maxAccounts(maxAcc), 
      maxConcurrentTransactions(maxTrans), enableAuditLogging(audit) {
}

// ============================================================================
// BANK EXCEPTION IMPLEMENTATION
// ============================================================================

BankException::BankException(ErrorType type, const std::string& message, 
                             const std::string& accNumber)
    : errorType(type), errorMessage(message), accountNumber(accNumber) {
    
    std::ostringstream oss;
    oss << "Bank Error [" << static_cast<int>(type) << "]: " << message;
    if (!accountNumber.empty()) {
        oss << " (Account: " << accountNumber << ")";
    }
    fullMessage = oss.str();
}

const char* BankException::what() const noexcept {
    return fullMessage.c_str();
}

BankException::ErrorType BankException::getErrorType() const {
    return errorType;
}

std::string BankException::getAccountNumber() const {
    return accountNumber;
}

std::string BankException::getErrorMessage() const {
    return errorMessage;
}

// ============================================================================
// BANK IMPLEMENTATION
// ============================================================================

Bank::Bank(const Config& config)
    : config(config), systemRunning(false), totalTransactions(0), 
      successfulTransactions(0), failedTransactions(0), accountCounter(0) {
    
    // Initialize transaction processing components
    transactionProcessor = std::make_unique<TransactionProcessor>();
    transactionLogger = std::make_unique<TransactionLogger>("bank_transactions.log");
    
    // Initialize thread management components
    threadPool = std::make_unique<ThreadPool>(config.maxConcurrentTransactions);
    workQueue = std::make_unique<WorkQueue>(config.maxConcurrentTransactions);
    threadMonitor = std::make_unique<ThreadMonitor>();
}

Bank::~Bank() {
    stopBankingSystem();
}

std::string Bank::createAccount(const std::string& holderName, double initialBalance) {
    // Validate input
    if (holderName.empty()) {
        throw BankException(BankException::ErrorType::INVALID_ACCOUNT_NUMBER, 
                           "Account holder name cannot be empty");
    }
    
    if (!BankUtils::isValidInitialBalance(initialBalance)) {
        throw BankException(BankException::ErrorType::INVALID_AMOUNT, 
                           "Invalid initial balance");
    }
    
    // Check if we can create more accounts
    if (getTotalAccounts() >= config.maxAccounts) {
        throw BankException(BankException::ErrorType::SYSTEM_ERROR, 
                           "Maximum number of accounts reached");
    }
    
    // Generate unique account number
    std::string accountNumber = BankUtils::generateAccountNumber();
    
    // Create account
    auto account = std::make_shared<Account>(accountNumber, holderName, initialBalance);
    
    // Add to accounts map (THREAD-SAFE)
    {
        std::lock_guard<std::mutex> lock(accountsMutex);
        accounts[accountNumber] = account;
    }
    
    // Log account creation
    if (config.enableAuditLogging) {
        transactionLogger->logMessage(TransactionLogger::LogLevel::INFO, 
                                    "Account created: " + accountNumber + " for " + holderName);
    }
    
    return accountNumber;
}

bool Bank::closeAccount(const std::string& accountNumber) {
    if (!validateAccountNumber(accountNumber)) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(accountsMutex);
    
    auto it = accounts.find(accountNumber);
    if (it == accounts.end()) {
        return false; // Account not found
    }
    
    // Check if account has zero balance
    if (it->second->getBalance() != 0.0) {
        return false; // Cannot close account with non-zero balance
    }
    
    // Remove account
    accounts.erase(it);
    
    // Log account closure
    if (config.enableAuditLogging) {
        transactionLogger->logMessage(TransactionLogger::LogLevel::INFO, 
                                    "Account closed: " + accountNumber);
    }
    
    return true;
}

std::shared_ptr<Account> Bank::getAccount(const std::string& accountNumber) {
    if (!validateAccountNumber(accountNumber)) {
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(accountsMutex);
    auto it = accounts.find(accountNumber);
    return (it != accounts.end()) ? it->second : nullptr;
}

std::vector<std::shared_ptr<Account>> Bank::getAllAccounts() {
    std::lock_guard<std::mutex> lock(accountsMutex);
    std::vector<std::shared_ptr<Account>> result;
    result.reserve(accounts.size());
    
    for (const auto& pair : accounts) {
        result.push_back(pair.second);
    }
    
    return result;
}

bool Bank::processDeposit(const std::string& accountNumber, double amount, 
                          const std::string& description) {
    auto account = getAccount(accountNumber);
    if (!account) {
        updateStatistics(false);
        return false;
    }
    
    bool success = account->deposit(amount, description);
    updateStatistics(success);
    
    if (success && config.enableAuditLogging) {
        transactionLogger->logMessage(TransactionLogger::LogLevel::INFO, 
                                    "Deposit: $" + std::to_string(amount) + 
                                    " to account " + accountNumber);
    }
    
    return success;
}

bool Bank::processWithdraw(const std::string& accountNumber, double amount, 
                           const std::string& description) {
    auto account = getAccount(accountNumber);
    if (!account) {
        updateStatistics(false);
        return false;
    }
    
    bool success = account->withdraw(amount, description);
    updateStatistics(success);
    
    if (success && config.enableAuditLogging) {
        transactionLogger->logMessage(TransactionLogger::LogLevel::INFO, 
                                    "Withdrawal: $" + std::to_string(amount) + 
                                    " from account " + accountNumber);
    }
    
    return success;
}

bool Bank::processTransfer(const std::string& fromAccount, const std::string& toAccount, 
                           double amount, const std::string& description) {
    auto fromAcc = getAccount(fromAccount);
    auto toAcc = getAccount(toAccount);
    
    if (!fromAcc || !toAcc) {
        updateStatistics(false);
        return false;
    }
    
    bool success = fromAcc->transfer(*toAcc, amount, description);
    updateStatistics(success);
    
    if (success && config.enableAuditLogging) {
        transactionLogger->logMessage(TransactionLogger::LogLevel::INFO, 
                                    "Transfer: $" + std::to_string(amount) + 
                                    " from " + fromAccount + " to " + toAccount);
    }
    
    return success;
}

double Bank::getAccountBalance(const std::string& accountNumber) {
    auto account = getAccount(accountNumber);
    return account ? account->getBalance() : -1.0;
}

std::vector<Transaction> Bank::getAccountTransactions(const std::string& accountNumber) {
    auto account = getAccount(accountNumber);
    return account ? account->getTransactionHistory() : std::vector<Transaction>();
}

std::string Bank::getAccountStatus(const std::string& accountNumber) {
    auto account = getAccount(accountNumber);
    return account ? account->getStatus() : "NOT_FOUND";
}

void Bank::startBankingSystem() {
    if (systemRunning) {
        return; // Already running
    }
    
    systemRunning = true;
    
    // Start thread pool
    threadPool->start();
    
    // Thread monitor is automatically ready
    
    if (config.enableAuditLogging) {
        transactionLogger->logMessage(TransactionLogger::LogLevel::INFO, 
                                    "Banking system started");
    }
}

void Bank::stopBankingSystem() {
    if (!systemRunning) {
        return; // Already stopped
    }
    
    systemRunning = false;
    
    // Stop thread pool
    threadPool->stop();
    
    // Thread monitor cleanup is automatic
    
    if (config.enableAuditLogging) {
        transactionLogger->logMessage(TransactionLogger::LogLevel::INFO, 
                                    "Banking system stopped");
    }
}

bool Bank::isSystemRunning() const {
    return systemRunning;
}

size_t Bank::getTotalAccounts() const {
    std::lock_guard<std::mutex> lock(accountsMutex);
    return accounts.size();
}

size_t Bank::getActiveAccounts() const {
    std::lock_guard<std::mutex> lock(accountsMutex);
    size_t count = 0;
    for (const auto& pair : accounts) {
        if (pair.second->isActive()) {
            count++;
        }
    }
    return count;
}

size_t Bank::getTotalTransactions() const {
    return totalTransactions;
}

size_t Bank::getSuccessfulTransactions() const {
    return successfulTransactions;
}

size_t Bank::getFailedTransactions() const {
    return failedTransactions;
}

std::string Bank::getBankName() const {
    return config.bankName;
}

std::string Bank::getBankCode() const {
    return config.bankCode;
}

std::string Bank::getSystemStatus() const {
    std::ostringstream oss;
    oss << "System: " << (systemRunning ? "RUNNING" : "STOPPED") << "\n"
        << "Accounts: " << getTotalAccounts() << "/" << config.maxAccounts << "\n"
        << "Transactions: " << totalTransactions << "\n"
        << "Success Rate: " << (totalTransactions > 0 ? 
            (successfulTransactions * 100.0 / totalTransactions) : 0.0) << "%";
    
    return oss.str();
}

std::string Bank::getPerformanceReport() const {
    std::ostringstream oss;
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    oss << "=== BANK PERFORMANCE REPORT ===\n"
        << "Bank: " << config.bankName << " (" << config.bankCode << ")\n"
        << "System Status: " << (systemRunning ? "RUNNING" : "STOPPED") << "\n"
        << "Total Accounts: " << getTotalAccounts() << "\n"
        << "Active Accounts: " << getActiveAccounts() << "\n"
        << "Total Transactions: " << totalTransactions << "\n"
        << "Successful: " << successfulTransactions << "\n"
        << "Failed: " << failedTransactions << "\n"
        << "Success Rate: " << (totalTransactions > 0 ? 
            (successfulTransactions * 100.0 / totalTransactions) : 0.0) << "%\n"
        << "Max Concurrent Transactions: " << config.maxConcurrentTransactions << "\n"
        << "Audit Logging: " << (config.enableAuditLogging ? "ENABLED" : "DISABLED") << "\n"
        << "Generated: " << std::put_time(std::gmtime(&time_t), "%Y-%m-%d %H:%M:%S");
    
    return oss.str();
}

void Bank::generateSampleData(size_t accountCount) {
    if (accountCount > config.maxAccounts) {
        accountCount = config.maxAccounts;
    }
    
    std::vector<std::string> names = {
        "John Smith", "Jane Doe", "Bob Johnson", "Alice Brown", "Charlie Wilson",
        "Diana Davis", "Edward Miller", "Fiona Garcia", "George Martinez", "Helen Taylor"
    };
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> balanceDist(100.0, 10000.0);
    
    for (size_t i = 0; i < accountCount; ++i) {
        std::string name = names[i % names.size()] + " " + std::to_string(i + 1);
        double balance = balanceDist(gen);
        
        try {
            createAccount(name, balance);
        } catch (const BankException& e) {
            // Log error but continue
            if (config.enableAuditLogging) {
                transactionLogger->logError("Failed to create sample account: " + 
                                          std::string(e.what()));
            }
        }
    }
    
    if (config.enableAuditLogging) {
        transactionLogger->logMessage(TransactionLogger::LogLevel::INFO, 
                                    "Generated " + std::to_string(accountCount) + " sample accounts");
    }
}

void Bank::clearAllData() {
    std::lock_guard<std::mutex> lock(accountsMutex);
    accounts.clear();
    accountCounter = 0;
    
    if (config.enableAuditLogging) {
        transactionLogger->logMessage(TransactionLogger::LogLevel::WARNING, 
                                    "All account data cleared");
    }
}

void Bank::exportTransactionLog(const std::string& filename) {
    if (!config.enableAuditLogging) {
        return; // No logging enabled
    }
    
    std::ofstream exportFile(filename);
    if (exportFile.is_open()) {
        exportFile << getPerformanceReport() << std::endl;
        exportFile.close();
        
        if (config.enableAuditLogging) {
            transactionLogger->logMessage(TransactionLogger::LogLevel::INFO, 
                                        "Transaction log exported to: " + filename);
        }
    }
}

void Bank::importTransactionLog(const std::string& filename) {
    // This is a simplified implementation
    // In a real system, you'd parse and import transaction data
    if (config.enableAuditLogging) {
        transactionLogger->logMessage(TransactionLogger::LogLevel::INFO, 
                                    "Transaction log import requested from: " + filename);
    }
}

// generateAccountNumber is now implemented in bank_utils.cpp

bool Bank::validateAccountNumber(const std::string& accountNumber) {
    return TransactionUtils::isValidAccountNumber(accountNumber);
}

bool Bank::validateTransaction(const std::string& accountNumber, double amount, 
                              const std::string& type) {
    if (!validateAccountNumber(accountNumber)) {
        return false;
    }
    
    if (amount <= 0) {
        return false;
    }
    
    auto account = getAccount(accountNumber);
    if (!account || !account->isActive()) {
        return false;
    }
    
    if ((type == "WITHDRAW" || type == "TRANSFER") && 
        account->getBalance() < amount) {
        return false;
    }
    
    return true;
}

void Bank::logTransaction(const Transaction& transaction) {
    if (config.enableAuditLogging) {
        transactionLogger->logTransaction(transaction);
    }
}

void Bank::updateStatistics(bool success) {
    totalTransactions++;
    if (success) {
        successfulTransactions++;
    } else {
        failedTransactions++;
    }
}

std::shared_ptr<Account> Bank::findAccountUnsafe(const std::string& accountNumber) {
    auto it = accounts.find(accountNumber);
    return (it != accounts.end()) ? it->second : nullptr;
}

void Bank::addAccountUnsafe(const std::string& accountNumber, std::shared_ptr<Account> account) {
    accounts[accountNumber] = account;
}

void Bank::removeAccountUnsafe(const std::string& accountNumber) {
    accounts.erase(accountNumber);
}

void Bank::processTransactionAsync(const std::function<void()>& transactionTask, 
                                  const std::string& description) {
    if (!systemRunning) {
        return;
    }
    
    // Add task to work queue
    WorkQueue::WorkItem workItem(transactionTask, description);
    if (workQueue->enqueue(workItem)) {
        // Process task in thread pool
        threadPool->submitTask(transactionTask);
    }
}

// ============================================================================
// BANK UTILS IMPLEMENTATION
// ============================================================================

namespace BankUtils {
    bool isValidBankCode(const std::string& code) {
        return !code.empty() && code.length() >= 3 && code.length() <= 10;
    }
    
    bool isValidAccountHolderName(const std::string& name) {
        return !name.empty() && name.length() >= 2 && name.length() <= 100;
    }
    
    bool isValidInitialBalance(double balance) {
        return balance >= 0.0 && balance < 1000000.0; // Reasonable limits
    }
    
    std::string formatAccountNumber(const std::string& number) {
        if (number.length() <= 8) {
            return number;
        }
        return number.substr(0, 4) + "-" + number.substr(4, 4);
    }
    
    std::string formatBankCode(const std::string& code) {
        return code;
    }
    
    std::string formatCurrency(double amount) {
        std::ostringstream oss;
        oss << "$" << std::fixed << std::setprecision(2) << amount;
        return oss.str();
    }
    
    std::string generateBankCode() {
        static std::atomic<unsigned long long> counter(0);
        std::ostringstream oss;
        oss << "BANK" << std::setfill('0') << std::setw(3) << (++counter % 1000);
        return oss.str();
    }
    
    std::string generateAccountNumber() {
        static std::atomic<unsigned long long> counter(0);
        std::ostringstream oss;
        oss << "ACC" << std::setfill('0') << std::setw(8) << (++counter % 100000000);
        return oss.str();
    }
    
    std::string generateTransactionId() {
        static std::atomic<unsigned long long> counter(0);
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        
        std::ostringstream oss;
        oss << "TXN_" << std::put_time(std::localtime(&time_t), "%Y%m%d_%H%M%S")
            << "_" << std::setfill('0') << std::setw(6) << (++counter % 1000000);
        
        return oss.str();
    }
    
    double calculateInterest(double principal, double rate, int months) {
        if (rate <= 0 || months <= 0) {
            return 0.0;
        }
        double monthlyRate = rate / 12.0 / 100.0;
        return principal * monthlyRate * months;
    }
    
    double calculateTransactionFee(double amount, const std::string& type) {
        if (type == TransactionType::TRANSFER) {
            return std::min(amount * 0.01, 10.0); // 1% fee, max $10
        } else if (type == TransactionType::WITHDRAW) {
            return 2.0; // Fixed $2 fee
        }
        return 0.0; // No fee for deposits
    }
    
    double calculateMinimumBalance(const std::string& accountType) {
        if (accountType == "SAVINGS") {
            return 100.0;
        } else if (accountType == "CHECKING") {
            return 0.0;
        } else if (accountType == "PREMIUM") {
            return 1000.0;
        }
        return 0.0;
    }
}
