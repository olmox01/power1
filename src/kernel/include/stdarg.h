/*
 * Power1 OS - Variable Arguments
 * Freestanding kernel implementation of stdarg.h
 */

#ifndef _STDARG_H
#define _STDARG_H

/* Variable argument list type */
typedef __builtin_va_list va_list;

/* Start variable argument processing */
#define va_start(ap, last) __builtin_va_start(ap, last)

/* Get next argument */
#define va_arg(ap, type) __builtin_va_arg(ap, type)

/* End variable argument processing */
#define va_end(ap) __builtin_va_end(ap)

/* Copy variable argument list */
#define va_copy(dest, src) __builtin_va_copy(dest, src)

#endif /* _STDARG_H */
