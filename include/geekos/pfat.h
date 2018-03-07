/*
 * Header file for the pseudo-fat filesystem.
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
 * $Revision: 1.11 $
 *
 */

#ifndef GEEKOS_PFAT_H
#define GEEKOS_PFAT_H

#include "fileio.h"
#include "vfs.h"

/*
 * WARNING: When changing this code, check out setup.asm and bootsect.asm,
 *  which use fields of this structure!
 */
typedef struct {
    int magic;                  /* id to tell the type of filesystem */
    int fileAllocationOffset;   /* where is the file allocation table */
    int fileAllocationLength;   /* length of allocation table */
    int rootDirectoryOffset;    /* offset in sectors of root directory */
    int rootDirectoryCount;     /* number of items in the directory */
    short setupStart;           /* first sector of secondary loader */
    short setupSize;            /* size in sectors of secondary loader */
    short kernelStart;          /* first sector of kernel to run */
    short kernelSize;           /* size in sectors of kernel to run */
} bootSector;

typedef struct {
    char fileName[8 + 4];

    /* attribute bits */
    char readOnly:1;
    char hidden:1;
    char systemFile:1;
    char volumeLabel:1;
    char directory:1;
    char isSetUid:1;

    short time;
    short date;
    int firstBlock;
    int fileSize;
    struct VFS_ACL_Entry acls[VFS_MAX_ACL_ENTRIES];
} directoryEntry;

#define FAT_ENTRY_FREE		0
#define FAT_ENTRY_EOF		1

/* magic number to indicate its a PFAT disk */
#define PFAT_MAGIC		0x78320000

/* where in the boot sector is the pfat record */
#define PFAT_BOOT_RECORD_OFFSET 482

void Init_PFAT(void);

#endif /* GEEKOS_PFAT_H */
