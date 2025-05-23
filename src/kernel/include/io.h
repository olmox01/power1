/*
 * Power1 OS - I/O Port Operations
 * x86_64 I/O port access functions
 */

#ifndef _IO_H
#define _IO_H

#include "stdint.h"

/**
 * outb - Output byte to I/O port
 */
static inline void outb(uint16_t port, uint8_t data)
{
    __asm__ volatile ("outb %0, %1" :: "a"(data), "Nd"(port));
}

/**
 * inb - Input byte from I/O port
 */
static inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    __asm__ volatile ("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

/**
 * io_wait - Short delay for I/O operations
 */
static inline void io_wait(void)
{
    outb(0x80, 0);
}

#endif /* _IO_H */
