/*
 * Boot information structure, passed to kernel Main() routine
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
 * $Revision: 1.6 $
 * 
 */

#ifndef GEEKOS_BOOTINFO_H
#define GEEKOS_BOOTINFO_H

struct MemRegion {
    unsigned int baseAddr_low, baseAddr_high;
    unsigned int length_low, length_high;
    unsigned int type;
    unsigned int extendedAttributes;
};


struct Boot_Info {
    int bootInfoSize;           /* size of this struct; for versioning */
    int startKernInfo;          /* address of lowest kernel info (before 0xa0000) */
    int memSizeKB;              /* number of KB, as reported by int 15h  = zero mean user regions */
    int bootDrive;              /* 0,1 floppy 0x80-81 hard disks */
    int numMemRegions;          /* number of mem segments */
    struct MemRegion *memRegions;       /* array of memory Regions */
};

#endif /* GEEKOS_BOOTINFO_H */
