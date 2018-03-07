/*
 * Kernel data types
 * Copyright (c) 2001,2003 David H. Hovemeyer <daveho@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.13 $
 * 
 */

#ifndef GEEKOS_KTYPES_H
#define GEEKOS_KTYPES_H

/*
 * GeekOS uses the C99 bool type, with true and false
 * constant values.
 */
#include <stdbool.h>

/*
 * Shorthand for commonly used integer types.
 */
typedef unsigned long ulong_t;
typedef unsigned int uint_t;
typedef unsigned short ushort_t;
typedef unsigned char uchar_t;

/*
 * MIN() and MAX() macros.
 * By using gcc extensions, they are type-correct and
 * evaulate their arguments only once.
 */
#define MIN(a,b) ({typeof (a) _a = (a); typeof (b) _b = (b); (_a < _b) ? _a : _b; })
#define MAX(a,b) ({typeof (a) _a = (a); typeof (b) _b = (b); (_a > _b) ? _a : _b; })

/*
 * Some ASCII character access and manipulation macros.
 */
#define ISDIGIT(c) ((c) >= '0' && (c) <= '9')
#define TOLOWER(c) (((c) >= 'A' && (c) <= 'Z') ? ((c) + ('a' - 'A')) : (c))
#define TOUPPER(c) (((c) >= 'a' && (c) <= 'z') ? ((c) - ('a' - 'A')) : (c))

#endif /* GEEKOS_KTYPES_H */
