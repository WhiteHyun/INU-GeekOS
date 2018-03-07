/*
 * GeekOS memory allocation API
 * Copyright (c) 2001, David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
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

#ifndef GEEKOS_MALLOC_H
#define GEEKOS_MALLOC_H

#include <geekos/ktypes.h>

void Init_Heap(ulong_t start, ulong_t size);
           /*@only@*//*@null@ */
void *Malloc(ulong_t size);
void Free( /*@only@ *//*@out@ *//*@null@ */ void *buf);

#endif /* GEEKOS_MALLOC_H */
