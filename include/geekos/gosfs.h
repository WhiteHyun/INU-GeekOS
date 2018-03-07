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
#ifndef GOSFS_H
#define GOSFS_H


#include <geekos/blockdev.h>
#include <geekos/fileio.h>
#include <geekos/vfs.h>


typedef struct {
    char name[64];              /* name of file */
    unsigned int size;          /* size of the file */
    unsigned int isUsed:1;      /* is entry active */
    unsigned int isDirectory:1; /* is this file a directory */
    unsigned int isSetUid:1;    /* is this file setuid */
    int blocks[10];             /* 8 are for direct blocks, 1 for indirect and 1 for double indirect */
    struct VFS_ACL_Entry acls[VFS_MAX_ACL_ENTRIES];
} GOSFSfileNode;

/* should fit in one block */
#define MAX_FILES_PER_DIR  (4096/(sizeof(GOSFSfileNode)))

typedef struct {
    GOSFSfileNode files[MAX_FILES_PER_DIR];     /* all of the files */
} GOSFSdirectory;

typedef struct {
    // cached version of file node
    GOSFSfileNode node;

    //location of filenode on disk:
    unsigned int blockNum;      /* number of block containing filenode */
    unsigned int offset;        /* offset of filenode within block     */
} GOSFSptr;

int GOSFS_Format(struct Block_Device *blockDev);
int GOSFS_Mount(struct Mount_Point *mountPoint);
int GOSFS_Open(struct Mount_Point *mountPoint, const char *path, int mode,
               struct File **pFile);
int GOSFS_Close(struct File *file);
int GOSFS_Delete(struct Mount_Point *mountPoint, const char *path,
                 bool recursive);
int GOSFS_SetSetUid(struct Mount_Point *mountPoint, const char *path,
                    int setuid);
int GOSFS_SetAcl(struct Mount_Point *mountPoint, const char *path,
                 int uid, int permissions);
int GOSFS_Read(struct File *file, void *buf, ulong_t numBytes);
int GOSFS_Write(struct File *file, void *buf, ulong_t numBytes);
int GOSFS_FStat(struct File *file, struct VFS_File_Stat *stat);
int GOSFS_Seek(struct File *file, ulong_t pos);
int GOSFS_Create_Directory(struct Mount_Point *mountPoint,
                           const char *path);
int GOSFS_Read_Entry(struct File *dir, struct VFS_Dir_Entry *entry);
int GOSFS_Open_Directory(struct Mount_Point *mountPoint, const char *path,
                         struct File **pDir);

int GOSFS_Stat(struct Mount_Point *mountPoint, const char *path,
               struct VFS_File_Stat *stat);
int GOSFS_Sync(struct Mount_Point *mountPoint);

void Init_GOSFS();

#endif
