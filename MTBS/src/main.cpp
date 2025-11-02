#include "../include/bank.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <random>

/**
 * @brief Main demonstration program for the Multithreaded Bank Transaction System
 * 
 * This program showcases:
 * - Account creation and management
 * - Concurrent transaction processing
 * - Thread safety demonstration
 * - Race condition prevention
 */

void demonstrateBasicOperations(Bank& bank) {
    std::cout << "\n=== BASIC BANKING OPERATIONS ===" << std::endl;
    
    // Create accounts
    std::string account1 = bank.createAccount("John Doe", 1000.0);
    std::string account2 = bank.createAccount("Jane Smith", 2500.0);
    std::string account3 = bank.createAccount("Bob Johnson", 500.0);
    
    std::cout << "Created accounts: " << account1 << ", " << account2 << ", " << account3 << std::endl;
    
    // Perform basic transactions
    bank.processDeposit(account1, 500.0, "Salary deposit");
    bank.processWithdraw(account2, 200.0, "ATM withdrawal");
    bank.processTransfer(account1, account3, 150.0, "Loan repayment");
    
    // Wait for transactions to complete
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Display final balances
    std::cout << "\nFinal Balances:" << std::endl;
    std::cout << account1 << " (John Doe): $" << bank.getAccountBalance(account1) << std::endl;
    std::cout << account2 << " (Jane Smith): $" << bank.getAccountBalance(account2) << std::endl;
    std::cout << account3 << " (Bob Johnson): $" << bank.getAccountBalance(account3) << std::endl;
}

void demonstrateConcurrentTransactions(Bank& bank) {
    std::cout << "\n=== CONCURRENT TRANSACTIONS DEMONSTRATION ===" << std::endl;
    
    // Get existing accounts
    auto accounts = bank.getAllAccounts();
    if (accounts.size() < 2) {
        std::cout << "Need at least 2 accounts for concurrent demonstration" << std::endl;
        return;
    }
    
    std::string account1 = accounts[0]->getAccountNumber();
    std::string account2 = accounts[1]->getAccountNumber();
    
    std::cout << "Initiating 10 concurrent transfers between " << account1 << " and " << account2 << std::endl;
    
    // Create multiple threads for concurrent transfers
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&bank, account1, account2, i]() {
            double amount = 10.0 + (i * 5.0); // Different amounts for each thread
            std::string desc = "Concurrent transfer " + std::to_string(i + 1);
            
            if (i % 2 == 0) {
                bank.processTransfer(account1, account2, amount, desc);
            } else {
                bank.processTransfer(account2, account1, amount, desc);
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : threads) {
        thread.join();
    }
    
    // Wait for transactions to process
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    std::cout << "Concurrent transfers completed!" << std::endl;
    std::cout << "Final balance " << account1 << ": $" << bank.getAccountBalance(account1) << std::endl;
    std::cout << "Final balance " << account2 << ": $" << bank.getAccountBalance(account2) << std::endl;
}

void demonstrateRaceConditionPrevention(Bank& bank) {
    std::cout << "\n=== RACE CONDITION PREVENTION DEMONSTRATION ===" << std::endl;
    
    auto accounts = bank.getAllAccounts();
    if (accounts.size() < 2) {
        std::cout << "Need at least 2 accounts for race condition demonstration" << std::endl;
        return;
    }
    
    std::string account1 = accounts[0]->getAccountNumber();
    std::string account2 = accounts[1]->getAccountNumber();
    
    std::cout << "Demonstrating race condition prevention with rapid balance checks..." << std::endl;
    
    // Create threads that rapidly check balances and perform operations
    std::vector<std::thread> balanceThreads;
    std::vector<std::thread> operationThreads;
    
    // Balance checking threads (readers)
    for (int i = 0; i < 5; ++i) {
        balanceThreads.emplace_back([&bank, account1, account2, i]() {
            for (int j = 0; j < 20; ++j) {
                double balance1 = bank.getAccountBalance(account1);
                double balance2 = bank.getAccountBalance(account2);
                
                if (j % 10 == 0) {
                    std::cout << "Thread " << i << " - Balance check " << j << ": "
                              << account1 << "=$" << balance1 << ", "
                              << account2 << "=$" << balance2 << std::endl;
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }
    
    // Operation threads (writers)
    for (int i = 0; i < 3; ++i) {
        operationThreads.emplace_back([&bank, account1, account2, i]() {
            for (int j = 0; j < 10; ++j) {
                double amount = 5.0 + (j * 2.0);
                std::string desc = "Race test " + std::to_string(i) + "-" + std::to_string(j);
                
                if (i % 2 == 0) {
                    bank.processDeposit(account1, amount, desc);
                } else {
                    bank.processWithdraw(account2, amount, desc);
                }
                
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });
    }
    
    // Wait for all threads to complete
    for (auto& thread : balanceThreads) {
        thread.join();
    }
    for (auto& thread : operationThreads) {
        thread.join();
    }
    
    std::cout << "Race condition prevention test completed!" << std::endl;
}

void demonstrateSystemMonitoring(Bank& bank) {
    std::cout << "\n=== SYSTEM MONITORING ===" << std::endl;
    
    std::cout << "Bank Name: " << bank.getBankName() << std::endl;
    std::cout << "Bank Code: " << bank.getBankCode() << std::endl;
    std::cout << "System Status: " << bank.getSystemStatus() << std::endl;
    std::cout << "Performance Report:\n" << bank.getPerformanceReport() << std::endl;
}

void demonstrateErrorHandling(Bank& bank) {
    std::cout << "\n=== ERROR HANDLING DEMONSTRATION ===" << std::endl;
    
    // Try to withdraw more than available balance
    auto accounts = bank.getAllAccounts();
    if (!accounts.empty()) {
        std::string testAccount = accounts[0]->getAccountNumber();
        double currentBalance = bank.getAccountBalance(testAccount);
        
        std::cout << "Attempting to withdraw $" << (currentBalance + 1000.0) 
                  << " from account with balance $" << currentBalance << std::endl;
        
        bool result = bank.processWithdraw(testAccount, currentBalance + 1000.0, "Overdraft test");
        std::cout << "Withdrawal result: " << (result ? "SUCCESS" : "FAILED (expected)") << std::endl;
    }
    
    // Try to access non-existent account
    std::cout << "Attempting to access non-existent account..." << std::endl;
    double balance = bank.getAccountBalance("NONEXISTENT");
    std::cout << "Balance of non-existent account: " << balance << std::endl;
}

int main() {
    std::cout << "🏦 MULTITHREADED BANK TRANSACTION SYSTEM 🏦" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << "Demonstrating Operating System Concepts:" << std::endl;
    std::cout << "- Multithreading and Concurrency" << std::endl;
    std::cout << "- Thread Synchronization (Mutex)" << std::endl;
    std::cout << "- Critical Section Protection" << std::endl;
    std::cout << "- Race Condition Prevention" << std::endl;
    std::cout << "- Deadlock Prevention" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    try {
        // Create and configure the banking system
        Bank::Config config("MTBS Bank", "MTBS001", 100, 50, true);
        Bank bank(config);
        
        // Start the banking system
        bank.startBankingSystem();
        
        // Generate sample data
        bank.generateSampleData(5);
        
        // Demonstrate various aspects of the system
        demonstrateBasicOperations(bank);
        demonstrateConcurrentTransactions(bank);
        demonstrateRaceConditionPrevention(bank);
        demonstrateSystemMonitoring(bank);
        demonstrateErrorHandling(bank);
        
        // Final demonstration
        std::cout << "\n=== FINAL SYSTEM STATE ===" << std::endl;
        auto allAccounts = bank.getAllAccounts();
        std::cout << "Total accounts: " << allAccounts.size() << std::endl;
        
        for (const auto& account : allAccounts) {
            std::cout << "Account: " << account->getAccountNumber() 
                      << " | Holder: " << account->getAccountHolderName()
                      << " | Balance: $" << account->getBalance() << std::endl;
        }
        
        // Stop the banking system
        bank.stopBankingSystem();
        
        std::cout << "\n✅ All demonstrations completed successfully!" << std::endl;
        std::cout << "🎓 This project demonstrates key OS concepts:" << std::endl;
        std::cout << "   • Thread creation and management" << std::endl;
        std::cout << "   • Mutex-based synchronization" << std::endl;
        std::cout << "   • Critical section protection" << std::endl;
        std::cout << "   • Race condition prevention" << std::endl;
        std::cout << "   • Deadlock avoidance strategies" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}



