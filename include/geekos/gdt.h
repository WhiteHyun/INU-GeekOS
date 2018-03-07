/*
 * Initialize kernel GDT.
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
 * $Revision: 1.7 $
 * 
 */

#ifndef GEEKOS_GDT_H
#define GEEKOS_GDT_H

struct Segment_Descriptor;

void Init_GDT(int CPUid);
struct Segment_Descriptor *Allocate_Segment_Descriptor(void);
struct Segment_Descriptor *Allocate_Segment_Descriptor_On_CPU(int cpu);
void Free_Segment_Descriptor(struct Segment_Descriptor
                             *descriptor_to_free);
int Get_Descriptor_Index(struct Segment_Descriptor *desc);

#endif /* GEEKOS_GDT_H */
