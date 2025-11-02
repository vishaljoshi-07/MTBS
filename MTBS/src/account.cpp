#include "../include/account.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>

// Static member initialization
std::atomic<unsigned long long> Transaction::transactionCounter(0);

// ============================================================================
// TRANSACTION IMPLEMENTATION
// ============================================================================

Transaction::Transaction(const std::string& id, const std::string& from, const std::string& to,
                       const std::string& t, double amt, const std::string& desc)
    : transactionId(id), fromAccount(from), toAccount(to), type(t), 
      amount(amt), description(desc), timestamp(std::chrono::system_clock::now()),
      status(TransactionStatus::PENDING) {
}

std::string Transaction::toString() const {
    std::ostringstream oss;
    oss << "Transaction ID: " << transactionId << "\n"
        << "Type: " << type << "\n"
        << "Amount: $" << std::fixed << std::setprecision(2) << amount << "\n"
        << "From: " << (fromAccount.empty() ? "N/A" : fromAccount) << "\n"
        << "To: " << (toAccount.empty() ? "N/A" : toAccount) << "\n"
        << "Description: " << description << "\n"
        << "Status: " << status << "\n"
        << "Timestamp: " << getFormattedTimestamp();
    return oss.str();
}

std::string Transaction::getFormattedTimestamp() const {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

bool Transaction::isSuccessful() const {
    return status == TransactionStatus::SUCCESS;
}

// ============================================================================
// ACCOUNT IMPLEMENTATION
// ============================================================================

Account::Account(const std::string& number, const std::string& holderName, double initialBalance)
    : accountNumber(number), accountHolderName(holderName), balance(initialBalance),
      createdAt(std::chrono::system_clock::now()) {
    
    // Validate initial balance
    if (initialBalance < 0) {
        throw std::invalid_argument("Initial balance cannot be negative");
    }
    
    // Add initial deposit transaction if balance > 0
    if (initialBalance > 0) {
        std::string transactionId = "INIT_" + std::to_string(++Transaction::transactionCounter);
        Transaction initTransaction(transactionId, "", number, TransactionType::DEPOSIT, 
                                  initialBalance, "Initial deposit");
        addTransaction(initTransaction);
    }
}

std::string Account::getAccountNumber() const {
    std::lock_guard<std::mutex> lock(accountMutex);
    return accountNumber;
}

std::string Account::getAccountHolderName() const {
    std::lock_guard<std::mutex> lock(accountMutex);
    return accountHolderName;
}

double Account::getBalance() const {
    std::lock_guard<std::mutex> lock(accountMutex);
    return balance;
}

std::vector<Transaction> Account::getTransactionHistory() const {
    std::lock_guard<std::mutex> lock(accountMutex);
    return transactionHistory;
}

std::chrono::system_clock::time_point Account::getCreatedAt() const {
    std::lock_guard<std::mutex> lock(accountMutex);
    return createdAt;
}

bool Account::deposit(double amount, const std::string& description) {
    // CRITICAL SECTION - Protect balance modification
    std::lock_guard<std::mutex> lock(accountMutex);
    
    // Validate deposit amount
    if (amount <= 0) {
        return false;
    }
    
    // Update balance
    balance += amount;
    
    // Create and add transaction record
    std::string transactionId = "DEP_" + std::to_string(++Transaction::transactionCounter);
    Transaction depositTransaction(transactionId, "", accountNumber, 
                                  TransactionType::DEPOSIT, amount, description);
    depositTransaction.status = TransactionStatus::SUCCESS;
    addTransaction(depositTransaction);
    
    return true;
}

bool Account::withdraw(double amount, const std::string& description) {
    // CRITICAL SECTION - Protect balance modification
    std::lock_guard<std::mutex> lock(accountMutex);
    
    // Validate withdrawal amount
    if (amount <= 0) {
        return false;
    }
    
    // Check sufficient funds
    if (balance < amount) {
        // Create failed transaction record
        std::string transactionId = "WTH_" + std::to_string(++Transaction::transactionCounter);
        Transaction withdrawTransaction(transactionId, accountNumber, "", 
                                      TransactionType::WITHDRAW, amount, description);
        withdrawTransaction.status = TransactionStatus::INSUFFICIENT_FUNDS;
        addTransaction(withdrawTransaction);
        return false;
    }
    
    // Update balance
    balance -= amount;
    
    // Create successful transaction record
    std::string transactionId = "WTH_" + std::to_string(++Transaction::transactionCounter);
    Transaction withdrawTransaction(transactionId, accountNumber, "", 
                                  TransactionType::WITHDRAW, amount, description);
    withdrawTransaction.status = TransactionStatus::SUCCESS;
    addTransaction(withdrawTransaction);
    
    return true;
}

bool Account::transfer(Account& targetAccount, double amount, const std::string& description) {
    // CRITICAL SECTION - Protect both accounts during transfer
    // Use ordered locking to prevent deadlock
    std::unique_lock<std::mutex> lock1(accountMutex, std::defer_lock);
    std::unique_lock<std::mutex> lock2(targetAccount.accountMutex, std::defer_lock);
    
    // Lock in order to prevent deadlock
    if (accountNumber < targetAccount.accountNumber) {
        lock1.lock();
        lock2.lock();
    } else {
        lock2.lock();
        lock1.lock();
    }
    
    // Validate transfer amount
    if (amount <= 0) {
        return false;
    }
    
    // Check sufficient funds
    if (balance < amount) {
        // Create failed transaction record
        std::string transactionId = "TRF_" + std::to_string(++Transaction::transactionCounter);
        Transaction transferTransaction(transactionId, accountNumber, targetAccount.accountNumber,
                                      TransactionType::TRANSFER, amount, description);
        transferTransaction.status = TransactionStatus::INSUFFICIENT_FUNDS;
        addTransaction(transferTransaction);
        return false;
    }
    
    // Perform transfer
    balance -= amount;
    targetAccount.balance += amount;
    
    // Create successful transaction records for both accounts
    std::string transactionId = "TRF_" + std::to_string(++Transaction::transactionCounter);
    
    // Outgoing transfer record
    Transaction outgoingTransfer(transactionId, accountNumber, targetAccount.accountNumber,
                                TransactionType::TRANSFER, amount, description);
    outgoingTransfer.status = TransactionStatus::SUCCESS;
    addTransaction(outgoingTransfer);
    
    // Incoming transfer record for target account
    Transaction incomingTransfer(transactionId, accountNumber, targetAccount.accountNumber,
                                TransactionType::TRANSFER, amount, description);
    incomingTransfer.status = TransactionStatus::SUCCESS;
    targetAccount.addTransaction(incomingTransfer);
    
    return true;
}

void Account::addTransaction(const Transaction& transaction) {
    // CRITICAL SECTION - Protect transaction history
    std::lock_guard<std::mutex> lock(accountMutex);
    transactionHistory.push_back(transaction);
}

void Account::clearTransactionHistory() {
    // CRITICAL SECTION - Protect transaction history
    std::lock_guard<std::mutex> lock(accountMutex);
    transactionHistory.clear();
}

bool Account::isActive() const {
    std::lock_guard<std::mutex> lock(accountMutex);
    return !accountNumber.empty() && !accountHolderName.empty();
}

std::string Account::getStatus() const {
    std::lock_guard<std::mutex> lock(accountMutex);
    if (accountNumber.empty()) return "INVALID";
    if (accountHolderName.empty()) return "UNNAMED";
    return "ACTIVE";
}

std::string Account::toString() const {
    std::lock_guard<std::mutex> lock(accountMutex);
    
    std::ostringstream oss;
    oss << "Account Number: " << accountNumber << "\n"
        << "Holder Name: " << accountHolderName << "\n"
        << "Balance: $" << std::fixed << std::setprecision(2) << balance << "\n"
        << "Status: " << getStatus() << "\n"
        << "Created: " << std::put_time(std::localtime(&std::chrono::system_clock::to_time_t(createdAt)), 
                                       "%Y-%m-%d %H:%M:%S") << "\n"
        << "Transactions: " << transactionHistory.size();
    
    return oss.str();
}

void Account::printDetails() const {
    std::cout << toString() << std::endl;
}

