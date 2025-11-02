#include "../include/bank_utils.h"
#include <random>
#include <algorithm>
#include <cctype>
#include <regex>

namespace BankUtils {

std::string generateAccountNumber() {
    static std::atomic<unsigned long long> counter(0);
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(1000, 9999);
    
    // Generate format: MTBS-XXXX-XXXX
    std::ostringstream oss;
    oss << "MTBS-" << std::setw(4) << std::setfill('0') << dis(gen) 
        << "-" << std::setw(4) << std::setfill('0') << dis(gen);
    
    return oss.str();
}

std::string formatCurrency(double amount) {
    std::ostringstream oss;
    oss << "$" << std::fixed << std::setprecision(2) << amount;
    return oss.str();
}

std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp) {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string generateRandomString(size_t length) {
    static const std::string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, charset.length() - 1);
    
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        result += charset[dis(gen)];
    }
    
    return result;
}

bool isValidEmail(const std::string& email) {
    if (email.empty() || email.length() > 254) {
        return false;
    }
    
    // Basic email validation regex
    std::regex emailPattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email, emailPattern);
}

std::string sanitizeInput(const std::string& input) {
    if (input.empty()) {
        return input;
    }
    
    std::string sanitized = input;
    
    // Remove or escape potentially dangerous characters
    const std::string dangerous = "<>\"'&";
    for (char c : dangerous) {
        size_t pos = 0;
        while ((pos = sanitized.find(c, pos)) != std::string::npos) {
            sanitized.replace(pos, 1, "&#" + std::to_string(static_cast<int>(c)) + ";");
            pos += 4;
        }
    }
    
    // Trim whitespace
    sanitized.erase(0, sanitized.find_first_not_of(" \t\n\r\f\v"));
    sanitized.erase(sanitized.find_last_not_of(" \t\n\r\f\v") + 1);
    
    return sanitized;
}

} // namespace BankUtils

