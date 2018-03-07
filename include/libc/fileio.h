/*
 * User File I/O
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
 * $Revision: 1.23 $
 *
 */

#ifndef FILEIO_H
#define FILEIO_H

#include <geekos/fileio.h>

int Stat(const char *path, struct VFS_File_Stat *stat);
int FStat(int fd, struct VFS_File_Stat *stat);
int Open(const char *path, int mode);
int Create_Directory(const char *path);
int Open_Directory(const char *path);
int Close(int fd);
int Read_Entry(int fd, struct VFS_Dir_Entry *dirEntry);
int Read(int fd, void *buf, unsigned long len);
int Write(int fd, const void *buf, unsigned long len);
int Sync(void);
int Mount(const char *dev, const char *prefix, const char *fstype);
int Seek(int fd, int pos);
int Delete(const char *path, bool recursive);
int Rename(const char *oldpath, const char *newpath);
int Link(const char *oldpath, const char *newpath);
int SymLink(const char *oldpath, const char *newpath);
int ReadBlock(const char *path, void *buf, unsigned int len,
              unsigned int blocknum);
int WriteBlock(const char *path, void *buf, unsigned int len,
               unsigned int blocknum);

int Format(const char *device, const char *filesystem_type);

int PlaySoundFile(const char *filename);

int Pipe(int *read_fd, int *write_fd);

int Diagnostic(void);
int Disk_Properties(const char *path, unsigned int *block_size,
                    unsigned int *blocks_on_disk);

int SetSetUid(const char *path, int setUid);
int SetAcl(const char *path, int user, int permissions);

int SetEffectiveUid(int uid);

#endif /* FILEIO_H */
