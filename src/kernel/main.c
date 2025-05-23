/*
 * Power1 OS - Kernel Main
 * Primary kernel orchestrator with BSD-style modular architecture
 * POSIX-compliant system call interface preparation
 */

#include "include/stdint.h"
#include "include/stddef.h"
#include "include/stdarg.h"
#include "include/stdbool.h"
#include "include/kernel.h"

/* Forward declarations */
static void int_to_str(uint32_t value, char *buffer, int base);
static void write_string_vga(const char *str, int row);

/* Kernel version information */
const char kernel_version[] = "Power1 OS v0.1.0-alpha";
const char kernel_build[] = __DATE__ " " __TIME__;

/**
 * write_string_vga - Write string directly to VGA buffer
 */
static void write_string_vga(const char *str, int row)
{
    volatile uint16_t *vga = (volatile uint16_t *)0xB8000;
    int col = 0;
    
    while (*str && col < 80) {
        vga[row * 80 + col] = 0x0F00 | *str;
        str++;
        col++;
    }
}

/**
 * kernel_main - Primary kernel entry point
 */
void kernel_main(void)
{
    volatile uint16_t *vga = (volatile uint16_t *)0xB8000;
    
    /* IMMEDIATE debug marker - should appear as soon as kernel_main starts */
    vga[240] = 0x3F4B; /* 'K' in cyan on black - Kernel reached */
    vga[241] = 0x3F45; /* 'E' */
    vga[242] = 0x3F52; /* 'R' */
    vga[243] = 0x3F4E; /* 'N' */
    
    /* Clear screen first */
    for (int i = 0; i < 80*25; i++) {
        vga[i] = 0x0F20; /* Space in white on black */
    }
    
    /* Write multiple debug messages */
    write_string_vga("POWER1 KERNEL LOADED SUCCESSFULLY", 0);
    write_string_vga("Kernel Version: ", 2);
    write_string_vga(kernel_version, 3);
    write_string_vga("Build: ", 4);
    write_string_vga(kernel_build, 5);
    write_string_vga("Architecture: x86_64", 6);
    write_string_vga("Status: Running in 64-bit mode", 8);
    write_string_vga("System: Operational", 10);
    
    /* Write a blinking cursor */
    vga[12 * 80] = 0x0F5F; /* '_' character */
    
    /* Simple infinite loop with periodic output */
    int counter = 0;
    while (1) {
        /* Update counter display every so often */
        if (++counter == 10000000) {
            char buffer[16];
            int_to_str(counter / 1000000, buffer, 10);
            write_string_vga("Counter: ", 14);
            
            /* Write counter value */
            volatile uint16_t *pos = vga + 14 * 80 + 9;
            for (int i = 0; buffer[i]; i++) {
                pos[i] = 0x0E00 | buffer[i]; /* Yellow text */
            }
            
            counter = 0;
        }
        
        __asm__ volatile ("hlt");
    }
}

/**
 * int_to_str - Convert integer to string
 */
static void int_to_str(uint32_t value, char *buffer, int base)
{
    char *p = buffer;
    char *p1, *p2;
    uint32_t digits = 0;
    
    if (value == 0) {
        *p++ = '0';
        *p = '\0';
        return;
    }
    
    while (value) {
        uint32_t remainder = value % base;
        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
        value /= base;
        digits++;
    }
    
    *p = '\0';
    
    /* Reverse string */
    p1 = buffer;
    p2 = buffer + digits - 1;
    while (p1 < p2) {
        char temp = *p1;
        *p1 = *p2;
        *p2 = temp;
        p1++;
        p2--;
    }
}

/**
 * kernel_panic - Halt system with error message
 */
void kernel_panic(const char *message)
{
    /* Disable interrupts */
    __asm__ volatile ("cli");
    
    /* Write panic message to VGA directly */
    write_string_vga("*** KERNEL PANIC ***", 20);
    write_string_vga("Error: ", 21);
    write_string_vga(message, 22);
    write_string_vga("System halted.", 23);
    
    /* Halt forever */
    while (1) {
        __asm__ volatile ("hlt");
    }
}

/* Stub functions to satisfy linker */
int kprintf(const char *format, ...) { (void)format; return 0; }
int cpu_registers_init(void) { return 0; }
int memory_manager_init(void) { return 0; }
int interrupt_system_init(void) { return 0; }
int device_manager_init(void) { return 0; }
int filesystem_init(void) { return 0; }
int syscall_interface_init(void) { return 0; }
int runtime_services_init(void) { return 0; }
int system_base_init(void) { return 0; }
void schedule_next_task(void) { }
void scheduler_loop(void) { while(1) __asm__("hlt"); }
