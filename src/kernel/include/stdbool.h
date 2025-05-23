/*
 * Power1 OS - Boolean Type
 * Freestanding kernel implementation of stdbool.h
 */

#ifndef _STDBOOL_H
#define _STDBOOL_H

#ifndef __cplusplus

#define bool    _Bool
#define true    1
#define false   0

#define __bool_true_false_are_defined 1

#endif /* __cplusplus */

#endif /* _STDBOOL_H */
