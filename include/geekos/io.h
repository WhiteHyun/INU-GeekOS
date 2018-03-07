/*
 * x86 port IO routines
 * Copyright (c) 2001, David H. Hovemeyer <daveho@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.9 $
 * 
 */

#ifndef GEEKOS_IO_H
#define GEEKOS_IO_H

#include <geekos/ktypes.h>

void Out_Byte(ushort_t port, uchar_t value);
uchar_t In_Byte(ushort_t port);

void Out_Word(ushort_t port, ushort_t value);
ushort_t In_Word(ushort_t port);

void IO_Delay(void);

#endif /* GEEKOS_IO_H */
