@echo off
echo ========================================
echo MTBS - Multithreaded Bank Transaction System
echo Windows Build Script
echo ========================================

REM Check if g++ is available
where g++ >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: g++ compiler not found!
    echo Please install MinGW-w64 or MSYS2 with g++ compiler
    echo Download from: https://www.mingw-w64.org/downloads/
    pause
    exit /b 1
)

echo Compiler found: g++
echo.

REM Create build directory
if not exist "build" mkdir build
if not exist "bin" mkdir bin

echo Cleaning previous build...
if exist "build\*.o" del /q "build\*.o"
if exist "bin\*.exe" del /q "bin\*.exe"

echo.
echo ========================================
echo Compiling source files...
echo ========================================

REM Compile all source files
echo Compiling account.cpp...
g++ -std=c++17 -Wall -Wextra -O2 -pthread -Iinclude -c src/account.cpp -o build/account.o
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile account.cpp
    pause
    exit /b 1
)

echo Compiling bank.cpp...
g++ -std=c++17 -Wall -Wextra -O2 -pthread -Iinclude -c src/bank.cpp -o build/bank.o
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile bank.cpp
    pause
    exit /b 1
)

echo Compiling bank_utils.cpp...
g++ -std=c++17 -Wall -Wextra -O2 -pthread -Iinclude -c src/bank_utils.cpp -o build/bank_utils.o
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile bank_utils.cpp
    pause
    exit /b 1
)

echo Compiling transaction.cpp...
g++ -std=c++17 -Wall -Wextra -O2 -pthread -Iinclude -c src/transaction.cpp -o build/transaction.o
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile transaction.cpp
    pause
    exit /b 1
)

echo Compiling thread_manager.cpp...
g++ -std=c++17 -Wall -Wextra -O2 -pthread -Iinclude -c src/thread_manager.cpp -o build/thread_manager.o
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile thread_manager.cpp
    pause
    exit /b 1
)

echo Compiling main.cpp...
g++ -std=c++17 -Wall -Wextra -O2 -pthread -Iinclude -c src/main.cpp -o build/main.o
if %errorlevel% neq 0 (
    echo ERROR: Failed to compile main.cpp
    pause
    exit /b 1
)

echo.
echo ========================================
echo Linking executable...
echo ========================================

REM Link all object files
g++ -std=c++17 -Wall -Wextra -O2 -pthread build/*.o -o bin/bank_system.exe
if %errorlevel% neq 0 (
    echo ERROR: Failed to link executable
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.
echo Executable created: bin\bank_system.exe
echo.
echo To run the program:
echo   bin\bank_system.exe
echo.
echo To open the frontend:
echo   start frontend\index.html
echo.
pause

