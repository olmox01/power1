/*
 * Power1 OS - Multiboot2 Definitions
 * Multiboot2 specification implementation
 */

#ifndef _MULTIBOOT2_H
#define _MULTIBOOT2_H

#include "stdint.h"

/* Multiboot2 magic values */
#define MULTIBOOT2_BOOTLOADER_MAGIC     0x36d76289
#define MULTIBOOT2_HEADER_MAGIC         0xe85250d6

/* Multiboot2 tag types */
#define MULTIBOOT_TAG_TYPE_END                  0
#define MULTIBOOT_TAG_TYPE_CMDLINE              1
#define MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME     2
#define MULTIBOOT_TAG_TYPE_MODULE               3
#define MULTIBOOT_TAG_TYPE_BASIC_MEMINFO        4
#define MULTIBOOT_TAG_TYPE_BOOTDEV              5
#define MULTIBOOT_TAG_TYPE_MMAP                 6
#define MULTIBOOT_TAG_TYPE_VBE                  7
#define MULTIBOOT_TAG_TYPE_FRAMEBUFFER          8
#define MULTIBOOT_TAG_TYPE_ELF_SECTIONS         9
#define MULTIBOOT_TAG_TYPE_APM                  10
#define MULTIBOOT_TAG_TYPE_EFI32                11
#define MULTIBOOT_TAG_TYPE_EFI64                12
#define MULTIBOOT_TAG_TYPE_SMBIOS               13
#define MULTIBOOT_TAG_TYPE_ACPI_OLD             14
#define MULTIBOOT_TAG_TYPE_ACPI_NEW             15
#define MULTIBOOT_TAG_TYPE_NETWORK              16
#define MULTIBOOT_TAG_TYPE_EFI_MMAP             17
#define MULTIBOOT_TAG_TYPE_EFI_BS               18

/* Legacy tag type aliases for compatibility */
#define MULTIBOOT_TAG_TYPE_MEMORY       MULTIBOOT_TAG_TYPE_MMAP

/* Memory map entry types */
#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5

/* Basic structures */
struct multiboot_info {
    uint32_t total_size;
    uint32_t reserved;
};

struct multiboot_tag {
    uint32_t type;
    uint32_t size;
};

struct multiboot_tag_string {
    uint32_t type;
    uint32_t size;
    char string[0];
};

struct multiboot_tag_module {
    uint32_t type;
    uint32_t size;
    uint32_t mod_start;
    uint32_t mod_end;
    char cmdline[0];
};

struct multiboot_tag_basic_meminfo {
    uint32_t type;
    uint32_t size;
    uint32_t mem_lower;
    uint32_t mem_upper;
};

struct multiboot_tag_bootdev {
    uint32_t type;
    uint32_t size;
    uint32_t biosdev;
    uint32_t slice;
    uint32_t part;
};

/* Memory map entry structure */
struct multiboot_mmap_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
} __attribute__((packed));

struct multiboot_tag_mmap {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
    struct multiboot_mmap_entry entries[0];
};

/* Function prototypes */
int multiboot2_parse_info(struct multiboot_info *info);
struct multiboot_tag *multiboot2_find_tag(struct multiboot_info *info, uint32_t type);
void multiboot2_print_info(struct multiboot_info *info);

#endif /* _MULTIBOOT2_H */
