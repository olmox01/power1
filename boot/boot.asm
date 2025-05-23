; Power1 OS - Primary Bootloader
; x86_64 Multiboot2 compliant bootloader with modular architecture

MAGIC equ 0xE85250D6                ; Multiboot2 magic number
ARCHITECTURE equ 0                  ; i386 protected mode
HEADER_LENGTH equ header_end - header_start
CHECKSUM equ -(MAGIC + ARCHITECTURE + HEADER_LENGTH) ; Correct calculation

; This section MUST be first in the file for multiboot2 compliance
section .multiboot_header
align 8
header_start:
    dd MAGIC                        ; Multiboot2 magic
    dd ARCHITECTURE                 ; Architecture (i386)
    dd HEADER_LENGTH               ; Header length
    dd CHECKSUM                    ; Checksum
    
    ; Information request tag (optional)
    align 8
    dw 1                           ; Type: information request
    dw 0                           ; Flags
    dd 20                          ; Size
    dd 4                           ; Basic memory info
    dd 6                           ; Memory map
    dd 8                           ; Framebuffer info
    
    ; Entry address tag (simplified)
    align 8
    dw 3                           ; Type: entry address tag
    dw 0                           ; Flags
    dd 12                          ; Size
    dd _start                      ; Entry point
    
    ; Module alignment tag
    align 8
    dw 6                           ; Type: module alignment
    dw 0                           ; Flags  
    dd 8                           ; Size
    
    ; End tag (must be 8-byte aligned)
    align 8
    dw 0                           ; Type: end
    dw 0                           ; Flags
    dd 8                           ; Size
header_end:

section .bss
align 16
stack_bottom:
    resb 16384              ; 16KB stack
stack_top:

; Kernel stack (16KB)
align 16
kernel_stack_bottom:
    resb 16384
kernel_stack_top:

; Start in 32-bit mode
[bits 32]
section .text
global _start
_start:
    ; Immediate debug to screen
    mov edi, 0xb8000
    mov word [edi], 0x4F53      ; 'S' in white on red (Start)
    
    ; Disable interrupts and setup stack
    cli
    mov esp, stack_top
    
    ; Verify multiboot2 bootloader
    cmp eax, 0x36d76289
    jne .no_multiboot
    
    ; Write multiboot OK marker
    mov word [edi + 2], 0x4F4D  ; 'M'
    
    ; Save multiboot info pointer for later
    mov [multiboot_info], ebx
    
    ; Check CPU capabilities
    call check_cpuid
    call check_long_mode
    
    ; Write CPU OK marker
    mov word [edi + 4], 0x4F43  ; 'C'
    
    ; Setup paging for long mode transition
    call setup_page_tables
    call enable_paging
    
    ; Write paging OK marker
    mov word [edi + 6], 0x4F50  ; 'P'
    
    ; Load GDT and transition to long mode
    lgdt [gdt64.pointer]
    
    ; Write GDT loaded marker
    mov word [edi + 8], 0x4F47  ; 'G'
    
    jmp gdt64.code:long_mode_start

.no_multiboot:
    mov edi, 0xb8000
    mov word [edi], 0x4F45      ; 'E' in white on red (Error)
    mov word [edi + 2], 0x4F52  ; 'R'
    mov word [edi + 4], 0x4F52  ; 'R'
    hlt

; CPU Capability Checks (32-bit mode)
check_cpuid:
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 1 << 21
    push eax
    popfd
    pushfd
    pop eax
    push ecx
    popfd
    xor eax, ecx
    jz .no_cpuid
    ret
.no_cpuid:
    mov esi, no_cpuid_msg
    call print_error
    hlt

check_long_mode:
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb .no_long_mode
    
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz .no_long_mode
    ret
.no_long_mode:
    mov esi, no_long_mode_msg
    call print_error
    hlt

; Page Table Setup for Long Mode (32-bit mode)
setup_page_tables:
    ; Clear page tables
    mov edi, 0x1000
    mov cr3, edi
    xor eax, eax
    mov ecx, 4096
    rep stosd
    mov edi, cr3
    
    ; Setup P4 table
    mov DWORD [edi], 0x2003         ; P3 table at 0x2000
    
    ; Setup P3 table  
    mov edi, 0x2000
    mov DWORD [edi], 0x3003         ; P2 table at 0x3000
    
    ; Setup P2 table (2MB pages)
    mov edi, 0x3000
    mov ebx, 0x00000083             ; Present, writable, huge page
    mov ecx, 512
.map_p2_table:
    mov [edi], ebx
    add ebx, 0x200000               ; 2MB increment
    add edi, 8
    loop .map_p2_table
    ret

enable_paging:
    ; Enable PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    
    ; Enable long mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr
    
    ; Enable paging
    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax
    ret

; Error printing routine (32-bit mode)
print_error:
    mov edi, 0xb8000
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x4f                    ; White on red
    stosw
    jmp .loop
.done:
    ret

; Switch to 64-bit mode
[bits 64]
long_mode_start:
    ; Clear segment registers
    xor ax, ax
    mov ss, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; Setup kernel stack
    mov rsp, kernel_stack_top
    
    ; Write long mode OK marker
    mov rdi, 0xb8000
    mov word [rdi + 10], 0x4F4C    ; "L"
    mov word [rdi + 12], 0x4F4F    ; "O"
    mov word [rdi + 14], 0x4F4E    ; "N"
    mov word [rdi + 16], 0x4F47    ; "G"
    
    ; Get multiboot info from saved location
    mov edi, [multiboot_info]      ; Get saved multiboot info
    and rdi, 0xFFFFFFFF            ; Clear upper 32 bits
    
    ; Write stage2 call marker
    mov rsi, 0xb8000
    mov word [rsi + 18], 0x4F32    ; "2" - Calling stage2
    
    ; Call stage2_main with multiboot info
    extern stage2_main
    call stage2_main
    
    ; Should never return
    mov rsi, 0xb8000
    mov word [rsi + 20], 0x4F52    ; "R" - Stage2 returned (error)
    mov word [rsi + 22], 0x4F45    ; "E"
    mov word [rsi + 24], 0x4F54    ; "T"
    
.hang:
    cli
    hlt
    jmp .hang

; Global Descriptor Table for 64-bit
section .rodata
align 8
gdt64:
    dq 0                            ; Null descriptor
.code: equ $ - gdt64
    dq (1<<44) | (1<<47) | (1<<41) | (1<<43) | (1<<53) ; Code segment
.data: equ $ - gdt64  
    dq (1<<44) | (1<<47) | (1<<41)  ; Data segment
.pointer:
    dw $ - gdt64 - 1                ; Length
    dq gdt64                        ; Address

; Error messages
section .data
no_multiboot_msg: db "ERROR: Not loaded by multiboot2 bootloader", 0
no_cpuid_msg: db "ERROR: CPUID not supported", 0
no_long_mode_msg: db "ERROR: Long mode not supported", 0

; Data section for multiboot info storage
align 4
multiboot_info: dd 0               ; Storage for multiboot info pointer
_data_end:

section .bss
align 4
_bss_end: