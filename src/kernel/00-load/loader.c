/*
 * Power1 OS - Kernel Loader
 * Primary kernel entry point from bootloader
 * BSD-style modular architecture
 */

#include "../include/stdint.h"
#include "../include/stddef.h"
#include "../include/stdbool.h"
#include "../include/kernel.h"
#include "../include/multiboot2.h"

/* Early kernel stack */
extern uint8_t kernel_stack_top[];

/* Multiboot2 information structure */
static struct multiboot_info *mb_info = NULL;

/* Kernel early state */
struct kernel_early_state kernel_state = {0};

/* Early VGA console implementation */
static volatile uint16_t *vga_buffer = (volatile uint16_t *)VGA_BUFFER_ADDR;
static size_t terminal_row = 0;
static size_t terminal_column = 0;
static uint8_t terminal_color = (VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4));

static void early_console_putchar(char c);
static void early_console_clear(void);
static void early_console_set_color(uint8_t fg, uint8_t bg);

static struct console_ops early_console = {
    .putchar = early_console_putchar,
    .clear = early_console_clear,
    .set_color = early_console_set_color
};

struct console_ops *current_console = &early_console;

/* Forward declarations */
static int parse_multiboot_info(struct multiboot_info *info);
static void parse_memory_map_tag(struct multiboot_tag *tag);

/**
 * early_console_init - Initialize early VGA text console
 */
void early_console_init(void)
{
    early_console_clear();
    kernel_state.console_initialized = true;
}

/**
 * early_console_putchar - Output character to VGA buffer
 */
static void early_console_putchar(char c)
{
    if (c == '\n') {
        terminal_column = 0;
        if (++terminal_row >= VGA_HEIGHT) {
            terminal_row = 0;
        }
        return;
    }
    
    size_t index = terminal_row * VGA_WIDTH + terminal_column;
    vga_buffer[index] = (uint16_t)c | ((uint16_t)terminal_color << 8);
    
    if (++terminal_column >= VGA_WIDTH) {
        terminal_column = 0;
        if (++terminal_row >= VGA_HEIGHT) {
            terminal_row = 0;
        }
    }
}

/**
 * early_console_clear - Clear VGA screen
 */
static void early_console_clear(void)
{
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (uint16_t)' ' | ((uint16_t)terminal_color << 8);
    }
    terminal_row = 0;
    terminal_column = 0;
}

/**
 * early_console_set_color - Set VGA text colors
 */
static void early_console_set_color(uint8_t fg, uint8_t bg)
{
    terminal_color = fg | (bg << 4);
}

/**
 * early_memory_init - Initialize early memory management
 */
void early_memory_init(void *mb_info_ptr)
{
    struct multiboot_info *info = (struct multiboot_info *)mb_info_ptr;
    struct multiboot_tag *tag;
    
    kernel_state.mb_info = info;
    kernel_state.total_memory = 0;
    kernel_state.available_memory = 0;
    
    /* Parse multiboot tags for memory information */
    for (tag = (struct multiboot_tag *)(info + 1);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7))) {
        
        if (tag->type == MULTIBOOT_TAG_TYPE_MEMORY) {
            parse_memory_map_tag(tag);
        }
    }
    
    kernel_state.memory_initialized = true;
}

/**
 * cpu_early_init - Initialize early CPU features
 */
void cpu_early_init(void)
{
    /* Enable SSE if available */
    uint32_t eax, ebx, ecx, edx;
    __asm__ volatile ("cpuid"
                      : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
                      : "a" (1));
    
    if (edx & (1 << 25)) { /* SSE support */
        uint64_t cr0, cr4;
        __asm__ volatile ("mov %%cr0, %0" : "=r" (cr0));
        __asm__ volatile ("mov %%cr4, %0" : "=r" (cr4));
        
        cr0 &= ~(1UL << 2);  /* Clear EM */
        cr0 |= (1UL << 1);   /* Set MP */
        cr4 |= (1UL << 9);   /* Set OSFXSR */
        cr4 |= (1UL << 10);  /* Set OSXMMEXCPT */
        
        __asm__ volatile ("mov %0, %%cr0" :: "r" (cr0) : "memory");
        __asm__ volatile ("mov %0, %%cr4" :: "r" (cr4) : "memory");
    }
}

/**
 * parse_memory_map_tag - Parse multiboot memory map
 */
static void parse_memory_map_tag(struct multiboot_tag *tag)
{
    if (!tag || tag->size < 16) {
        return;
    }
    
    /* Simple memory map parsing - avoid complex structures */
    struct multiboot_tag_mmap *mmap_tag = (struct multiboot_tag_mmap *)tag;
    
    uint8_t *entry_ptr = (uint8_t *)(mmap_tag + 1);
    uint8_t *tag_end = (uint8_t *)tag + tag->size;
    
    while (entry_ptr + mmap_tag->entry_size <= tag_end) {
        struct multiboot_mmap_entry *entry = (struct multiboot_mmap_entry *)entry_ptr;
        
        kernel_state.total_memory += entry->len;
        if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
            kernel_state.available_memory += entry->len;
        }
        
        entry_ptr += mmap_tag->entry_size;
    }
}

/**
 * stage2_main - Called from bootloader long mode
 * @mb_info_addr: Physical address of multiboot2 info structure
 */
void stage2_main(uint64_t mb_info_addr)
{
    /* Write immediate debug markers to VGA */
    volatile uint16_t *vga = (volatile uint16_t *)0xB8000;
    
    /* Clear screen completely first */
    for (int i = 0; i < 80*25; i++) {
        vga[i] = 0x0F20; /* Space in white on black */
    }
    
    /* Write debug progression */
    vga[0] = 0x4F53; /* 'S' in white on red - Stage2 start */
    vga[1] = 0x4F54; /* 'T' */
    vga[2] = 0x4F41; /* 'A' */
    vga[3] = 0x4F47; /* 'G' */
    vga[4] = 0x4F45; /* 'E' */
    vga[5] = 0x4F32; /* '2' */
    
    /* Save multiboot info */
    mb_info = (struct multiboot_info *)mb_info_addr;
    
    /* Write calling kernel marker */
    vga[80] = 0x2F43; /* 'C' in green - Calling kernel */
    vga[81] = 0x2F41; /* 'A' */
    vga[82] = 0x2F4C; /* 'L' */
    vga[83] = 0x2F4C; /* 'L' */
    
    /* Jump directly to main kernel without complex initialization */
    kernel_main();
    
    /* Should never reach here */
    vga[160] = 0x4F45; /* 'E' in white on red - Error */
    vga[161] = 0x4F52; /* 'R' */
    vga[162] = 0x4F52; /* 'R' */
    kernel_panic("Kernel main returned");
}

/**
 * kernel_entry - Alternative entry point for direct kernel loading
 */
void kernel_entry(void)
{
    /* Direct kernel entry without multiboot */
    early_console_init();
    kernel_main();
    kernel_panic("Kernel main returned");
}

/**
 * parse_multiboot_info - Parse multiboot2 information structure
 */
static int parse_multiboot_info(struct multiboot_info *info)
{
    struct multiboot_tag *tag;
    
    if (info->total_size < 8) {
        return -1;
    }
    
    /* Iterate through multiboot tags */
    for (tag = (struct multiboot_tag *)(info + 1);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)((uint8_t *)tag + ((tag->size + 7) & ~7))) {
        
        switch (tag->type) {
        case MULTIBOOT_TAG_TYPE_MEMORY:
            /* Handle memory map */
            break;
        case MULTIBOOT_TAG_TYPE_BOOTDEV:
            /* Handle boot device info */
            break;
        case MULTIBOOT_TAG_TYPE_CMDLINE:
            /* Handle command line */
            break;
        case MULTIBOOT_TAG_TYPE_MODULE:
            /* Handle loaded modules */
            break;
        }
    }
    
    return 0;
}
