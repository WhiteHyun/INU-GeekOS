/*
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
#include <geekos/list.h>
#include <geekos/gosfs.h>
#include <geekos/vfs.h>
#include <geekos/string.h>
#include <geekos/projects.h>


// You will implement these for project 5


/*
 * Format a drive with GOSFS.
 */
int GOSFS_Format(struct Block_Device *blockDev) {
    TODO_P(PROJECT_GOSFS, "GeekOS file system Format operation");
    return EUNSUPPORTED;
}

/*
 * Mount GOSFS. Return 0 on success, return < 0 on failure.
 * - Check that the magic number is correct.
 */
int GOSFS_Mount(struct Mount_Point *mountPoint) {
    TODO_P(PROJECT_GOSFS, "GeekOS file system Mount operation");
    return EUNSUPPORTED;
}

/*
 * Get metadata for given File. Called with a file descriptor.
 */
int GOSFS_FStat(struct File *file, struct VFS_File_Stat *stat) {
    TODO_P(PROJECT_GOSFS, "GeekOS file system FStat operation");
    return EUNSUPPORTED;
}

/*
 * Open a file with the given name and mode.
 * Return > 0 on success, < 0 on failure (e.g. does not exist).
 */
int GOSFS_Open(struct Mount_Point *mp, const char *const_path, int mode,
               struct File **pFile) {
    TODO_P(PROJECT_GOSFS, "GeekOS file system Open operation");
    return EUNSUPPORTED;
}

/*
 * Read data from current position in file.
 * Return > 0 on success, < 0 on failure.
 */
int GOSFS_Read(struct File *file, void *ptr, ulong_t numBytes) {
    TODO_P(PROJECT_GOSFS, "GeekOS file system Read operation");
    return EUNSUPPORTED;
}

/*
 * Write data to current position in file.
 * Return > 0 on success, < 0 on failure.
 */
int GOSFS_Write(struct File *file, void *ptr, ulong_t numBytes) {
    TODO_P(PROJECT_GOSFS, "GeekOS file system Write operation");
    return EUNSUPPORTED;
}

/*
 * Get metadata for given file. Need to find the file from the given path.
 */
int GOSFS_Stat(struct Mount_Point *mountPoint, const char *path,
               struct VFS_File_Stat *stat) {
    TODO_P(PROJECT_GOSFS, "GeekOS file system Stat operation");
    return EUNSUPPORTED;
}

/*
 * Synchronize the filesystem data with the disk
 * (i.e., flush out all buffered filesystem data).
 */
int GOSFS_Sync(struct Mount_Point *mountPoint) {
    TODO_P(PROJECT_GOSFS, "GeekOS file system Sync operation");
    return EUNSUPPORTED;
}

/*
 * Close a file.
 */
int GOSFS_Close(struct File *file) {
    TODO_P(PROJECT_GOSFS, "GeekOS file system Close operation");
    return EUNSUPPORTED;
}

/*
 * Create a directory named by given path.
 */
int GOSFS_Create_Directory(struct Mount_Point *mp, const char *const_path) {
    TODO_P(PROJECT_GOSFS,
           "GeekOS file system Create Directory operation");
    return EUNSUPPORTED;
}

/*
 * Open a directory named by given path.
 */
int GOSFS_Open_Directory(struct Mount_Point *mountPoint, const char *path,
                         struct File **pDir) {
    TODO_P(PROJECT_GOSFS, "GeekOS file system Open Directory operation");
    return EUNSUPPORTED;
}

/*
 * Seek to a position in file. Should not seek beyond the end of the file.
 */
int GOSFS_Seek(struct File *file, ulong_t pos) {
    TODO_P(PROJECT_GOSFS, "GeekOS file system Seek operation");
    return EUNSUPPORTED;
}

/*
 * Delete the given file.
 * Return > 0 on success, < 0 on failure.
 */
int GOSFS_Delete(struct Mount_Point *mp, const char *const_path,
                 bool recursive) {
    TODO_P(PROJECT_GOSFS, "GeekOS file system Delete operation");
    return EUNSUPPORTED;
}


int GOSFS_SetSetUid(struct Mount_Point *mp, const char *file, int setuid) {
    struct File *pFile;

    TODO_P(PROJECT_USER, "GeekOS file system SetSetUID operation");
    return EUNSUPPORTED;
}

int GOSFS_SetAcl(struct Mount_Point *mp, const char *file, int uid,
                 int permissions) {
    TODO_P(PROJECT_USER, "GeekOS file system SetAcl operation");
    return EUNSUPPORTED;
}

/*
 * Read a directory entry from an open directory.
 */
int GOSFS_Read_Entry(struct File *file, struct VFS_Dir_Entry *entry) {
    TODO_P(PROJECT_GOSFS, "GeekOS file system Read Directory operation");
    return EUNSUPPORTED;
}

static struct Filesystem_Ops s_gosfsFilesystemOps = {
    &GOSFS_Format,
    &GOSFS_Mount,
};

/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

void Init_GOSFS(void) {
    Register_Filesystem("gosfs", &s_gosfsFilesystemOps);
}
