# Power1 OS - Build System
# Multiboot2 compliant kernel build configuration

# Toolchain configuration
AS = nasm
CC = gcc
LD = ld
OBJCOPY = objcopy

# Architecture flags
ARCH_FLAGS = -m64 -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2

# Compiler flags
CFLAGS = -std=c11 -ffreestanding -O2 -Wall -Wextra -nostdlib -nostdinc
CFLAGS += $(ARCH_FLAGS) -fno-builtin -fno-stack-protector -fno-pic
CFLAGS += -I$(KERNEL_DIR)/include

# Assembler flags
ASFLAGS = -f elf64 -g -F dwarf

# Bootloader specific flags (32-bit start, 64-bit target)
BOOT_ASFLAGS = -f elf64 -g -F dwarf

# Linker flags
LDFLAGS = -nostdlib -n -T kernel.ld

# Directories
BOOT_DIR = boot
KERNEL_DIR = src/kernel
BUILD_DIR = build
ISO_DIR = $(BUILD_DIR)/iso

# Source files
BOOT_ASM_SOURCES = $(BOOT_DIR)/boot.asm
KERNEL_C_SOURCES = $(shell find $(KERNEL_DIR) -name "*.c" 2>/dev/null || echo "")
KERNEL_ASM_SOURCES = $(shell find $(KERNEL_DIR) -name "*.asm" 2>/dev/null || echo "")

# Object files
BOOT_ASM_OBJECTS = $(patsubst $(BOOT_DIR)/%.asm, $(BUILD_DIR)/%.o, $(BOOT_ASM_SOURCES))
KERNEL_C_OBJECTS = $(patsubst $(KERNEL_DIR)/%.c, $(BUILD_DIR)/kernel/%.o, $(KERNEL_C_SOURCES))
KERNEL_ASM_OBJECTS = $(patsubst $(KERNEL_DIR)/%.asm, $(BUILD_DIR)/kernel/%.o, $(KERNEL_ASM_SOURCES))

ALL_OBJECTS = $(BOOT_ASM_OBJECTS) $(KERNEL_C_OBJECTS) $(KERNEL_ASM_OBJECTS)

# Targets
.PHONY: all clean iso run debug debug-build deps-check

all: deps-check $(BUILD_DIR)/power1.bin

# Check dependencies
deps-check:
	@echo "Checking build dependencies..."
	@command -v nasm >/dev/null 2>&1 || { echo "Error: nasm not found. Please install nasm."; exit 1; }
	@command -v gcc >/dev/null 2>&1 || { echo "Error: gcc not found. Please install build-essential."; exit 1; }
	@command -v ld >/dev/null 2>&1 || { echo "Error: ld not found. Please install binutils."; exit 1; }
	@echo "Dependencies OK"

# Create build directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/kernel:
	mkdir -p $(BUILD_DIR)/kernel
	mkdir -p $(BUILD_DIR)/kernel/00-load
	mkdir -p $(BUILD_DIR)/kernel/01-reg
	mkdir -p $(BUILD_DIR)/kernel/02-load
	mkdir -p $(BUILD_DIR)/kernel/03-instr
	mkdir -p $(BUILD_DIR)/kernel/04-devices
	mkdir -p $(BUILD_DIR)/kernel/05-files
	mkdir -p $(BUILD_DIR)/kernel/07-syscall
	mkdir -p $(BUILD_DIR)/kernel/08-runtime
	mkdir -p $(BUILD_DIR)/kernel/09-sysbase
	mkdir -p $(BUILD_DIR)/kernel/lib

# Compile assembly sources
$(BUILD_DIR)/%.o: $(BOOT_DIR)/%.asm | $(BUILD_DIR)
	@echo "Assembling $<..."
	$(AS) $(BOOT_ASFLAGS) $< -o $@
	@echo "Assembly successful: $@"

# Compile C sources
$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/%.c | $(BUILD_DIR)/kernel
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.o: $(KERNEL_DIR)/%.asm | $(BUILD_DIR)/kernel
	@echo "Assembling $<..."
	$(AS) $(ASFLAGS) $< -o $@

# Link kernel to create ELF executable
$(BUILD_DIR)/power1.bin: $(ALL_OBJECTS) kernel.ld
	@echo "Linking kernel..."
	$(LD) $(LDFLAGS) -o $@ $(ALL_OBJECTS)
	@echo "Kernel linked successfully: $@"
	@echo "Checking kernel format..."
	@file $@ | grep -q "ELF" && echo "✓ ELF format detected" || echo "✗ Not ELF format"
	@echo "Checking kernel symbols..."
	@if nm $@ | grep -q "kernel_main" 2>/dev/null; then echo "✓ kernel_main found"; else echo "✗ kernel_main missing"; fi
	@if nm $@ | grep -q "stage2_main" 2>/dev/null; then echo "✓ stage2_main found"; else echo "✗ stage2_main missing"; fi
	@if nm $@ | grep -q "_start" 2>/dev/null; then echo "✓ _start found"; else echo "✗ _start missing"; fi
	@echo "Kernel size: $$(stat -c%s $@) bytes"
	@echo "Checking multiboot2 magic..."
	@readelf -h $@ >/dev/null 2>&1 && echo "✓ Valid ELF header" || echo "✗ Invalid ELF header"
	@hexdump -C $@ | head -1 | grep -q "7f 45 4c 46" && echo "✓ ELF magic found" || echo "✗ ELF magic missing"

# Create ISO image
iso: $(BUILD_DIR)/power1.bin
	@echo "Creating ISO image..."
	@command -v grub-mkrescue >/dev/null 2>&1 || { echo "Error: grub-mkrescue not found. Please install grub-pc-bin."; exit 1; }
	mkdir -p $(ISO_DIR)/boot/grub
	cp $(BUILD_DIR)/power1.bin $(ISO_DIR)/boot/
	echo 'menuentry "Power1 OS" {' > $(ISO_DIR)/boot/grub/grub.cfg
	echo '    multiboot2 /boot/power1.bin' >> $(ISO_DIR)/boot/grub/grub.cfg
	echo '}' >> $(ISO_DIR)/boot/grub/grub.cfg
	grub-mkrescue -o $(BUILD_DIR)/power1.iso $(ISO_DIR)
	@echo "ISO created: $(BUILD_DIR)/power1.iso"

# Run with minimal QEMU options for debugging
run: iso
	@command -v qemu-system-x86_64 >/dev/null 2>&1 || { echo "Error: qemu-system-x86_64 not found. Please install qemu-system-x86."; exit 1; }
	qemu-system-x86_64 -cdrom $(BUILD_DIR)/power1.iso -m 256M -vga std

# Debug con output più dettagliato
debug: iso
	@command -v qemu-system-x86_64 >/dev/null 2>&1 || { echo "Error: qemu-system-x86_64 not found. Please install qemu-system-x86."; exit 1; }
	qemu-system-x86_64 -cdrom $(BUILD_DIR)/power1.iso -m 256M \
		-serial stdio -vga std -no-reboot -no-shutdown \
		-d guest_errors,cpu_reset,int,exec -D qemu-debug.log \
		-s -S

# Debug build with maximum verbosity
debug-run: iso
	@echo "=== DEBUG RUN ==="
	@echo "Expected boot sequence:"
	@echo "1. GRUB loads and displays menu"
	@echo "2. Bootloader shows: SMCPG"
	@echo "3. Long mode shows: LONG"
	@echo "4. Stage2 shows: STAGE2"
	@echo "5. Kernel call shows: CALL"
	@echo "6. Kernel starts shows: KERN (cyan)"
	@echo "7. Kernel content shows: POWER1 KERNEL LOADED"
	@echo ""
	@echo "If you only see up to CALL, kernel_main is not starting"
	qemu-system-x86_64 -cdrom $(BUILD_DIR)/power1.iso -m 256M \
		-vga std -no-reboot -no-shutdown

# Add verbose mode for debugging
debug-build: CFLAGS += -DDEBUG -g
debug-build: ASFLAGS += -dDEBUG
debug-build: all

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
	@echo "Build directory cleaned"

# Show variables for debugging
show-vars:
	@echo "BOOT_ASM_SOURCES: $(BOOT_ASM_SOURCES)"
	@echo "KERNEL_C_SOURCES: $(KERNEL_C_SOURCES)"
	@echo "KERNEL_C_OBJECTS: $(KERNEL_C_OBJECTS)"
	@echo "ALL_OBJECTS: $(ALL_OBJECTS)"

# Help target
help:
	@echo "Power1 OS Build System"
	@echo "======================"
	@echo "Available targets:"
	@echo "  all       - Build kernel binary"
	@echo "  iso       - Create bootable ISO image"
	@echo "  run       - Build and run in QEMU"
	@echo "  debug     - Build and run in QEMU with debugging"
	@echo "  clean     - Clean build files"
	@echo "  deps-check- Check build dependencies"
	@echo "  help      - Show this help"
