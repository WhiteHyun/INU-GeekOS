/*
 * GeekOS system calls
 * Copyright (c) 2001,2003 David H. Hovemeyer <daveho@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.36 $
 *
 */

#ifndef GEEKOS_SYSCALL_H
#define GEEKOS_SYSCALL_H

#if defined(GEEKOS)

struct Interrupt_State;

/*
 * Function signature of a system call.
 */
typedef int (*Syscall) (struct Interrupt_State * state);

/*
 * Number of system calls implemented.
 */
extern const unsigned int g_numSyscalls;

/*
 * Table of system call handler functions.
 */
extern const Syscall g_syscallTable[];

#endif /* defined(GEEKOS) */

#define SYSCALL "int $0x90"     /* Assembly instruction for the system call trap. */

/*
 * System call numbers
 */
enum {
    SYS_NULL,                   /* Null (no-op) system call  */
    SYS_EXIT,                   /* Exit system call  */
    SYS_PRINTSTRING,            /* Print string system call  */
    SYS_GETKEY,                 /* Get key system call  */
    SYS_SETATTR,                /* Set screen attribute system call  */
    SYS_GETCURSOR,              /* Get current cursor position */
    SYS_PUTCURSOR,              /* Put current cursor position */
    SYS_SPAWN,                  /* Spawn process system call  */
    SYS_WAIT,                   /* Wait for child process to exit system call  */
    SYS_GETPID,                 /* Get pid (process id) system call  */
    SYS_KILL,                   /* Kill a process */
    SYS_PS,                     /* Get the current process table */
    SYS_SIGNAL,                 /* Register a signal handler for a signal */
    SYS_REGDELIVER,             /* Register user-space handler routines */
    SYS_RETURNSIG,              /* Called when signal handler is done executing */
    SYS_WAITNOPID,              /* Like Wait, but doesn't require a PID. */
    SYS_SETSCHEDULINGPOLICY,    /* Set scheduler policy system call  */
    SYS_GETTIMEOFDAY,           /* Get time of day system call  */
    SYS_OPEN_SEMAPHORE,         /* Create semaphore system call  */
    SYS_P,                      /* P (acquire semaphore) system call  */
    SYS_V,                      /* V (release semaphore) system call  */
    SYS_CLOSE_SEMAPHORE,        /* Destroy semaphore system call  */
    SYS_MOUNT,                  /* Mount filesystem system call  */
    SYS_OPEN,                   /* Open file system call  */
    SYS_OPENDIRECTORY,          /* Open directory system call  */
    SYS_CLOSE,                  /* Close file system call  */
    SYS_DELETE,                 /* Delete file system call  */
    SYS_READ,                   /* Read from file system call  */
    SYS_READENTRY,              /* Read directory entry system call  */
    SYS_WRITE,                  /* Write to file system call  */
    SYS_STAT,                   /* Stat system call  */
    SYS_FSTAT,                  /* FStat system call  */
    SYS_SEEK,                   /* Seek in file system call  */
    SYS_CREATEDIR,              /* Create directory system call  */
    SYS_SYNC,                   /* Sync filesystems system call  */
    SYS_FORMAT,
    SYS_SHUTDOWN,               /*  misguided / unused */
    SYS_READBLOCK,              /* Read a block from disk */
    SYS_WRITEBLOCK,             /* Write a block to disk */
    SYS_ETHPACKETSEND,          /* Send an ethernet packet */
    SYS_ETHPACKETRECEIVE,       /* Receiven an ethernet packet */
    SYS_ARP,                    /* Send an arp request for IP */
    SYS_ROUTEADD,               /* Add a route to the routing table */
    SYS_ROUTEDEL,               /* Delete a route from the routing table */
    SYS_ROUTEGET,               /* Get the routing table */
    SYS_IPCONFIGURE,            /* Manage IP address assignment to network devices */
    SYS_IPGET,                  /* Get the IP device mappings */
    SYS_IPSEND,                 /* Send an IP packet */
    SYS_SOCKET,                 /* Create a socket */
    SYS_BIND,                   /* Bind a socket */
    SYS_LISTEN,                 /* Listen for an incoming connection */
    SYS_ACCEPT,                 /* Accept an incoming connection */
    SYS_CONNECT,                /* Create a connection to a socket */
    SYS_SEND,                   /* Send data to a remote socket */
    SYS_RECEIVE,                /* Receive data from a socket */
    SYS_SENDTO,                 /* Send data to a remote socket at the address */
    SYS_RECEIVEFROM,            /* Get the data's source */
    SYS_CLOSESOCKET,            /* Close a socket connection */
    SYS_LIMIT,                  /* Limit resources system call  */
    SYS_GET_UID,                /* Get uid current of process */
    SYS_SET_SET_UID,            /* set the setuid bit on the passwed file to true=1, or off false=0 */
    SYS_SET_EFFECTIVE_UID,      /* set the effective uid of the current process */
    SYS_SET_ACL,                /* set the acl for the passed file for the passed user to the passwed acl */
    SYS_PLAY_SOUND_FILE,        /* Play a named sound file. */
    SYS_PIPE,                   /* Create a pipe */
    SYS_FORK,                   /* Fork this process */
    SYS_EXECL,                  /* Replace this process with a new one. */
    SYS_DIAGNOSTIC,             /* Print diagnostic information to the console. */
    SYS_DISKPROPERTIES,         /* Query disk properties. */
    SYS_SET_AFFINITY,           /* set scheduler affinity */
    SYS_GET_AFFINITY,           /* get scheduler affinity */
    SYS_CLONE,                  /* LWP version of fork */
    SYS_MMAP,                   /* mmap a file into a process address space */
    SYS_MUNMAP,                 /* unmap a file from a process address space */
    SYS_ALARM,                  /* set an alarm to happen seconds in the future */
    SYS_RENAME,                 /* Rename a file system call  */
    SYS_LINK,                   /* hard link two files */
    SYS_SYMLINK,                /* Symbolic link two files */
    SYS_SBRK,                   /* sbrk */
};

/*
 * Macros for convenient generation of user space
 * system call wrapper functions.
 *
 * See "src/libc/conio.c" and "src/libc/process.c" for examples of how
 * to use these macros.
 *
 * Register conventions:
 * eax - system call number [input], return value [output]
 * ebx - first argument [input]
 * ecx - second argument [input]
 * edx - third argument [input]
 * esi - fourth argument [input]
 * edi - fifth argument [input]
 */

#define SYSCALL_REGS_0
#define SYSCALL_REGS_1 , "b" (arg0)
#define SYSCALL_REGS_2 , "b" (arg0), "c" (arg1)
#define SYSCALL_REGS_3 , "b" (arg0), "c" (arg1), "d" (arg2)
#define SYSCALL_REGS_4 , "b" (arg0), "c" (arg1), "d" (arg2), "S" (arg3)
#define SYSCALL_REGS_5 , "b" (arg0), "c" (arg1), "d" (arg2), "S" (arg3), "D" (arg4)

#define DEF_SYSCALL(name,num,retType,params,argDefs,regs)		\
retType name params {							\
    int sysNum = (num), rc;						\
    argDefs								\
    __asm__ __volatile__ (SYSCALL : "=a" (rc) :"a" (sysNum) regs);	\
    return (retType) rc;						\
}
#define DEF_SYSCALL_NORETURN(name,num,retType,params,argDefs,regs)		\
retType name params {							\
    int sysNum = (num), rc;						\
    argDefs								\
    __asm__ __volatile__ (SYSCALL : "=a" (rc) :"a" (sysNum) regs);	\
    __builtin_unreachable(); \
}

#endif /* GEEKOS_SYSCALL_H */
