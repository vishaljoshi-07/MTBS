# MTBS - Multithreaded Bank Transaction System
# PowerShell Build Script

param(
    [string]$BuildType = "Release",
    [switch]$Clean,
    [switch]$Run,
    [switch]$Frontend
)

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "MTBS - Multithreaded Bank Transaction System" -ForegroundColor Yellow
Write-Host "PowerShell Build Script" -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan

# Check if g++ is available
try {
    $gppPath = Get-Command g++ -ErrorAction Stop
    Write-Host "✓ Compiler found: $($gppPath.Source)" -ForegroundColor Green
} catch {
    Write-Host "❌ ERROR: g++ compiler not found!" -ForegroundColor Red
    Write-Host "Please install MinGW-w64 or MSYS2 with g++ compiler" -ForegroundColor Yellow
    Write-Host "Download from: https://www.mingw-w64.org/downloads/" -ForegroundColor Yellow
    Read-Host "Press Enter to exit"
    exit 1
}

# Create directories
$directories = @("build", "bin")
foreach ($dir in $directories) {
    if (!(Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir | Out-Null
        Write-Host "Created directory: $dir" -ForegroundColor Green
    }
}

# Clean if requested
if ($Clean) {
    Write-Host "🧹 Cleaning previous build..." -ForegroundColor Yellow
    if (Test-Path "build\*.o") { Remove-Item "build\*.o" -Force }
    if (Test-Path "bin\*.exe") { Remove-Item "bin\*.exe" -Force }
    Write-Host "✓ Clean completed" -ForegroundColor Green
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Compiling source files..." -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan

# Compiler flags based on build type
$cflags = "-std=c++17 -Wall -Wextra -pthread -Iinclude"
if ($BuildType -eq "Debug") {
    $cflags += " -g -O0"
} else {
    $cflags += " -O2"
}

# Source files to compile
$sourceFiles = @(
    @{File="src/account.cpp"; Output="build/account.o"},
    @{File="src/bank.cpp"; Output="build/bank.o"},
    @{File="src/bank_utils.cpp"; Output="build/bank_utils.o"},
    @{File="src/transaction.cpp"; Output="build/transaction.o"},
    @{File="src/thread_manager.cpp"; Output="build/thread_manager.o"},
    @{File="src/main.cpp"; Output="build/main.o"}
)

# Compile each source file
$compilationSuccess = $true
foreach ($source in $sourceFiles) {
    Write-Host "Compiling $($source.File)..." -ForegroundColor Blue
    $result = & g++ $cflags.Split(' ') -c $source.File -o $source.Output 2>&1
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "❌ ERROR: Failed to compile $($source.File)" -ForegroundColor Red
        Write-Host $result -ForegroundColor Red
        $compilationSuccess = $false
        break
    } else {
        Write-Host "✓ $($source.File) compiled successfully" -ForegroundColor Green
    }
}

if (!$compilationSuccess) {
    Write-Host "❌ Build failed during compilation" -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Linking executable..." -ForegroundColor Yellow
Write-Host "========================================" -ForegroundColor Cyan

# Link all object files
$objectFiles = Get-ChildItem "build\*.o" | ForEach-Object { $_.FullName }
$linkResult = & g++ $cflags.Split(' ') $objectFiles -o "bin/bank_system.exe" 2>&1

if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ ERROR: Failed to link executable" -ForegroundColor Red
    Write-Host $linkResult -ForegroundColor Red
    Read-Host "Press Enter to exit"
    exit 1
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Build completed successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Executable created: bin\bank_system.exe" -ForegroundColor Green
Write-Host "Build type: $BuildType" -ForegroundColor Cyan
Write-Host ""

# Run if requested
if ($Run) {
    Write-Host "🚀 Running MTBS Banking System..." -ForegroundColor Yellow
    & "bin\bank_system.exe"
}

# Open frontend if requested
if ($Frontend) {
    Write-Host "🌐 Opening frontend..." -ForegroundColor Yellow
    Start-Process "frontend\index.html"
}

Write-Host ""
Write-Host "Available commands:" -ForegroundColor Cyan
Write-Host "  .\build.ps1 -Run          # Build and run" -ForegroundColor White
Write-Host "  .\build.ps1 -Frontend     # Build and open frontend" -ForegroundColor White
Write-Host "  .\build.ps1 -Clean        # Clean and rebuild" -ForegroundColor White
Write-Host "  .\build.ps1 -BuildType Debug  # Build with debug symbols" -ForegroundColor White

if (!$Run -and !$Frontend) {
    Read-Host "Press Enter to exit"
}

