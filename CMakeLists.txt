cmake_minimum_required(VERSION 3.16)

project(imemdb VERSION 0.1 LANGUAGES CXX)

# ----------------------
# C++ Standard
# ----------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ----------------------
# Build type
# ----------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting default build type to Debug")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# ----------------------
# Options
# ----------------------
option(IMEMDB_BUILD_TESTS "Build tests" ON)

# ----------------------
# Source files
# ----------------------
set(IMEMDB_SOURCES
    src/kv_store.cpp
    src/cli.cpp
    src/command_validator.cpp
    src/command_dispatcher.cpp
    src/wal_manager.cpp
    src/snapshot_manager.cpp
)

set(IMEMDB_HEADERS
    include/imemdb.h
    include/imemdb/kv_store.h
    include/imemdb/command.h
    include/imemdb/command_parser.h
    include/imemdb/command_validator.h
    include/imemdb/command_dispatcher.h
    include/imemdb/wal_manager.h
    include/imemdb/snapshot_manager.h
)

# ----------------------
# Main executable/library
# ----------------------
add_executable(imemdb ${IMEMDB_SOURCES} ${IMEMDB_HEADERS})
target_include_directories(imemdb PRIVATE include)

# ----------------------
# Compiler warnings (optional but recommended)
# ----------------------
if(MSVC)
    target_compile_options(imemdb PRIVATE /W4 /permissive-)
else()
    target_compile_options(imemdb PRIVATE -Wall -Wextra -pedantic -Wshadow)
endif()

# ----------------------
# Tests
# ----------------------
# if(IMEMDB_BUILD_TESTS)
#     enable_testing()
#     add_executable(imemdb_tests
#         tests/test_kv_store.cpp
#         src/kv_store.cpp
#     )
#     target_include_directories(imemdb_tests PRIVATE include)

#     if(MSVC)
#         target_compile_options(imemdb_tests PRIVATE /W4 /permissive-)
#     else()
#         target_compile_options(imemdb_tests PRIVATE -Wall -Wextra -pedantic -Wshadow)
#     endif()

#     add_test(NAME kv_store_tests COMMAND imemdb_tests)
# endif()