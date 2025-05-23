/* Power1 OS - Kernel Linker Script */
/* Multiboot2 compliant ELF kernel layout */

ENTRY(_start)

SECTIONS
{
    /* Kernel loaded at 1MB physical address */
    . = 1M;

    /* Multiboot header must be early in the file, within first 32KB */
    .multiboot ALIGN(8) : {
        *(.multiboot_header)
    }

    /* Text segment immediately after multiboot header */
    .text ALIGN(4K) : {
        *(.text)
        *(.text.*)
    }

    /* Read-only data */
    .rodata ALIGN(4K) : {
        *(.rodata)
        *(.rodata.*)
    }

    /* Data segment */
    .data ALIGN(4K) : {
        *(.data)
        *(.data.*)
    }

    /* BSS segment */
    .bss ALIGN(4K) : {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /* Stack space */
    . = ALIGN(16);
    stack_bottom = .;
    . += 0x4000;
    stack_top = .;
    
    kernel_stack_bottom = .;
    . += 0x4000;
    kernel_stack_top = .;

    /* Discard debug and other sections that might cause issues */
    /DISCARD/ : {
        *(.comment)
        *(.note)
        *(.note.*)
        *(.eh_frame)
        *(.eh_frame_hdr)
        *(.gnu.hash)
        *(.hash)
        *(.dynamic)
        *(.dynstr)
        *(.dynsym)
        *(.interp)
    }
}