# Keikaku Programming Language - CMake Build
#
# "All proceeds according to plan."

cmake_minimum_required(VERSION 3.16)

project(keikaku
    VERSION 1.0.0
    DESCRIPTION "Keikaku Programming Language - Everything proceeds according to plan"
    LANGUAGES C
)

# C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-g -O0 -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/compiler)

# Source files
set(COMPILER_SOURCES
    compiler/main.c
    compiler/lexer.c
    compiler/parser.c
    compiler/ast.c
    compiler/interpreter.c
)

# Main executable
add_executable(keikaku ${COMPILER_SOURCES})
target_link_libraries(keikaku m)

# Install target
install(TARGETS keikaku DESTINATION bin)
install(FILES include/keikaku.h DESTINATION include)

# Testing
enable_testing()

# Custom target to run examples
add_custom_target(run-examples
    COMMAND ${CMAKE_BINARY_DIR}/keikaku ${CMAKE_SOURCE_DIR}/examples/hello.kei
    COMMAND ${CMAKE_BINARY_DIR}/keikaku ${CMAKE_SOURCE_DIR}/examples/fizzbuzz.kei
    COMMAND ${CMAKE_BINARY_DIR}/keikaku ${CMAKE_SOURCE_DIR}/examples/factorial.kei
    DEPENDS keikaku
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running example programs..."
)

# Package information
set(CPACK_PACKAGE_NAME "keikaku")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${PROJECT_DESCRIPTION})
include(CPack)

message(STATUS "")
message(STATUS "  ╔═══════════════════════════════════════════════════════════╗")
message(STATUS "  ║                K E I K A K U  v${PROJECT_VERSION}                      ║")
message(STATUS "  ║         \"Everything proceeds according to plan.\"          ║")
message(STATUS "  ╚═══════════════════════════════════════════════════════════╝")
message(STATUS "")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")
