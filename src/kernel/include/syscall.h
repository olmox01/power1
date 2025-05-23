/*
 * Power1 OS - System Call Interface
 * POSIX-compliant system call definitions
 */

#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "stdint.h"
#include "stddef.h"

/* System call numbers */
#define SYS_EXIT        1
#define SYS_FORK        2
#define SYS_READ        3
#define SYS_WRITE       4
#define SYS_OPEN        5
#define SYS_CLOSE       6
#define SYS_WAITPID     7
#define SYS_CREAT       8
#define SYS_LINK        9
#define SYS_UNLINK      10
#define SYS_EXECVE      11
#define SYS_CHDIR       12
#define SYS_TIME        13
#define SYS_MKNOD       14
#define SYS_CHMOD       15
#define SYS_LSEEK       19
#define SYS_GETPID      20

/* System call handler */
struct syscall_frame {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8, r9, r10, r11;
    uint64_t r12, r13, r14, r15;
    uint64_t rip, rflags;
} __attribute__((packed));

/* Function prototypes */
int syscall_interface_init(void);
uint64_t syscall_handler(struct syscall_frame *frame);

/* Individual system call handlers */
uint64_t sys_exit(int status);
uint64_t sys_fork(void);
uint64_t sys_read(int fd, void *buf, size_t count);
uint64_t sys_write(int fd, const void *buf, size_t count);
uint64_t sys_open(const char *pathname, int flags, int mode);
uint64_t sys_close(int fd);

#endif /* _SYSCALL_H */
