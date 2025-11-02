# 🚀 MTBS Setup Guide for Windows

This guide will help you set up and run the Multithreaded Bank Transaction System on Windows.

## 📋 Prerequisites

### 1. Install C++ Compiler

You need a C++ compiler with C++17 support. We recommend **MinGW-w64**:

#### Option A: Install MinGW-w64 (Recommended)
1. Download from: https://www.mingw-w64.org/downloads/
2. Or use MSYS2: https://www.msys2.org/
3. Add the `bin` directory to your system PATH

#### Option B: Install Visual Studio Community
1. Download Visual Studio Community: https://visualstudio.microsoft.com/vs/community/
2. Install with "Desktop development with C++" workload
3. Use the Developer Command Prompt

### 2. Verify Installation

Open Command Prompt or PowerShell and run:
```bash
g++ --version
```

You should see output like:
```
g++ (MinGW-W64 8.1.0) 8.1.0
```

## 🏗️ Building the Project

### Method 1: Using PowerShell Script (Recommended)

1. **Open PowerShell** in your project directory
2. **Run the build script:**
   ```powershell
   .\build.ps1
   ```

3. **Available options:**
   ```powershell
   .\build.ps1 -Run          # Build and run immediately
   .\build.ps1 -Frontend     # Build and open frontend
   .\build.ps1 -Clean        # Clean and rebuild
   .\build.ps1 -BuildType Debug  # Build with debug symbols
   ```

### Method 2: Using Batch File

1. **Open Command Prompt** in your project directory
2. **Run the build script:**
   ```cmd
   build.bat
   ```

### Method 3: Using CMake

1. **Install CMake** from: https://cmake.org/download/
2. **Create build directory:**
   ```cmd
   mkdir build
   cd build
   ```
3. **Configure and build:**
   ```cmd
   cmake ..
   cmake --build . --config Release
   ```

## 🚀 Running the Project

### 1. Start the Backend

After successful compilation, run:
```cmd
bin\bank_system.exe
```

You should see output like:
```
🏦 MULTITHREADED BANK TRANSACTION SYSTEM 🏦
=============================================
Demonstrating Operating System Concepts:
- Multithreading and Concurrency
- Thread Synchronization (Mutex)
- Critical Section Protection
- Race Condition Prevention
- Deadlock Prevention
=============================================
```

### 2. Open the Frontend

#### Option A: Direct File Opening
- Navigate to `frontend\index.html`
- Double-click to open in your browser

#### Option B: Local Web Server
1. **Using Python (if installed):**
   ```cmd
   python -m http.server 8000
   ```

2. **Using Node.js (if installed):**
   ```cmd
   npx http-server frontend -p 8000
   ```

3. **Open browser and go to:** `http://localhost:8000`

## 🔧 Troubleshooting

### Common Issues and Solutions

#### 1. "g++ is not recognized"
**Solution:** Install MinGW-w64 and add to PATH
- Download from: https://www.mingw-w64.org/downloads/
- Add `C:\mingw64\bin` to your system PATH
- Restart Command Prompt/PowerShell

#### 2. Compilation Errors
**Solution:** Check compiler version
```cmd
g++ --version
```
Ensure you have C++17 support (GCC 7+ or Clang 5+)

#### 3. pthread Errors
**Solution:** MinGW-w64 includes pthread support by default
- If using MSVC, the CMake build will handle this automatically

#### 4. Frontend Not Loading
**Solution:** Check browser console for JavaScript errors
- Ensure all frontend files are present
- Try using a local web server instead of file:// protocol

### Build Output

Successful build should create:
```
MTBS/
├── build/           # Object files
├── bin/            # Executable
│   └── bank_system.exe
└── frontend/       # Web interface
    ├── index.html
    ├── styles.css
    └── script.js
```

## 📚 Project Structure

```
MTBS/
├── src/                    # C++ source files
│   ├── main.cpp           # Main program
│   ├── bank.cpp           # Banking logic
│   ├── account.cpp        # Account management
│   ├── transaction.cpp    # Transaction processing
│   ├── thread_manager.cpp # Thread management
│   └── bank_utils.cpp     # Utility functions
├── include/                # Header files
├── frontend/               # Web interface
├── build.ps1              # PowerShell build script
├── build.bat              # Batch build script
├── CMakeLists.txt         # CMake configuration
└── README.md              # Project documentation
```

## 🎯 What You'll See

### Backend Console Output
- Account creation and management
- Concurrent transaction processing
- Thread synchronization demonstration
- Race condition prevention tests
- Performance metrics

### Frontend Web Interface
- **Dashboard**: Real-time banking statistics
- **Accounts**: Account management
- **Transactions**: Transaction history
- **Thread Monitor**: Live thread status
- **Performance**: System metrics

## 🧪 Testing the System

1. **Create multiple accounts** through the frontend
2. **Initiate concurrent transactions** to see multithreading in action
3. **Monitor thread behavior** in the Thread Monitor section
4. **Check transaction logs** for consistency

## 📞 Getting Help

If you encounter issues:

1. **Check the console output** for error messages
2. **Verify compiler installation** with `g++ --version`
3. **Ensure all files are present** in the project directory
4. **Check browser console** for frontend errors

## 🎉 Success!

Once everything is working, you'll have a fully functional multithreaded banking system demonstrating key operating system concepts:

- ✅ **Multithreading and Concurrency**
- ✅ **Thread Synchronization**
- ✅ **Critical Section Protection**
- ✅ **Race Condition Prevention**
- ✅ **Deadlock Avoidance**

Happy learning! 🎓

