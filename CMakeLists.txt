cmake_minimum_required(VERSION 3.15)
project(ansys_cdb_parser VERSION 0.1.0 LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(BUILD_TESTS "Build tests" ON)
option(BUILD_EXAMPLES "Build examples" ON)

# Library
add_library(ansys_cdb
    src/parser.cpp
    src/node.cpp
    src/element.cpp
    src/component.cpp
    src/archive.cpp
)

target_include_directories(ansys_cdb
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Compiler warnings
if(MSVC)
    target_compile_options(ansys_cdb PRIVATE /W4 /WX)
else()
    target_compile_options(ansys_cdb PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Tests
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Examples
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation
install(TARGETS ansys_cdb
    EXPORT ansys_cdb_targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(EXPORT ansys_cdb_targets
    FILE ansys_cdb_targets.cmake
    NAMESPACE cdb::
    DESTINATION lib/cmake/ansys_cdb
)
