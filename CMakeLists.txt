cmake_minimum_required(VERSION 3.10)

# Set clang as compiler
set(CMAKE_C_COMPILER /usr/bin/clang-18)
set(CMAKE_CXX_COMPILER /usr/bin/clang-18)

# Set C++23 standard BEFORE project()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++23")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")

# Set default build type BEFORE project()
# Note: To override to Debug, use: cmake -DCMAKE_BUILD_TYPE=Debug ..
# To reset to Release: cmake -U CMAKE_BUILD_TYPE .. && cmake ..
set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type: Debug, Release, RelWithDebInfo, MinSizeRel")

project(AoC2025)

# Link std lib
set(CMAKE_EXE_LINKER_FLAGS "-lstdc++")

# Enable clangd support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Output executables to build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Include project root and improved folder for headers
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/puzzles)

# Add executable for each puzzle
file(GLOB DAYS "puzzles/day*/main.cpp")
foreach(DAY ${DAYS})
    get_filename_component(DAY_DIR ${DAY} DIRECTORY)
    get_filename_component(DAY_NAME ${DAY_DIR} NAME)

    add_executable(aoc_${DAY_NAME} ${DAY})
    target_link_libraries(aoc_${DAY_NAME} m pthread)

    # Copy .txt files to build/puzzles/day_name/
    file(GLOB TXT_FILES "${DAY_DIR}/*.txt")
    file(COPY ${TXT_FILES} DESTINATION ${CMAKE_BINARY_DIR}/puzzles/${DAY_NAME}/)
endforeach()