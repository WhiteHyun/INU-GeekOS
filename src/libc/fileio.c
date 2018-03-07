/*
 * User File I/O
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2004, David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.29 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/errno.h>
#include <geekos/syscall.h>
#include <fileio.h>
#include <string.h>

DEF_SYSCALL(Stat, SYS_STAT, int,
            (const char *filename, struct VFS_File_Stat * stat),
            const char *arg0 = filename;
            size_t arg1 = strlen(filename);
            struct VFS_File_Stat *arg2 = stat;
            , SYSCALL_REGS_3)
DEF_SYSCALL(FStat, SYS_FSTAT, int, (int fd, struct VFS_File_Stat * stat),
            int arg0 = fd;
            struct VFS_File_Stat *arg1 = stat;
            , SYSCALL_REGS_2)
DEF_SYSCALL(Open, SYS_OPEN, int, (const char *filename, int mode),
            const char *arg0 = filename;
            size_t arg1 = strlen(filename);
            int arg2 = mode;
            , SYSCALL_REGS_3)
DEF_SYSCALL(Create_Directory, SYS_CREATEDIR, int, (const char *dirname),
            const char *arg0 = dirname;
            size_t arg1 = strlen(dirname);
            , SYSCALL_REGS_2)
DEF_SYSCALL(Open_Directory, SYS_OPENDIRECTORY, int, (const char *dirname),
            const char *arg0 = dirname;
            size_t arg1 = strlen(dirname);
            , SYSCALL_REGS_2)
DEF_SYSCALL(Close, SYS_CLOSE, int, (int fd), int arg0 = fd;
            , SYSCALL_REGS_1)
DEF_SYSCALL(Read_Entry, SYS_READENTRY, int,
                (int fd, struct VFS_Dir_Entry * entry), int arg0 = fd;
            struct VFS_Dir_Entry *arg1 = entry;
            , SYSCALL_REGS_2)
DEF_SYSCALL(Read, SYS_READ, int, (int fd, void *buf, ulong_t len),
            int arg0 = fd;
            void *arg1 = buf;
            ulong_t arg2 = len;
            , SYSCALL_REGS_3)
DEF_SYSCALL(Write, SYS_WRITE, int, (int fd, const void *buf, ulong_t len),
            int arg0 = fd;
            const void *arg1 = buf;
            ulong_t arg2 = len;
            , SYSCALL_REGS_3)
DEF_SYSCALL(Sync, SYS_SYNC, int, (void),, SYSCALL_REGS_0)
    DEF_SYSCALL(Format, SYS_FORMAT, int,
                (const char *devname, const char *fstype),
                const char *arg0 = devname;
                size_t arg1 = strlen(devname);
                const char *arg2 = fstype;
                size_t arg3 = strlen(fstype);
                , SYSCALL_REGS_4)
DEF_SYSCALL(Seek, SYS_SEEK, int, (int fd, int pos), int arg0 = fd;
            int arg1 = pos;
            , SYSCALL_REGS_2)
DEF_SYSCALL(Delete, SYS_DELETE, int, (const char *path, bool recursive),
            const char *arg0 = path;
            size_t arg1 = strlen(path);
            bool arg2 = recursive;
            , SYSCALL_REGS_3)
DEF_SYSCALL(Rename, SYS_RENAME, int,
                (const char *oldpath, const char *newpath),
            const char *arg0 = oldpath;
            size_t arg1 = strlen(oldpath);
            const char *arg2 = newpath;
            size_t arg3 = strlen(newpath);
            , SYSCALL_REGS_4)
DEF_SYSCALL(Link, SYS_LINK, int,
                (const char *oldpath, const char *newpath),
            const char *arg0 = oldpath;
            size_t arg1 = strlen(oldpath);
            const char *arg2 = newpath;
            size_t arg3 = strlen(newpath);
            , SYSCALL_REGS_4)
DEF_SYSCALL(SymLink, SYS_SYMLINK, int,
                (const char *oldpath, const char *newpath),
            const char *arg0 = oldpath;
            size_t arg1 = strlen(oldpath);
            const char *arg2 = newpath;
            size_t arg3 = strlen(newpath);
            , SYSCALL_REGS_4)
DEF_SYSCALL(ReadBlock, SYS_READBLOCK, int,
                (const char *path, void *buf, unsigned int len,
                 unsigned int blocknum), const char *arg0 = path;
            size_t arg1 = strlen(path);
            void *arg2 = buf;
            unsigned int arg3 = len;
            unsigned int arg4 = blocknum;
            , SYSCALL_REGS_5)
DEF_SYSCALL(WriteBlock, SYS_WRITEBLOCK, int,
                (const char *path, void *buf, unsigned int len,
                 unsigned int blocknum), const char *arg0 = path;
            size_t arg1 = strlen(path);
            void *arg2 = buf;
            unsigned int arg3 = len;
            unsigned int arg4 = blocknum;
            , SYSCALL_REGS_5)

DEF_SYSCALL(PlaySoundFile, SYS_PLAY_SOUND_FILE, int, (const char *path),
            const char *arg0 = path;
            size_t arg1 = strlen(path);
            , SYSCALL_REGS_2)

DEF_SYSCALL(Pipe, SYS_PIPE, int, (int *read_fd, int *write_fd),
            int *arg0 = read_fd;
            int *arg1 = write_fd;
            , SYSCALL_REGS_2)
DEF_SYSCALL(Diagnostic, SYS_DIAGNOSTIC, int, (void),, SYSCALL_REGS_0)
    DEF_SYSCALL(Disk_Properties, SYS_DISKPROPERTIES, int,
                (const char *path, unsigned int *block_size,
                 unsigned int *blocks_on_disk), const char *arg0 = path;
                size_t arg1 = strlen(path);
                unsigned int *arg2 = block_size;
                unsigned int *arg3 = blocks_on_disk;
                , SYSCALL_REGS_4)

DEF_SYSCALL(SetAcl, SYS_SET_ACL, int,
                (const char *file, int uid, int permissions),
            const char *arg0 = file;
            int arg1 = strlen(file);
            int arg2 = uid;
            int arg3 = permissions;
            , SYSCALL_REGS_4)
DEF_SYSCALL(GetUid, SYS_GET_UID, int, (),;
            , SYSCALL_REGS_0)
DEF_SYSCALL(SetEffectiveUid, SYS_SET_EFFECTIVE_UID, int, (int uid),
            int arg0 = uid;
            , SYSCALL_REGS_1)
DEF_SYSCALL(SetSetUid, SYS_SET_SET_UID, int, (const char *file, int uid),
            const char *arg0 = file;
            int arg1 = strlen(file);
            int arg2 = uid;
            , SYSCALL_REGS_3)

DEF_SYSCALL(Mmap, SYS_MMAP, int,
                (const void *addr, size_t length, int prot, int flags,
                 int fd), const void *arg0 = addr;
            size_t arg1 = length;
            int arg2 = prot;
            int arg3 = flags;
            int arg4 = fd;
            , SYSCALL_REGS_5)
DEF_SYSCALL(Munmap, SYS_MUNMAP, int, (const void *addr),
            const void *arg0 = addr;
            , SYSCALL_REGS_1)

static bool Copy_String(char *dst, const char *src, size_t len) {
    if(strnlen(src, len) == len)
        return false;
    strcpy(dst, src);
    return true;
}

/*
 * The Mount() system call requires special handling because
 * its arguments are passed in a struct, since too many registers
 * would be required to pass the arguments entirely in
 * registers.
 */
int Mount(const char *devname, const char *prefix, const char *fstype) {
    int num = SYS_MOUNT, rc;
    struct VFS_Mount_Request args;

    if(!Copy_String(args.devname, devname, sizeof(args.devname)) ||
       !Copy_String(args.prefix, prefix, sizeof(args.prefix)) ||
       !Copy_String(args.fstype, fstype, sizeof(args.fstype)))
        return EINVALID;

    __asm__ __volatile__(SYSCALL:"=a"(rc)
                         :"a"(num), "b"(&args)
        );
    return rc;
}
