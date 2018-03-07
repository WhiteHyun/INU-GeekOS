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
#ifndef CFS_H
#define CFS_H


#include <geekos/blockdev.h>
#include <geekos/fileio.h>
#include <geekos/vfs.h>


typedef struct {
    unsigned int size;          /* size of the file */
    unsigned int refCount;      /* number of files names (hard links) that use this */
    unsigned int isUsed:1;      /* is entry active */
    unsigned int isDirectory:1; /* is this file a directory */
    unsigned int isSetUid:1;    /* is this file setuid */
    unsigned int isSymbolicLink:1;      /* is this a symbolic link */
    int blocks[10];             /* 8 are for direct blocks, 1 for indirect and 1 for double indirect */
    struct VFS_ACL_Entry acls[VFS_MAX_ACL_ENTRIES];
    char pad[36];               // pad to 128 bytes to fit into disk blocks
} CFSiNode;

#define INODES_PER_BLOCK  (4096/(sizeof(CFSiNode)))

typedef struct {
    char name[64];              /* name of file */
    unsigned int inode;         /* index of inode */
    unsigned int isUsed:1;      /* is this entry used */
} CFSfileNode;

/* should fit in one block */
#define MAX_CFILES_PER_DIR  (4096/(sizeof(CFSfileNode)))

typedef struct {
    CFSfileNode files[MAX_CFILES_PER_DIR];      /* all of the files */
} CFSdirectory;

typedef struct {
    // pointer cached version of file's inode
    CFSiNode *node;
    unsigned int inodeNum;      /* inode number of the file */
    unsigned int offset;        /* offset of filenode within block     */
} CFSptr;

typedef struct {
    int magic;
    int size;                   // in 4KB blocks
    int numInodes;
    int firstInodeBlock;
    unsigned int firstFreeInode;
} cfsHeader;

int CFS_Format(struct Block_Device *blockDev);
int CFS_Mount(struct Mount_Point *mountPoint);
int CFS_Open(struct Mount_Point *mountPoint, const char *path, int mode,
             struct File **pFile);
int CFS_Close(struct File *file);
int CFS_Delete(struct Mount_Point *mountPoint, const char *path,
               bool recursive);
int CFS_Rename(struct Mount_Point *mountPoint, const char *oldpath,
               const char *newpath);
int CFS_Link(struct Mount_Point *mountPoint, const char *oldpath,
             const char *newpath);
int CFS_SymLink(struct Mount_Point *mountPoint, const char *oldpath,
                const char *newpath);
int CFS_SetSetUid(struct Mount_Point *mountPoint, const char *path,
                  int setuid);
int CFS_SetAcl(struct Mount_Point *mountPoint, const char *path, int uid,
               int permissions);
int CFS_Read(struct File *file, void *buf, ulong_t numBytes);
int CFS_Write(struct File *file, void *buf, ulong_t numBytes);
int CFS_FStat(struct File *file, struct VFS_File_Stat *stat);
int CFS_Seek(struct File *file, ulong_t pos);
int CFS_Create_Directory(struct Mount_Point *mountPoint,
                         const char *path);
int CFS_Read_Entry(struct File *dir, struct VFS_Dir_Entry *entry);
int CFS_Open_Directory(struct Mount_Point *mountPoint, const char *path,
                       struct File **pDir);

int CFS_Stat(struct Mount_Point *mountPoint, const char *path,
             struct VFS_File_Stat *stat);
int CFS_Sync(struct Mount_Point *mountPoint);

void Init_CFS();

#endif
