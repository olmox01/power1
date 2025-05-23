/*
 * Power1 OS - Kernel Common Definitions
 * BSD-style kernel architecture with POSIX compatibility layer
 */

#ifndef _KERNEL_H
#define _KERNEL_H

#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"

/* Kernel configuration */
#define KERNEL_VERSION_MAJOR    0
#define KERNEL_VERSION_MINOR    1
#define KERNEL_VERSION_PATCH    0

/* Memory layout constants */
#define KERNEL_VIRTUAL_BASE     0xFFFFFFFF80000000UL
#define KERNEL_HEAP_START       0xFFFFFFFF90000000UL
#define KERNEL_STACK_SIZE       0x4000  /* 16KB */

/* Function prototypes - initialization */
extern void kernel_main(void);
extern void kernel_panic(const char *message) __attribute__((noreturn));

/* Early initialization */
extern void early_console_init(void);
extern void early_memory_init(void *mb_info);
extern void cpu_early_init(void);

/* Subsystem initialization functions */
extern int cpu_registers_init(void);
extern int memory_manager_init(void);
extern int interrupt_system_init(void);
extern int device_manager_init(void);
extern int filesystem_init(void);
extern int syscall_interface_init(void);
extern int runtime_services_init(void);
extern int system_base_init(void);

/* Scheduler functions */
extern void schedule_next_task(void);
extern void scheduler_loop(void);

/* Console/debug output */
extern int kprintf(const char *format, ...);

/* Forward declarations for multiboot structures */
struct multiboot_info;
struct multiboot_tag;

/* Error codes */
#define KERNEL_SUCCESS          0
#define KERNEL_ERROR_NOMEM      -1
#define KERNEL_ERROR_INVALID    -2
#define KERNEL_ERROR_NOTFOUND   -3

/* Console interface */
struct console_ops {
    void (*putchar)(char c);
    void (*clear)(void);
    void (*set_color)(uint8_t fg, uint8_t bg);
};

extern struct console_ops *current_console;

/* Memory region descriptor */
struct memory_region {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
    struct memory_region *next;
};

/* Early kernel state */
struct kernel_early_state {
    struct multiboot_info *mb_info;
    struct memory_region *memory_map;
    uint64_t total_memory;
    uint64_t available_memory;
    bool console_initialized;
    bool memory_initialized;
};

extern struct kernel_early_state kernel_state;

/* VGA text mode constants */
#define VGA_BUFFER_ADDR         0xB8000
#define VGA_WIDTH               80
#define VGA_HEIGHT              25

/* VGA Colors */
#define VGA_COLOR_BLACK         0
#define VGA_COLOR_BLUE          1
#define VGA_COLOR_GREEN         2
#define VGA_COLOR_CYAN          3
#define VGA_COLOR_RED           4
#define VGA_COLOR_MAGENTA       5
#define VGA_COLOR_BROWN         6
#define VGA_COLOR_LIGHT_GREY    7
#define VGA_COLOR_DARK_GREY     8
#define VGA_COLOR_LIGHT_BLUE    9
#define VGA_COLOR_LIGHT_GREEN   10
#define VGA_COLOR_LIGHT_CYAN    11
#define VGA_COLOR_LIGHT_RED     12
#define VGA_COLOR_LIGHT_MAGENTA 13
#define VGA_COLOR_YELLOW        14
#define VGA_COLOR_WHITE         15

/* Utility macros */
#define ALIGN_UP(x, align)      (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align)    ((x) & ~((align) - 1))
#define ARRAY_SIZE(x)           (sizeof(x) / sizeof((x)[0]))
#define MIN(a, b)               ((a) < (b) ? (a) : (b))
#define MAX(a, b)               ((a) > (b) ? (a) : (b))

/* Compiler attributes */
#define __packed                __attribute__((packed))
#define __aligned(x)            __attribute__((aligned(x)))
#define __section(s)            __attribute__((section(s)))

#endif /* _KERNEL_H */
