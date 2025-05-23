#!/bin/bash
# Power1 OS - QEMU Launch Script with dependency checking

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Power1 OS Build System${NC}"
echo "=========================="

# Function to check if command exists
check_command() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}Error: $1 is not installed${NC}"
        return 1
    fi
    return 0
}

# Function to install dependencies on Ubuntu/Debian
install_dependencies() {
    echo -e "${YELLOW}Installing missing dependencies...${NC}"
    sudo apt-get update
    sudo apt-get install -y nasm gcc-multilib build-essential grub-pc-bin grub2-common xorriso qemu-system-x86
}

# Check for required tools
echo "Checking dependencies..."
MISSING_DEPS=false

if ! check_command nasm; then
    MISSING_DEPS=true
fi

if ! check_command gcc; then
    MISSING_DEPS=true
fi

if ! check_command ld; then
    MISSING_DEPS=true
fi

if ! check_command grub-mkrescue; then
    MISSING_DEPS=true
fi

if ! check_command qemu-system-x86_64; then
    MISSING_DEPS=true
fi

# Install dependencies if missing
if [ "$MISSING_DEPS" = true ]; then
    echo -e "${YELLOW}Some dependencies are missing.${NC}"
    read -p "Do you want to install them? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        install_dependencies
    else
        echo -e "${RED}Cannot proceed without dependencies.${NC}"
        exit 1
    fi
fi

echo -e "${GREEN}All dependencies found!${NC}"
echo

echo "Building Power1 OS..."
make clean
make iso

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build successful!${NC}"
    
    # Check if kernel binary exists and has correct format
    if [ -f "build/power1.bin" ]; then
        echo "Kernel binary found, checking format..."
        
        # Check file type
        FILE_TYPE=$(file build/power1.bin)
        echo "File type: $FILE_TYPE"
        
        # Check if it's a valid ELF file
        if echo "$FILE_TYPE" | grep -q "ELF"; then
            echo -e "${GREEN}✓ Valid ELF file${NC}"
            
            # Check ELF header details
            if command -v readelf >/dev/null 2>&1; then
                echo "ELF header info:"
                readelf -h build/power1.bin | grep -E "(Entry point|Start of|Machine)"
            fi
            
            # Check if multiboot header is present in first few KB
            echo "Checking for multiboot2 header in first 8KB..."
            if hexdump -C build/power1.bin | head -20 | grep -q "d6 50 52 e8"; then
                echo -e "${GREEN}✓ Multiboot2 header found in file${NC}"
            else
                echo -e "${RED}✗ Multiboot2 header not found in first 8KB${NC}"
            fi
        else
            echo -e "${RED}✗ Not a valid ELF file${NC}"
            echo "This will cause GRUB to reject the kernel"
        fi
        
        # Check file size
        echo "Kernel size: $(stat -c%s build/power1.bin) bytes"
        
        # Check symbols if possible
        if command -v nm >/dev/null 2>&1; then
            echo "Kernel symbols:"
            nm build/power1.bin 2>/dev/null | grep -E "(kernel_main|stage2_main|_start)" | head -10 || echo "No symbols found or not available"
        fi
    fi
    
    echo "Starting Power1 OS in QEMU..."
    echo "Expected sequence: GRUB -> STAGE2 -> CALL -> Kernel messages"
    echo "If you see only 'STAGE2' and 'CALL', the kernel is not starting"
    
    # Launch QEMU without conflicting stdio usage
    qemu-system-x86_64 -cdrom build/power1.iso -m 256M \
        -vga std -no-reboot
    
    echo -e "\n${YELLOW}QEMU session ended${NC}"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi