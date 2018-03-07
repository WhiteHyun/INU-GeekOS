/*
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2003,2013,2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2015 Neil Spring <nspring@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 */
#ifndef GFS3_H
#define GFS3_H

#include <geekos/vfs.h>
#include <geekos/fileio.h>

/* ==Typedefs== */
typedef unsigned int gfs3_blocknum;     /* For disk block numbers */
typedef unsigned int gfs3_inodenum;     /* For inode numbers */

/* ==Constants== */
#define GFS3_DIRECTORY 1
#define GFS3_FILE 2

#define GFS3_MAGIC 0x47465333

#define GFS3_INUM_ROOT   1
#define GFS3_INUM_INUSE_BITS 2

/* ==Structures== */
struct gfs3_superblock {
    unsigned int gfs3_magic;    /* 0x47465333 */
    unsigned int gfs3_version;  /* 0x00000100 */
    gfs3_blocknum block_with_inode_zero;        /* likely 4. */
    unsigned int number_of_inodes;      /* calculated on format */
    unsigned int blocks_per_disk;       /* blocks in total */
    short setupStart;           /* first sector of secondary loader */
    short setupSize;            /* size in sectors of secondary loader */
    short kernelStart;          /* first sector of kernel to run */
    short kernelSize;           /* size in sectors of kernel to run */
};

struct gfs3_extent {
    unsigned int start_block;   /* if zero, this is a sparse extent (all zeroes) */
    /* sparse extents are unlikely to be used without 
       extent trees */
    unsigned int length_blocks; /* if zero, this is an invalid extent (unused) 
                                   length must not be larger than 1<<32 / 1<<9 == 1<<23 
                                   due to file size being an unsigned int. */
};
#define GFS3_EXTENTS 3
struct gfs3_inode {
    unsigned int size;          /* size of the data stored in the direct and indirect blocks  */
    unsigned char type;         /* GFS3_DIRECTORY = 1, GFS3_FILE = 2, */
    unsigned char reference_count;      /* 1 (for each directory entry pointing to this node, which shall be 1 since we don't link) */
    unsigned short mode;        /* permissions bits; ignore */
    struct gfs3_extent extents[GFS3_EXTENTS];   /* if more than three are needed, must coalesce */
};

/* note that a gfs3_dirent is a variably sized structure */
struct gfs3_dirent {
    gfs3_inodenum inum;         /* *must* be word-aligned */
    unsigned char entry_length; /* after the inum.  i.e., the min entry length is 4.  max is 252, for alignment */
    unsigned char name_length;  /* may be < entry length - 2. */
    char name[2];               /* where overrun provides the rest of the name; NOT (necessarily) null terminated. */
};

void Init_GFS3(void);


#endif
