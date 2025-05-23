/*
 * Power1 OS - CPU Management Definitions
 * x86_64 CPU features and register management
 */

#ifndef _CPU_H
#define _CPU_H

#include "stdint.h"
#include "stdbool.h"

/* CPU feature flags (CPUID) */
#define CPU_FEATURE_FPU         (1 << 0)
#define CPU_FEATURE_VME         (1 << 1)
#define CPU_FEATURE_DE          (1 << 2)
#define CPU_FEATURE_PSE         (1 << 3)
#define CPU_FEATURE_TSC         (1 << 4)
#define CPU_FEATURE_MSR         (1 << 5)
#define CPU_FEATURE_PAE         (1 << 6)
#define CPU_FEATURE_MCE         (1 << 7)
#define CPU_FEATURE_CX8         (1 << 8)
#define CPU_FEATURE_APIC        (1 << 9)
#define CPU_FEATURE_SEP         (1 << 11)
#define CPU_FEATURE_MTRR        (1 << 12)
#define CPU_FEATURE_PGE         (1 << 13)
#define CPU_FEATURE_MCA         (1 << 14)
#define CPU_FEATURE_CMOV        (1 << 15)
#define CPU_FEATURE_PAT         (1 << 16)
#define CPU_FEATURE_PSE36       (1 << 17)
#define CPU_FEATURE_CLFLUSH     (1 << 19)
#define CPU_FEATURE_MMX         (1 << 23)
#define CPU_FEATURE_FXSR        (1 << 24)
#define CPU_FEATURE_SSE         (1 << 25)
#define CPU_FEATURE_SSE2        (1 << 26)

/* Extended CPU features */
#define CPU_FEATURE_EXT_SYSCALL (1 << 11)
#define CPU_FEATURE_EXT_NX      (1 << 20)
#define CPU_FEATURE_EXT_LM      (1 << 29)

/* CPU information structure */
struct cpu_info {
    uint32_t vendor_id[4];
    uint32_t brand_string[12];
    uint32_t family;
    uint32_t model;
    uint32_t stepping;
    uint32_t features_edx;
    uint32_t features_ecx;
    uint32_t ext_features_edx;
    uint32_t ext_features_ecx;
    bool long_mode_supported;
    bool sse_supported;
    bool fpu_supported;
};

/* Register state structure */
struct cpu_registers {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip, rflags;
    uint16_t cs, ds, es, fs, gs, ss;
} __attribute__((packed));

/* Function prototypes */
void cpu_early_init(void);
int cpu_registers_init(void);
void cpu_detect_features(void);
bool cpu_has_feature(uint32_t feature);
void cpu_enable_sse(void);
void cpu_enable_syscall(void);
uint64_t cpu_read_msr(uint32_t msr);
void cpu_write_msr(uint32_t msr, uint64_t value);
void cpu_get_info(struct cpu_info *info);

/* Inline assembly helpers */
static inline void cpu_halt(void)
{
    __asm__ volatile ("hlt");
}

static inline void cpu_disable_interrupts(void)
{
    __asm__ volatile ("cli" ::: "memory");
}

static inline void cpu_enable_interrupts(void)
{
    __asm__ volatile ("sti" ::: "memory");
}

static inline uint64_t cpu_read_cr0(void)
{
    uint64_t val;
    __asm__ volatile ("mov %%cr0, %0" : "=r" (val));
    return val;
}

static inline void cpu_write_cr0(uint64_t val)
{
    __asm__ volatile ("mov %0, %%cr0" :: "r" (val) : "memory");
}

static inline uint64_t cpu_read_cr3(void)
{
    uint64_t val;
    __asm__ volatile ("mov %%cr3, %0" : "=r" (val));
    return val;
}

static inline void cpu_write_cr3(uint64_t val)
{
    __asm__ volatile ("mov %0, %%cr3" :: "r" (val) : "memory");
}

static inline uint64_t cpu_read_cr4(void)
{
    uint64_t val;
    __asm__ volatile ("mov %%cr4, %0" : "=r" (val));
    return val;
}

static inline void cpu_write_cr4(uint64_t val)
{
    __asm__ volatile ("mov %0, %%cr4" :: "r" (val) : "memory");
}

#endif /* _CPU_H */
