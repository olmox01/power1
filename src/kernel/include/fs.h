/*
 * Power1 OS - File System Interface
 * VFS and file system abstractions
 */

#ifndef _FS_H
#define _FS_H

#include "stdint.h"
#include "stddef.h"

/* File types */
#define S_IFREG     0x8000  /* Regular file */
#define S_IFDIR     0x4000  /* Directory */
#define S_IFCHR     0x2000  /* Character device */
#define S_IFBLK     0x6000  /* Block device */
#define S_IFIFO     0x1000  /* FIFO */

/* File permissions */
#define S_IRUSR     0x0100  /* Read by owner */
#define S_IWUSR     0x0080  /* Write by owner */
#define S_IXUSR     0x0040  /* Execute by owner */
#define S_IRGRP     0x0020  /* Read by group */
#define S_IWGRP     0x0010  /* Write by group */
#define S_IXGRP     0x0008  /* Execute by group */
#define S_IROTH     0x0004  /* Read by others */
#define S_IWOTH     0x0002  /* Write by others */
#define S_IXOTH     0x0001  /* Execute by others */

/* File descriptor structure */
struct file_descriptor {
    uint32_t fd;
    uint32_t flags;
    uint64_t offset;
    struct inode *inode;
    struct file_operations *ops;
};

/* Inode structure */
struct inode {
    uint32_t ino;
    uint16_t mode;
    uint16_t nlink;
    uint32_t uid;
    uint32_t gid;
    uint64_t size;
    uint64_t atime;
    uint64_t mtime;
    uint64_t ctime;
    struct file_operations *ops;
    void *private_data;
};

/* File operations */
struct file_operations {
    int (*open)(struct inode *inode, struct file_descriptor *fd);
    int (*close)(struct file_descriptor *fd);
    ssize_t (*read)(struct file_descriptor *fd, void *buf, size_t count);
    ssize_t (*write)(struct file_descriptor *fd, const void *buf, size_t count);
    int (*ioctl)(struct file_descriptor *fd, uint32_t cmd, void *arg);
};

/* Function prototypes */
int filesystem_init(void);
int vfs_mount(const char *source, const char *target, const char *fstype);
struct file_descriptor *vfs_open(const char *pathname, int flags);
int vfs_close(struct file_descriptor *fd);
ssize_t vfs_read(struct file_descriptor *fd, void *buf, size_t count);
ssize_t vfs_write(struct file_descriptor *fd, const void *buf, size_t count);

#endif /* _FS_H */
