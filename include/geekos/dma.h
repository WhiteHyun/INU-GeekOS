/*
 * 8237A DMA Controller Support
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2003,2013,2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.10 $
 * 
 */

#ifndef GEEKOS_DMA_H
#define GEEKOS_DMA_H

#include <geekos/ktypes.h>

enum DMA_Direction {
    DMA_READ,
    DMA_WRITE
};

void Init_DMA(void);
bool Reserve_DMA(int chan);
void Setup_DMA(enum DMA_Direction direction, int chan, void *addr,
               ulong_t size);

void Mask_DMA(int chan);
void Unmask_DMA(int chan);

#endif /* GEEKOS_DMA_H */
