/*
 * GeekOS file system
 * Copyright (c) 2008, David H. Hovemeyer <daveho@cs.umd.edu>, 
 * Neil Spring <nspring@cs.umd.edu>, Aaron Schulman <schulman@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 */


#include <limits.h>
#include <geekos/errno.h>
#include <geekos/kassert.h>
#include <geekos/screen.h>
#include <geekos/malloc.h>
#include <geekos/string.h>
#include <geekos/bitset.h>
#include <geekos/synch.h>
#include <geekos/bufcache.h>
#include <geekos/gfs3.h>
#include <geekos/pfat.h>
#include <geekos/projects.h>

/* ----------------------------------------------------------------------
 * Private data and functions
 * ---------------------------------------------------------------------- */


/* ----------------------------------------------------------------------
 * Implementation of VFS operations
 * ---------------------------------------------------------------------- */

/*
 * Get metadata for given file.
 */
static int GFS3_FStat(struct File *file, struct VFS_File_Stat *stat) {
    TODO_P(PROJECT_GFS3, "GeekOS filesystem FStat operation");
    return 0;
}

/*
 * Read data from current position in file.
 */
static int GFS3_Read(struct File *file, void *buf, ulong_t numBytes) {
    TODO_P(PROJECT_GFS3, "GeekOS filesystem read operation");

    return EUNSUPPORTED;
}

/*
 * Write data to current position in file.
 */
static int GFS3_Write(struct File *file, void *buf, ulong_t numBytes) {
    TODO_P(PROJECT_GFS3, "GeekOS filesystem write operation");
    return EUNSUPPORTED;
}


/*
 * Seek to a position in file; returns 0 on success.
 */
static int GFS3_Seek(struct File *file, ulong_t pos) {
    TODO_P(PROJECT_GFS3, "GeekOS filesystem seek operation");
    return EUNSUPPORTED;
}

/*
 * Close a file.
 */
static int GFS3_Close(struct File *file) {
    TODO_P(PROJECT_GFS3, "GeekOS filesystem close operation");
    return EUNSUPPORTED;
}

/*static*/ struct File_Ops s_gfs3FileOps = {
    &GFS3_FStat,
    &GFS3_Read,
    &GFS3_Write,
    &GFS3_Seek,
    &GFS3_Close,
    0,                          /* Read_Entry */
};

/*
 * Stat operation for an already open directory.
 */
static int GFS3_FStat_Directory(struct File *dir,
                                struct VFS_File_Stat *stat) {
    /* may be unused. */
    TODO_P(PROJECT_GFS3, "GeekOS filesystem FStat directory operation");
    return 0;
}

/*
 * Directory Close operation.
 */
static int GFS3_Close_Directory(struct File *dir) {
    TODO_P(PROJECT_GFS3, "GeekOS filesystem Close directory operation");
    return EUNSUPPORTED;
}

/*
 * Read a directory entry from an open directory.
 */
static int GFS3_Read_Entry(struct File *dir, struct VFS_Dir_Entry *entry) {
    TODO_P(PROJECT_GFS3, "GeekOS filesystem Read_Entry operation");
    return EUNSUPPORTED;
}

/*static*/ struct File_Ops s_gfs3DirOps = {
    &GFS3_FStat,
    0,                          /* Read */
    0,                          /* Write */
    0,                          /* Seek */
    &GFS3_Close_Directory,
    &GFS3_Read_Entry,
};



/*
 * Open a file named by given path.
 */
static int GFS3_Open(struct Mount_Point *mountPoint, const char *path,
                     int mode, struct File **pFile) {
    TODO_P(PROJECT_GFS3, "GeekOS filesystem open operation");
    return EUNSUPPORTED;
}

/*
 * Create a directory named by given path.
 */
static int GFS3_Create_Directory(struct Mount_Point *mountPoint,
                                 const char *path) {
    TODO_P(PROJECT_GFS3, "GeekOS filesystem create directory operation");
    return EUNSUPPORTED;
}

/*
 * Open a directory named by given path.
 */
static int GFS3_Open_Directory(struct Mount_Point *mountPoint,
                               const char *path, struct File **pDir) {
    TODO_P(PROJECT_GFS3, "GeekOS filesystem open directory operation");
    return EUNSUPPORTED;
}

/*
 * Open a directory named by given path.
 */
static int GFS3_Delete(struct Mount_Point *mountPoint, const char *path,
                       bool recursive) {
    TODO_P(PROJECT_GFS3, "GeekOS filesystem delete operation");
    return EUNSUPPORTED;
}

/*
 * Get metadata (size, permissions, etc.) of file named by given path.
 */
static int GFS3_Stat(struct Mount_Point *mountPoint, const char *path,
                     struct VFS_File_Stat *stat) {
    TODO_P(PROJECT_GFS3, "GeekOS filesystem stat operation");
    return EUNSUPPORTED;
}

/*
 * Synchronize the filesystem data with the disk
 * (i.e., flush out all buffered filesystem data).
 */
static int GFS3_Sync(struct Mount_Point *mountPoint) {
    TODO_P(PROJECT_GFS3, "GeekOS filesystem sync operation");
    return EUNSUPPORTED;
}

static int GFS3_Disk_Properties(struct Mount_Point *mountPoint,
                                unsigned int *block_size,
                                unsigned int *blocks_in_disk) {
    TODO_P(PROJECT_GFS3,
           "GeekOS filesystem infomation operation; set variables.");
    return EUNSUPPORTED;
}

/*static*/ struct Mount_Point_Ops s_gfs3MountPointOps = {
    &GFS3_Open,
    &GFS3_Create_Directory,
    &GFS3_Open_Directory,
    &GFS3_Stat,
    &GFS3_Sync,
    &GFS3_Delete,
    0,                          /* Rename  */
    0,                          /* Link  */
    0,                          /* SymLink  */
    0,                          /* setuid  */
    0,                          /* acl  */
    &GFS3_Disk_Properties,
};

static int GFS3_Format(struct Block_Device *blockDev
                       __attribute__ ((unused))) {
    TODO_P(PROJECT_GFS3,
           "DO NOT IMPLEMENT: There is no format operation for GFS3");
    return EUNSUPPORTED;
}

static int GFS3_Mount(struct Mount_Point *mountPoint) {
    TODO_P(PROJECT_GFS3, "GeekOS filesystem mount operation");
    return EUNSUPPORTED;
}


static struct Filesystem_Ops s_gfs3FilesystemOps = {
    &GFS3_Format,
    &GFS3_Mount,
};

/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

void Init_GFS3(void) {
    Register_Filesystem("gfs3", &s_gfs3FilesystemOps);
}
