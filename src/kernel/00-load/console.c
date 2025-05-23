/*
 * Power1 OS - Early Console Implementation
 * VGA text mode console for early kernel debugging
 */

#include "../include/stdint.h"
#include "../include/stddef.h"
#include "../include/stdarg.h"
#include "../include/stdbool.h"
#include "../include/kernel.h"

/* Local I/O port definitions to avoid include issues */
static inline void outb_local(uint16_t port, uint8_t data)
{
    __asm__ volatile ("outb %0, %1" :: "a"(data), "Nd"(port));
}

/* VGA hardware interface */
#define VGA_CTRL_REGISTER       0x3D4
#define VGA_DATA_REGISTER       0x3D5
#define VGA_CURSOR_HIGH         0x0E
#define VGA_CURSOR_LOW          0x0F

/* Console state */
static struct {
    volatile uint16_t *buffer;
    size_t width;
    size_t height;
    size_t row;
    size_t col;
    uint8_t color;
    bool initialized;
} console_state = {
    .buffer = (volatile uint16_t *)VGA_BUFFER_ADDR,
    .width = VGA_WIDTH,
    .height = VGA_HEIGHT,
    .row = 0,
    .col = 0,
    .color = VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4),
    .initialized = false
};

/* Forward declarations */
static void console_putchar_impl(char c);
static void console_clear_impl(void);
static void console_set_color_impl(uint8_t fg, uint8_t bg);

/* Console operations structure */
static struct console_ops vga_console_ops = {
    .putchar = console_putchar_impl,
    .clear = console_clear_impl,
    .set_color = console_set_color_impl
};

/**
 * console_scroll - Scroll screen up by one line
 */
static void console_scroll(void)
{
    /* Move all lines up */
    for (size_t row = 1; row < console_state.height; row++) {
        for (size_t col = 0; col < console_state.width; col++) {
            size_t src = row * console_state.width + col;
            size_t dst = (row - 1) * console_state.width + col;
            console_state.buffer[dst] = console_state.buffer[src];
        }
    }
    
    /* Clear last line */
    size_t last_line = (console_state.height - 1) * console_state.width;
    for (size_t col = 0; col < console_state.width; col++) {
        console_state.buffer[last_line + col] = 
            (uint16_t)' ' | ((uint16_t)console_state.color << 8);
    }
    
    console_state.row = console_state.height - 1;
}

/**
 * console_update_cursor - Update hardware cursor position
 */
static void console_update_cursor(void)
{
    uint16_t pos = console_state.row * console_state.width + console_state.col;
    
    outb_local(VGA_CTRL_REGISTER, VGA_CURSOR_HIGH);
    outb_local(VGA_DATA_REGISTER, (pos >> 8) & 0xFF);
    outb_local(VGA_CTRL_REGISTER, VGA_CURSOR_LOW);
    outb_local(VGA_DATA_REGISTER, pos & 0xFF);
}

/**
 * console_putchar_at - Put character at specific position
 */
void console_putchar_at(char c, uint8_t color, size_t x, size_t y)
{
    if (x >= console_state.width || y >= console_state.height) {
        return;
    }
    
    size_t index = y * console_state.width + x;
    console_state.buffer[index] = (uint16_t)c | ((uint16_t)color << 8);
}

/**
 * console_putchar_impl - Implementation of putchar for console_ops
 */
static void console_putchar_impl(char c)
{
    if (c == '\n') {
        console_state.col = 0;
        if (++console_state.row >= console_state.height) {
            console_scroll();
        }
        console_update_cursor();
        return;
    }
    
    if (c == '\r') {
        console_state.col = 0;
        console_update_cursor();
        return;
    }
    
    if (c == '\t') {
        /* Tab to next 8-character boundary */
        console_state.col = (console_state.col + 8) & ~7;
        if (console_state.col >= console_state.width) {
            console_state.col = 0;
            if (++console_state.row >= console_state.height) {
                console_scroll();
            }
        }
        console_update_cursor();
        return;
    }
    
    /* Handle backspace */
    if (c == '\b') {
        if (console_state.col > 0) {
            console_state.col--;
            size_t index = console_state.row * console_state.width + console_state.col;
            console_state.buffer[index] = (uint16_t)' ' | ((uint16_t)console_state.color << 8);
            console_update_cursor();
        }
        return;
    }
    
    /* Regular character */
    size_t index = console_state.row * console_state.width + console_state.col;
    console_state.buffer[index] = (uint16_t)c | ((uint16_t)console_state.color << 8);
    
    if (++console_state.col >= console_state.width) {
        console_state.col = 0;
        if (++console_state.row >= console_state.height) {
            console_scroll();
        }
    }
    
    console_update_cursor();
}

/**
 * console_clear_impl - Implementation of clear for console_ops
 */
static void console_clear_impl(void)
{
    for (size_t i = 0; i < console_state.width * console_state.height; i++) {
        console_state.buffer[i] = (uint16_t)' ' | ((uint16_t)console_state.color << 8);
    }
    console_state.row = 0;
    console_state.col = 0;
    console_update_cursor();
}

/**
 * console_set_color_impl - Implementation of set_color for console_ops
 */
static void console_set_color_impl(uint8_t fg, uint8_t bg)
{
    console_state.color = fg | (bg << 4);
}

/**
 * console_write_status - Write status message with color
 */
void console_write_status(const char *message, bool success)
{
    uint8_t old_color = console_state.color;
    console_state.color = success ? 
        (VGA_COLOR_WHITE | (VGA_COLOR_BLACK << 4)) :
        (VGA_COLOR_WHITE | (VGA_COLOR_RED << 4));
    
    while (*message) {
        console_putchar_impl(*message++);
    }
    
    console_state.color = old_color;
}

/**
 * console_init - Initialize VGA console
 */
void console_init(void)
{
    console_state.initialized = true;
    current_console = &vga_console_ops;
    console_clear_impl();
}

/**
 * console_get_position - Get current cursor position
 */
void console_get_position(size_t *row, size_t *col)
{
    if (row) *row = console_state.row;
    if (col) *col = console_state.col;
}

/**
 * console_set_position - Set cursor position
 */
void console_set_position(size_t row, size_t col)
{
    if (row < console_state.height && col < console_state.width) {
        console_state.row = row;
        console_state.col = col;
        console_update_cursor();
    }
}
