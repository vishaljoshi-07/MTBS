cmake_minimum_required(VERSION 3.16)
project(MTBS VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler flags
if(MSVC)
    # Microsoft Visual C++ flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /EHsc")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /Od")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /DNDEBUG")
else()
    # GCC/Clang flags
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
    
    # Check for pthread support
    find_package(Threads REQUIRED)
endif()

# Include directories
include_directories(include)

# Source files
set(SOURCES
    src/main.cpp
    src/bank.cpp
    src/bank_utils.cpp
    src/account.cpp
    src/transaction.cpp
    src/thread_manager.cpp
)

# Create executable
add_executable(bank_system ${SOURCES})

# Link libraries
if(MSVC)
    # Windows-specific linking
    target_link_libraries(bank_system)
else()
    # Unix-like systems with pthread
    target_link_libraries(bank_system Threads::Threads)
endif()

# Set properties
set_target_properties(bank_system PROPERTIES
    OUTPUT_NAME "bank_system"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Copy frontend files to build directory
file(COPY frontend DESTINATION ${CMAKE_BINARY_DIR})

# Install rules
install(TARGETS bank_system
    RUNTIME DESTINATION bin
)

install(DIRECTORY frontend/
    DESTINATION share/mtbs/frontend
    FILES_MATCHING PATTERN "*.html" PATTERN "*.css" PATTERN "*.js"
)

# Print configuration info
message(STATUS "MTBS Configuration:")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Output Directory: ${CMAKE_BINARY_DIR}/bin")
message(STATUS "  Frontend Directory: ${CMAKE_BINARY_DIR}/frontend")

# Add custom targets
add_custom_target(run
    COMMAND ${CMAKE_BINARY_DIR}/bin/bank_system
    DEPENDS bank_system
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running MTBS Banking System"
)

add_custom_target(frontend
    COMMAND ${CMAKE_COMMAND} -E echo "Opening frontend..."
    COMMAND ${CMAKE_COMMAND} -E open ${CMAKE_BINARY_DIR}/frontend/index.html
    COMMENT "Opening MTBS Frontend"
)

# Testing (if enabled)
option(BUILD_TESTS "Build test suite" OFF)
if(BUILD_TESTS)
    enable_testing()
    # Add test files here when available
    message(STATUS "Tests enabled")
endif()

