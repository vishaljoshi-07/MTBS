# Multithreaded Bank Transaction System

A comprehensive demonstration of **multithreading, synchronization, and safe access to shared resources** in operating systems.

## 🏦 Project Overview

This system simulates a real banking environment where multiple transactions can occur concurrently while maintaining data integrity and preventing race conditions. It serves as an excellent learning tool for understanding key OS concepts.

## 🎯 Learning Objectives

### Operating System Concepts Demonstrated:
- **Multithreading**: Multiple transaction threads executing concurrently
- **Synchronization**: Mutex locks preventing race conditions
- **Critical Sections**: Protected shared resources (account balances)
- **Deadlock Prevention**: Careful lock ordering and timeout mechanisms
- **Resource Management**: Safe access to shared banking data

### Key Features:
- Real-time transaction processing with multiple threads
- Thread-safe account operations (deposit, withdraw, transfer)
- Live transaction logging and monitoring
- Modern web-based user interface
- Comprehensive error handling and validation

## 🏗️ Architecture

```
┌─────────────────┐    ┌─────────────────┐    ┌─────────────────┐
│   Web Frontend  │    │   C++ Backend   │    │   Transaction   │
│   (HTML/CSS/JS) │◄──►│   (Multithread) │◄──►│     Threads     │
└─────────────────┘    └─────────────────┘    └─────────────────┘
```

## 🚀 Quick Start

### Prerequisites
- **C++ Compiler** (GCC 7+ or Clang 5+)
- **pthread library** (usually included with GCC)
- **Web Browser** (Chrome, Firefox, Safari, Edge)

### Compilation & Execution

1. **Compile the Backend:**
   ```bash
   make clean
   make
   ```

2. **Run the Banking System:**
   ```bash
   ./bank_system
   ```

3. **Open the Frontend:**
   - Navigate to `frontend/index.html` in your browser
   - Or start a local server: `python -m http.server 8000`

## 📁 Project Structure

```
MTBS/
├── src/                    # C++ source files
│   ├── main.cpp           # Main program entry point
│   ├── bank.cpp           # Core banking logic
│   ├── account.cpp        # Account management
│   ├── transaction.cpp    # Transaction processing
│   └── thread_manager.cpp # Thread management
├── include/                # Header files
│   ├── bank.h
│   ├── account.h
│   ├── transaction.h
│   └── thread_manager.h
├── frontend/               # Web interface
│   ├── index.html         # Main dashboard
│   ├── styles.css         # Styling
│   └── script.js          # Frontend logic
├── docs/                   # Documentation
│   ├── os_concepts.md     # OS concepts explanation
│   └── api_reference.md   # API documentation
├── Makefile               # Build configuration
└── README.md              # This file
```

## 🔧 Core Components

### 1. Banking Engine (`bank.cpp`)
- Account creation and management
- Transaction validation and processing
- Balance calculations and updates

### 2. Thread Manager (`thread_manager.cpp`)
- Thread creation and lifecycle management
- Work queue distribution
- Thread synchronization coordination

### 3. Transaction System (`transaction.cpp`)
- Individual transaction processing
- Critical section protection
- Transaction logging and history

### 4. Web Interface (`frontend/`)
- Real-time account monitoring
- Transaction initiation
- Live balance updates

## 🧪 Testing the System

### Race Condition Demonstration:
1. Create multiple accounts
2. Initiate concurrent transfers between accounts
3. Observe how synchronization prevents data corruption

### Thread Safety Verification:
1. Monitor transaction logs during high concurrency
2. Verify account balances remain consistent
3. Check for any race condition symptoms

## 📚 OS Concepts in Detail

### Mutex and Critical Sections
- **Purpose**: Prevent multiple threads from accessing shared resources simultaneously
- **Implementation**: `std::mutex` protecting account balance updates
- **Benefit**: Ensures transaction atomicity and data consistency

### Thread Synchronization
- **Challenge**: Multiple transactions competing for account access
- **Solution**: Ordered lock acquisition to prevent deadlocks
- **Result**: Safe concurrent banking operations

### Resource Management
- **Shared Resources**: Account balances, transaction logs
- **Protection Mechanism**: Reader-writer locks for optimal performance
- **Monitoring**: Real-time resource state tracking

## 🎨 UI Features

- **Responsive Design**: Works on desktop and mobile devices
- **Dark/Light Mode**: Toggle between themes
- **Real-time Updates**: Live balance and transaction monitoring
- **Interactive Dashboard**: Easy account and transaction management
- **Professional Styling**: Modern banking application appearance

## 🔍 Code Quality

- **Modular Design**: Clean separation of concerns
- **Comprehensive Comments**: Educational code documentation
- **Error Handling**: Robust validation and error reporting
- **Performance Optimized**: Efficient thread management
- **Cross-platform**: Compatible with major operating systems

## 🚨 Important Notes

- **Educational Purpose**: This is a demonstration system, not for production use
- **Thread Safety**: All shared resources are properly protected
- **Performance**: Designed to show OS concepts clearly, not for maximum speed
- **Scalability**: Can handle hundreds of concurrent transactions

## 🤝 Contributing

Feel free to enhance this project by:
- Adding more OS concepts (semaphores, condition variables)
- Implementing additional banking features
- Improving the UI/UX design
- Adding more comprehensive testing

## 📄 License

This project is created for educational purposes. Feel free to use and modify for learning about operating system concepts.

---

**Happy Learning! 🎓**

*Built with ❤️ for Operating Systems Education*

