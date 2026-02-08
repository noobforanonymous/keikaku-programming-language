#!/bin/bash
#
# Keikaku Programming Language - Build Script
#
# "All proceeds according to plan."
#

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo ""
echo -e "${CYAN}  ╔═══════════════════════════════════════════════════════════╗${NC}"
echo -e "${CYAN}  ║                K E I K A K U  v1.0.0                      ║${NC}"
echo -e "${CYAN}  ║         \"Everything proceeds according to plan.\"          ║${NC}"
echo -e "${CYAN}  ╚═══════════════════════════════════════════════════════════╝${NC}"
echo ""

# Parse arguments
BUILD_TYPE="Release"
INSTALL=false
CLEAN=false
TEST=false
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --install)
            INSTALL=true
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        --test)
            TEST=true
            shift
            ;;
        --verbose)
            VERBOSE=true
            shift
            ;;
        --help|-h)
            echo "Usage: ./build.sh [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug     Build with debug symbols"
            echo "  --release   Build optimized (default)"
            echo "  --install   Install after building"
            echo "  --clean     Clean before building"
            echo "  --test      Run tests after building"
            echo "  --verbose   Verbose output"
            echo "  --help      Show this help"
            echo ""
            exit 0
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            exit 1
            ;;
    esac
done

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SCRIPT_DIR"

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo -e "${BLUE}  Cleaning previous build...${NC}"
    rm -rf build
    cd compiler && make clean 2>/dev/null || true
    cd ..
    echo -e "${GREEN}  ✓ Clean complete${NC}"
fi

# Create build directory
mkdir -p build
cd build

# Check for CMake
if command -v cmake &> /dev/null; then
    echo -e "${BLUE}  Using CMake build system...${NC}"
    
    CMAKE_ARGS="-DCMAKE_BUILD_TYPE=$BUILD_TYPE"
    
    if [ "$VERBOSE" = true ]; then
        cmake $CMAKE_ARGS .. 
        make VERBOSE=1
    else
        cmake $CMAKE_ARGS .. > /dev/null
        make -j$(nproc) > /dev/null
    fi
    
    BINARY="./keikaku"
else
    # Fallback to Make
    echo -e "${BLUE}  Using Make build system...${NC}"
    
    cd ../compiler
    
    if [ "$BUILD_TYPE" = "Debug" ]; then
        make debug
        BINARY="../build/keikaku-debug"
    else
        make
        BINARY="../build/keikaku"
    fi
    
    cd ../build
fi

echo -e "${GREEN}  ✓ Build complete${NC}"
echo ""

# Test if requested
if [ "$TEST" = true ]; then
    echo -e "${BLUE}  Running tests...${NC}"
    
    cd "$SCRIPT_DIR"
    
    for example in examples/*.kei; do
        echo -e "  Testing: $example"
        ./build/keikaku "$example" > /dev/null
        echo -e "${GREEN}    ✓ Passed${NC}"
    done
    
    echo -e "${GREEN}  ✓ All tests passed${NC}"
    echo ""
fi

# Install if requested
if [ "$INSTALL" = true ]; then
    echo -e "${BLUE}  Installing...${NC}"
    
    cd "$SCRIPT_DIR/build"
    
    if [ -f "Makefile" ]; then
        sudo make install
    else
        sudo cp keikaku /usr/local/bin/
    fi
    
    echo -e "${GREEN}  ✓ Installed to /usr/local/bin/keikaku${NC}"
    echo ""
fi

# Summary
echo -e "${CYAN}  Build Type: $BUILD_TYPE${NC}"
echo -e "${CYAN}  Binary: $SCRIPT_DIR/build/keikaku${NC}"
echo ""
echo -e "  ${GREEN}The keikaku has been compiled. As intended.${NC}"
echo ""
