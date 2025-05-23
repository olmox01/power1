/*
 * Power1 OS - Memory Management Definitions
 * Physical and virtual memory management
 */

#ifndef _MEMORY_H
#define _MEMORY_H

#include "stdint.h"
#include "stddef.h"
#include "stdbool.h"

/* Memory layout constants */
#define PAGE_SIZE                   4096
#define PAGE_MASK                   (PAGE_SIZE - 1)
#define LARGE_PAGE_SIZE             (2 * 1024 * 1024)  /* 2MB */
#define HUGE_PAGE_SIZE              (1024 * 1024 * 1024) /* 1GB */

#define KERNEL_VIRTUAL_BASE         0xFFFFFFFF80000000UL
#define KERNEL_HEAP_START           0xFFFFFFFF90000000UL
#define USER_SPACE_END              0x00007FFFFFFFFFFF

/* Page table entry flags */
#define PAGE_PRESENT                (1UL << 0)
#define PAGE_WRITABLE               (1UL << 1)
#define PAGE_USER                   (1UL << 2)
#define PAGE_WRITE_THROUGH          (1UL << 3)
#define PAGE_CACHE_DISABLE          (1UL << 4)
#define PAGE_ACCESSED               (1UL << 5)
#define PAGE_DIRTY                  (1UL << 6)
#define PAGE_HUGE                   (1UL << 7)
#define PAGE_GLOBAL                 (1UL << 8)
#define PAGE_NO_EXECUTE             (1UL << 63)

/* Memory region types */
#define MEMORY_TYPE_AVAILABLE       1
#define MEMORY_TYPE_RESERVED        2
#define MEMORY_TYPE_ACPI_RECLAIM    3
#define MEMORY_TYPE_ACPI_NVS        4
#define MEMORY_TYPE_BAD             5

/* Memory allocation flags */
#define ALLOC_ZERO                  (1 << 0)
#define ALLOC_DMA                   (1 << 1)
#define ALLOC_ATOMIC                (1 << 2)

/* Function prototypes */
int memory_manager_init(void);
void early_memory_init(void *mb_info);

/* Physical memory management */
void *pmem_alloc_page(void);
void *pmem_alloc_pages(size_t count);
void pmem_free_page(void *page);
void pmem_free_pages(void *pages, size_t count);
uint64_t pmem_get_total_memory(void);
uint64_t pmem_get_available_memory(void);

/* Virtual memory management */
void *vmem_map_page(uint64_t vaddr, uint64_t paddr, uint64_t flags);
void vmem_unmap_page(uint64_t vaddr);
uint64_t vmem_get_physical_addr(uint64_t vaddr);
bool vmem_is_mapped(uint64_t vaddr);

/* Kernel heap management */
void *kmalloc(size_t size);
void *kzalloc(size_t size);
void *krealloc(void *ptr, size_t size);
void kfree(void *ptr);

/* Memory utility functions */
void *memset(void *dest, int c, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);

/* Inline helper functions */
static inline uint64_t virt_to_phys(uint64_t vaddr)
{
    return vaddr - KERNEL_VIRTUAL_BASE;
}

static inline uint64_t phys_to_virt(uint64_t paddr)
{
    return paddr + KERNEL_VIRTUAL_BASE;
}

static inline uint64_t page_align_down(uint64_t addr)
{
    return addr & ~PAGE_MASK;
}

static inline uint64_t page_align_up(uint64_t addr)
{
    return (addr + PAGE_SIZE - 1) & ~PAGE_MASK;
}

#endif /* _MEMORY_H */
