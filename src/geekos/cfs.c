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
#include <geekos/cfs.h>
#include <geekos/vfs.h>
#include <geekos/string.h>
#include <geekos/projects.h>
#include <geekos/mem.h>



// You will implement these for project 5


/*
 * Format a drive with CFS.
 */
int CFS_Format(struct Block_Device *blockDev) {
    TODO_P(PROJECT_CFS, "chameleon file system Format operation");
    return EUNSUPPORTED;
}

/*
 * Mount CFS. Return 0 on success, return < 0 on failure.
 * - Check that the magic number is correct.
 */
int CFS_Mount(struct Mount_Point *mountPoint) {
    TODO_P(PROJECT_CFS, "chameleon file system Mount operation");
    return EUNSUPPORTED;
}

/*
 * Get metadata for given File. Called with a file descriptor.
 */
int CFS_FStat(struct File *file, struct VFS_File_Stat *stat) {
    TODO_P(PROJECT_CFS, "chameleon file system FStat operation");
    return EUNSUPPORTED;
}



/*
 * Open a file with the given name and mode.
 * Return > 0 on success, < 0 on failure (e.g. does not exist).
 */
int CFS_Open(struct Mount_Point *mp, const char *path, int mode,
             struct File **pFile) {
    TODO_P(PROJECT_CFS, "chameleon file system Open operation");
    return EUNSUPPORTED;
}

/*
 * Read data from current position in file.
 * Return > 0 on success, < 0 on failure.
 */
int CFS_Read(struct File *file, void *ptr, ulong_t numBytes) {
    TODO_P(PROJECT_CFS, "chameleon file system Read operation");
    return EUNSUPPORTED;
}

/*
 * Write data to current position in file.
 * Return > 0 on success, < 0 on failure.
 */
int CFS_Write(struct File *file, void *ptr, ulong_t numBytes) {
    TODO_P(PROJECT_CFS, "chameleon file system Write operation");
    return EUNSUPPORTED;
}

/*
 * Get metadata for given file. Need to find the file from the given path.
 */
int CFS_Stat(struct Mount_Point *mountPoint, const char *path,
             struct VFS_File_Stat *stat) {
    TODO_P(PROJECT_CFS, "chameleon file system Stat operation");
    return EUNSUPPORTED;
}

/*
 * Synchronize the filesystem data with the disk
 * (i.e., flush out all buffered filesystem data).
 */
int CFS_Sync(struct Mount_Point *mountPoint) {
    TODO_P(PROJECT_CFS, "chameleon file system Sync operation");
    return EUNSUPPORTED;
}

/*
 * Close a file.
 */
int CFS_Close(struct File *file) {
    TODO_P(PROJECT_CFS, "chameleon file system Close operation");
    return EUNSUPPORTED;
}

/*
 * Create a directory named by given path.
 */
int CFS_Create_Directory(struct Mount_Point *mp, const char *const_path) {
    TODO_P(PROJECT_CFS,
           "chameleon file system Create Directory operation");
    return EUNSUPPORTED;
}

/*
 * Open a directory named by given path.
 */
int CFS_Open_Directory(struct Mount_Point *mountPoint, const char *path,
                       struct File **pDir) {
    TODO_P(PROJECT_CFS, "chameleon file system Open Directory operation");
    return EUNSUPPORTED;
}

/*
 * Seek to a position in file. Should not seek beyond the end of the file.
 */
int CFS_Seek(struct File *file, ulong_t pos) {
    TODO_P(PROJECT_CFS, "chameleon file system Seek operation");
    return EUNSUPPORTED;
}


/*
 * Delete the given file.
 * Return > 0 on success, < 0 on failure.
 */
int CFS_Delete(struct Mount_Point *mp, const char *const_path,
               bool recursive) {
    TODO_P(PROJECT_CFS, "chameleon file system Delete operation");
    return EUNSUPPORTED;
}

int CFS_Rename(struct Mount_Point *mp, const char *oldpath,
               const char *newpath) {
    TODO_P(PROJECT_USER, "chameleon file system Rename operation");
    return EUNSUPPORTED;
}

int CFS_Link(struct Mount_Point *mp, const char *oldpath,
             const char *newpath) {
    TODO_P(PROJECT_USER, "chameleon file system Link operation");
    return EUNSUPPORTED;
}

int CFS_SymLink(struct Mount_Point *mp, const char *oldpath,
                const char *newpath) {
    TODO_P(PROJECT_USER, "chameleon file system SymLink operation");
    return EUNSUPPORTED;
}

int CFS_SetSetUid(struct Mount_Point *mp, const char *file, int setuid) {
    struct File *pFile;

    TODO_P(PROJECT_USER, "chameleon file system SetSetUID operation");
    return EUNSUPPORTED;
}

int CFS_SetAcl(struct Mount_Point *mp, const char *file, int uid,
               int permissions) {
    TODO_P(PROJECT_USER, "chameleon file system SetAcl operation");
    return EUNSUPPORTED;
}

/*
 * Read a directory entry from an open directory.
 */
int CFS_Read_Entry(struct File *file, struct VFS_Dir_Entry *entry) {
    TODO_P(PROJECT_CFS, "chameleon file system Read Directory operation");
    return EUNSUPPORTED;
}

static struct Filesystem_Ops s_cfsFilesystemOps = {
    &CFS_Format,
    &CFS_Mount,
};

/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

void Init_CFS(void) {
    Register_Filesystem("cfs", &s_cfsFilesystemOps);
}
