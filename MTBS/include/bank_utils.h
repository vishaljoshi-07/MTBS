#ifndef BANK_UTILS_H
#define BANK_UTILS_H

#include <string>
#include <regex>
#include <chrono>
#include <iomanip>
#include <sstream>

/**
 * @brief Utility functions for banking operations
 * 
 * Provides common validation and formatting functions used across
 * the banking system.
 */
namespace BankUtils {
    
    /**
     * @brief Validates if an initial balance is acceptable
     * @param balance The balance amount to validate
     * @return true if valid, false otherwise
     */
    inline bool isValidInitialBalance(double balance) {
        return balance >= 0.0 && balance <= 1000000.0; // Max $1M initial balance
    }
    
    /**
     * @brief Validates if an account number format is correct
     * @param accountNumber The account number to validate
     * @return true if valid format, false otherwise
     */
    inline bool isValidAccountNumber(const std::string& accountNumber) {
        if (accountNumber.empty() || accountNumber.length() < 8) {
            return false;
        }
        
        // Check if it contains only alphanumeric characters
        std::regex accountPattern("^[A-Z0-9]{8,12}$");
        return std::regex_match(accountNumber, accountPattern);
    }
    
    /**
     * @brief Validates if a transaction amount is acceptable
     * @param amount The amount to validate
     * @return true if valid, false otherwise
     */
    inline bool isValidTransactionAmount(double amount) {
        return amount > 0.0 && amount <= 100000.0; // Max $100K per transaction
    }
    
    /**
     * @brief Validates if a holder name is acceptable
     * @param name The name to validate
     * @return true if valid, false otherwise
     */
    inline bool isValidHolderName(const std::string& name) {
        if (name.empty() || name.length() < 2 || name.length() > 100) {
            return false;
        }
        
        // Check if it contains only letters, spaces, and common punctuation
        std::regex namePattern("^[a-zA-Z\\s\\-\\'\\,\\.]+$");
        return std::regex_match(name, namePattern);
    }
    
    /**
     * @brief Generates a unique account number
     * @return A unique account number string
     */
    std::string generateAccountNumber();
    
    /**
     * @brief Formats currency amounts
     * @param amount The amount to format
     * @return Formatted currency string
     */
    std::string formatCurrency(double amount);
    
    /**
     * @brief Formats timestamps
     * @param timestamp The timestamp to format
     * @return Formatted timestamp string
     */
    std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp);
    
    /**
     * @brief Generates a random string
     * @param length The length of the string to generate
     * @return Random string
     */
    std::string generateRandomString(size_t length);
    
    /**
     * @brief Validates email format
     * @param email The email to validate
     * @return true if valid email format, false otherwise
     */
    bool isValidEmail(const std::string& email);
    
    /**
     * @brief Sanitizes input strings
     * @param input The input string to sanitize
     * @return Sanitized string
     */
    std::string sanitizeInput(const std::string& input);
}

#endif // BANK_UTILS_H

