# Makefile for Multithreaded Bank Transaction System
# Demonstrates OS concepts: multithreading, synchronization, critical sections

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread
DEBUG_FLAGS = -std=c++17 -Wall -Wextra -g -O0 -pthread -DDEBUG

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Target executable
TARGET = $(BIN_DIR)/bank_system

# Default target
all: directories $(TARGET)

# Create necessary directories
directories:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BIN_DIR)

# Build the main executable
$(TARGET): $(OBJECTS)
	@echo "🔗 Linking $@..."
	$(CXX) $(OBJECTS) -o $(TARGET) -pthread
	@echo "✅ Build completed successfully!"
	@echo "🚀 Run with: ./$(TARGET)"

# Compile source files to object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "🔨 Compiling $<..."
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# Debug build
debug: CXXFLAGS = $(DEBUG_FLAGS)
debug: directories $(TARGET)

# Clean build artifacts
clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "✅ Clean completed!"

# Install dependencies (for Ubuntu/Debian)
install-deps:
	@echo "📦 Installing dependencies..."
	sudo apt-get update
	sudo apt-get install -y build-essential g++ make
	@echo "✅ Dependencies installed!"

# Run the program
run: $(TARGET)
	@echo "🏃 Running the banking system..."
	./$(TARGET)

# Test compilation without running
test: $(TARGET)
	@echo "✅ Compilation test passed!"

# Show help
help:
	@echo "🏦 Multithreaded Bank Transaction System - Makefile"
	@echo "=================================================="
	@echo "Available targets:"
	@echo "  all          - Build the project (default)"
	@echo "  debug        - Build with debug symbols"
	@echo "  clean        - Remove build artifacts"
	@echo "  install-deps - Install system dependencies"
	@echo "  run          - Build and run the program"
	@echo "  test         - Test compilation only"
	@echo "  help         - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make              # Build the project"
	@echo "  make debug        # Build with debug info"
	@echo "  make run          # Build and run"
	@echo "  make clean        # Clean build files"

# Phony targets
.PHONY: all debug clean install-deps run test help directories

# Print build information
$(TARGET): $(OBJECTS)
	@echo "🔗 Linking $@..."
	$(CXX) $(OBJECTS) -o $(TARGET) -pthread
	@echo "✅ Build completed successfully!"
	@echo "🚀 Run with: ./$(TARGET)"
	@echo ""
	@echo "📊 Build Information:"
	@echo "   Compiler: $(CXX)"
	@echo "   Flags: $(CXXFLAGS)"
	@echo "   Source files: $(words $(SOURCES))"
	@echo "   Object files: $(words $(OBJECTS))"
	@echo "   Target: $(TARGET)"



