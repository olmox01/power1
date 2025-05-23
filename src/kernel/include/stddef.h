/*
 * Power1 OS - Standard Definitions
 * Freestanding kernel implementation of stddef.h
 */

#ifndef _STDDEF_H
#define _STDDEF_H

/* NULL pointer constant */
#define NULL ((void*)0)

/* Boolean type for C */
#ifndef __cplusplus
typedef enum {
    false = 0,
    true = 1
} bool;
#endif

/* Size type */
typedef unsigned long size_t;

/* Signed size type */
typedef long ssize_t;

/* Pointer difference type */
typedef long ptrdiff_t;

/* Wide character type */
typedef int wchar_t;

/* Maximum alignment type */
typedef long double max_align_t;

/* Offset of member in structure */
#define offsetof(type, member) __builtin_offsetof(type, member)

#endif /* _STDDEF_H */
