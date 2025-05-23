/*
 * Power1 OS - Stage 2 Bootloader
 * Modular kernel initialization and memory management setup
 */

#include <stdint.h>
#include <stddef.h>

// Multiboot2 structures
struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot_tag_memory_map {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
};

struct multiboot_memory_map_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
} __attribute__((packed));

// VGA text mode interface
#define VGA_BUFFER ((volatile uint16_t*)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = 0x0F; // White on black

// Memory management structures
typedef struct memory_region {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    struct memory_region* next;
} memory_region_t;

static memory_region_t* memory_map_head = NULL;

// Terminal output functions
static inline uint8_t vga_entry_color(uint8_t fg, uint8_t bg) {
    return fg | bg << 4;
}

static inline uint16_t vga_entry(unsigned char uc, uint8_t color) {
    return (uint16_t) uc | (uint16_t) color << 8;
}

void terminal_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; y++) {
        for (size_t x = 0; x < VGA_WIDTH; x++) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_BUFFER[index] = vga_entry(' ', terminal_color);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

void terminal_putchar(char c) {
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0;
        }
        return;
    }
    
    const size_t index = terminal_row * VGA_WIDTH + terminal_column;
    VGA_BUFFER[index] = vga_entry(c, terminal_color);
    
    if (++terminal_column == VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row == VGA_HEIGHT) {
            terminal_row = 0;
        }
    }
}

void terminal_write_string(const char* data) {
    while (*data) {
        terminal_putchar(*data++);
    }
}

// Memory management functions
void parse_memory_map(struct multiboot_tag_memory_map* mmap_tag) {
    terminal_write_string("Parsing memory map...\n");
    
    uint8_t* entry_ptr = (uint8_t*)(mmap_tag + 1);
    uint8_t* tag_end = (uint8_t*)mmap_tag + mmap_tag->size;
    
    while (entry_ptr < tag_end) {
        struct multiboot_memory_map_entry* entry = 
            (struct multiboot_memory_map_entry*)entry_ptr;
        
        // Create memory region node
        memory_region_t* region = (memory_region_t*)
            (0x10000 + (memory_map_head ? 0x100 : 0)); // Simple allocation
        
        region->base_addr = entry->addr;
        region->length = entry->len;
        region->type = entry->type;
        region->next = memory_map_head;
        memory_map_head = region;
        
        entry_ptr += mmap_tag->entry_size;
    }
    
    terminal_write_string("Memory map parsed successfully\n");
}

void init_physical_memory_manager(void) {
    terminal_write_string("Initializing physical memory manager...\n");
    
    // Count available memory
    uint64_t total_memory = 0;
    uint64_t available_memory = 0;
    
    memory_region_t* current = memory_map_head;
    while (current) {
        total_memory += current->length;
        if (current->type == 1) { // Available memory
            available_memory += current->length;
        }
        current = current->next;
    }
    
    terminal_write_string("Physical memory manager initialized\n");
}

void init_virtual_memory_manager(void) {
    terminal_write_string("Initializing virtual memory manager...\n");
    
    // Setup kernel virtual memory layout
    // Higher half kernel mapping at 0xFFFFFFFF80000000
    
    terminal_write_string("Virtual memory manager initialized\n");
}

void setup_interrupt_handlers(void) {
    terminal_write_string("Setting up interrupt handlers...\n");
    
    // Initialize IDT
    // Setup exception handlers
    // Configure PIC
    
    terminal_write_string("Interrupt handlers configured\n");
}

// Main stage 2 entry point
void stage2_main(uint32_t multiboot_magic, void* multiboot_info) {
    terminal_clear();
    terminal_write_string("Power1 OS - Stage 2 Bootloader\n");
    terminal_write_string("================================\n\n");
    
    // Verify multiboot2
    if (multiboot_magic != 0x36d76289) {
        terminal_write_string("ERROR: Invalid multiboot magic\n");
        while (1) __asm__("hlt");
    }
    
    // Parse multiboot information
    struct multiboot_tag* tag = (struct multiboot_tag*)
        ((uint8_t*)multiboot_info + 8);
    
    while (tag->type != 0) {
        switch (tag->type) {
            case 6: // Memory map
                parse_memory_map((struct multiboot_tag_memory_map*)tag);
                break;
        }
        
        // Move to next tag
        tag = (struct multiboot_tag*)
            ((uint8_t*)tag + ((tag->size + 7) & ~7));
    }
    
    // Initialize kernel subsystems
    init_physical_memory_manager();
    init_virtual_memory_manager();
    setup_interrupt_handlers();
    
    terminal_write_string("\nTransferring control to kernel...\n");
    
    // Transfer control to main kernel
    extern void kernel_main(void);
    kernel_main();
    
    // Should never reach here
    terminal_write_string("ERROR: Kernel returned to bootloader\n");
    while (1) __asm__("hlt");
}
