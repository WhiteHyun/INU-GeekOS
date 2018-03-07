/*
 * System call handlers
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
 */

#include <geekos/syscall.h>
#include <geekos/errno.h>
#include <geekos/kthread.h>
#include <geekos/int.h>
#include <geekos/elf.h>
#include <geekos/malloc.h>
#include <geekos/screen.h>
#include <geekos/keyboard.h>
#include <geekos/string.h>
#include <geekos/user.h>
#include <geekos/timer.h>
#include <geekos/vfs.h>
#include <geekos/signal.h>
#include <geekos/sem.h>
#include <geekos/projects.h>

#include <geekos/sys_net.h>
#include <geekos/pipe.h>
#include <geekos/mem.h>
#include <geekos/smp.h>

extern Spin_Lock_t kthreadLock;

/*
 * Allocate a buffer for a user string, and
 * copy it into kernel space.
 * Interrupts must be disabled.
 */
/* "extern" to note that it's used by semaphore and networking system calls, defined 
   in another file */
/* need not be called with interrupts disabled, since it should move data from 
   user space (which is blocked in the kernel) into a newly-allocated buffer */
extern int Copy_User_String(ulong_t uaddr, ulong_t len, ulong_t maxLen,
                            char **pStr) {
    int rc = 0;
    char *str;


    /* Ensure that string isn't too long. */
    if(len > maxLen)
        return EINVALID;

    /* Allocate space for the string. */
    str = (char *)Malloc(len + 1);
    if(str == 0) {
        rc = ENOMEM;
        goto done;
    }

    /* Copy data from user space. */
    if(!Copy_From_User(str, uaddr, len)) {
        rc = EINVALID;
        Free(str);
        goto done;
    }
    str[len] = '\0';

    /* Success! */
    *pStr = str;

  done:
    return rc;
}

/*
 * Null system call.
 * Does nothing except immediately return control back
 * to the interrupted user program.
 * Params:
 *  state - processor registers from user mode
 *
 * Returns:
 *   always returns the value 0 (zero)
 */
static int Sys_Null(struct Interrupt_State *state
                    __attribute__ ((unused))) {
    return 0;
}

/*
 * Exit system call.
 * The interrupted user process is terminated.
 * Params:
 *   state->ebx - process exit code
 * Returns:
 *   Never returns to user mode!
 */
static int Sys_Exit(struct Interrupt_State *state) {
    Enable_Interrupts();        /* ns14 */
    Exit(state->ebx);
    /* We will never get here. */
}

/*
** Shutdown Computer
** Normaly not within a user's powers,
** but it helps automate testing
*/
extern void Hardware_Shutdown();        /* is in keyboard.c for odd reasons */
static int Sys_ShutDown(struct Interrupt_State *state) {
    Print("------------------- THE END ------------------\n");
    Hardware_Shutdown();
    /* We will never get here. */
    return 0;
}

static Spin_Lock_t sprintLock;


/*
 * Print a string to the console.
 * Params:
 *   state->ebx - user pointer of string to be printed
 *   state->ecx - number of characters to print
 * Returns: 0 if successful, -1 if not
 */
static int Sys_PrintString(struct Interrupt_State *state) {
    int rc = 0;
    uint_t length = state->ecx;
    char *buf = 0;

    //    unlockKernel();
    Enable_Interrupts();

    if(length > 0) {
        /* Copy string into kernel. */
        if((rc =
            Copy_User_String(state->ebx, length, 1023,
                             (char **)&buf)) != 0)
            goto done;

        /* in reality, one wouldn't abort on this sort of thing.  but we do that
           just in case it helps track down a bug that much sooner */
        /* length is greater than zero, so someone thought there was a string here,
           but the first character is null, so they were either wrong or something 
           poor occurred. */
        if(!buf[0]) {
            Dump_Interrupt_State(state);
            KASSERT0(buf[0],
                     "Attempted to print a null string; this is likely a memory error.");
        }

        TODO_P(PROJECT_SERIAL,
               "Print to the serial console if appropriate");

        /* Write to console; only one may write. */
        Spin_Lock(&sprintLock);
        Put_Buf(buf, length);
        Spin_Unlock(&sprintLock);

    }

  done:
    if(buf != 0)
        Free(buf);

    /* somehow, ends up being locked here */
    // lockKernel();
    Disable_Interrupts();

    return rc;
}

/*
 * Get a single key press from the console.
 * Suspends the user process until a key press is available.
 * Params:
 *   state - processor registers from user mode
 * Returns: the key code
 *          -1 if this is a background process
 */
static int Sys_GetKey(struct Interrupt_State *state) {
    TODO_P(PROJECT_SERIAL,
           "Get a key from the serial console if appropriate");

    return Wait_For_Key();
}

/*
 * Set the current text attributes.
 * Params:
 *   state->ebx - character attributes to use
 * Returns: always returns 0
 */
static int Sys_SetAttr(struct Interrupt_State *state) {
    Set_Current_Attr((uchar_t) state->ebx);
    return 0;
}

/*
 * Get the current cursor position.
 * Params:
 *   state->ebx - pointer to user int where row value should be stored
 *   state->ecx - pointer to user int where column value should be stored
 * Returns: 0 if successful, -1 otherwise
 */
static int Sys_GetCursor(struct Interrupt_State *state) {
    int row, col, ret;
    TODO_P(PROJECT_SERIAL, "fail if invoked when serial port is in use.");
    Get_Cursor(&row, &col);
    if(!Copy_To_User(state->ebx, &row, sizeof(int)) ||
       !Copy_To_User(state->ecx, &col, sizeof(int))) {
        ret = -1;
    } else {
        ret = 0;
    }
    return ret;
}

/*
 * Set the current cursor position.
 * Params:
 *   state->ebx - new row value
 *   state->ecx - new column value
 * Returns: 0 if successful, -1 otherwise
 */
static int Sys_PutCursor(struct Interrupt_State *state) {
    TODO_P(PROJECT_SERIAL, "fail if serial.");
    return Put_Cursor(state->ebx, state->ecx) ? 0 : -1;
}

/*
 * Create a new user process.
 * Params:
 *   state->ebx - user address of name of executable
 *   state->ecx - length of executable name
 *   state->edx - user address of command string
 *   state->esi - length of command string
 *   state->edi == whether to spawn "detached" without a parent and without access to keys
 * Returns: pid of process if successful, error code (< 0) otherwise
 */
static int Sys_Spawn(struct Interrupt_State *state) {
    int rc;
    char *program = 0;
    char *command = 0;
    struct Kernel_Thread *process = NULL;

    Enable_Interrupts();

    /* Copy program name and command from user space. */
    if((rc =
        Copy_User_String(state->ebx, state->ecx, VFS_MAX_PATH_LEN,
                         &program)) != 0 ||
       (rc =
        Copy_User_String(state->edx, state->esi, 1023, &command)) != 0)
        goto done;

    /*
     * Now that we have collected the program name and command string
     * from user space, we can try to actually spawn the process.
     */
    rc = Spawn(program, command, &process, state->edi);

    if(rc == 0) {
        KASSERT(process != 0);
        rc = process->pid;
    }

  done:
    if(program != 0)
        Free(program);
    if(command != 0)
        Free(command);

    Disable_Interrupts();

    return rc;
}

/*
 * Wait for a process to exit.
 * Params:
 *   state->ebx - pid of process to wait for
 * Returns: the exit code of the process,
 *   or error code (< 0) on error
 */
static int Sys_Wait(struct Interrupt_State *state) {
    int exitCode;
    struct Kernel_Thread *kthread;

    Enable_Interrupts();
    kthread = Lookup_Thread(state->ebx, 0);
    if(kthread == 0) {
        // can't find the process id passed
        exitCode = EINVALID;
        goto finish;
    }

    if(kthread->detached) {
        // can't wait on a detached process
        exitCode = EINVALID;
        goto finish;
    }
    exitCode = Join(kthread);
  finish:
    Disable_Interrupts();
    return exitCode;
}

/*
 * Get pid (process id) of current thread.
 * Params:
 *   state - processor registers from user mode
 * Returns: the pid of the current thread
 */
static int Sys_GetPID(struct Interrupt_State *state) {
    return CURRENT_THREAD->pid;
}


extern struct All_Thread_List s_allThreadList;
extern struct Thread_Queue s_runQueue;


/*
 * Get information about the running processes
 * Params:
 *   state->ebx - pointer to user memory containing an array of
 *   Process_Info structs
 *   state->ecx - length of the passed in array in memory
 * Returns: -1 on failure
 *          0 if size of user memory too small
 *          N the number of entries in the table, on success
 */
static int Sys_PS(struct Interrupt_State *state) {
    TODO_P(PROJECT_BACKGROUND_JOBS, "Sys_PS system call");
    return 0;
}


/*
 * Send a signal to a process
 * Params:
 *   state->ebx - pid of process to send signal to
 *   state->ecx - signal number
 * Returns: 0 on success or error code (< 0) on error
 */
static int Sys_Kill(struct Interrupt_State *state) {
    TODO_P(PROJECT_SIGNALS, "Sys_Kill system call");
    return 0;
}

/*
 * Register a signal handler for a process
 * Params:
 *   state->ebx - pointer to handler function
 *   state->ecx - signal number
 * Returns: 0 on success or error code (< 0) on error
 */
static int Sys_Signal(struct Interrupt_State *state) {
    TODO_P(PROJECT_SIGNALS, "Sys_Signal system call");
    return 0;
}

/*
 * Register the Return_Signal trampoline for this process.
 * Signals cannot be delivered until this is registered.
 * Params:
 *   state->ebx - pointer to Return_Signal function
 * may not be used:
 *   state->ecx - pointer to the default handler
 *   state->edx - pointer to the ignore handler
 *
 * Returns: 0 on success or error code (< 0) on error
 */
static int Sys_RegDeliver(struct Interrupt_State *state) {
    return 0;
    TODO_P(PROJECT_SIGNALS, "Sys_RegDeliver system call");
}

/*
 * Complete signal handling for this process.
 * Params:
 *   none
 *
 * Returns: not expected to "return"
 */
static int Sys_ReturnSignal(struct Interrupt_State *state) {
    TODO_P(PROJECT_SIGNALS, "Sys_ReturnSignal system call");
    return EUNSUPPORTED;
}

/*
 * Reap a child process that has died
 * Params:
 *   state->ebx - pointer to status of process reaped
 * Returns: pid of reaped process on success, -1 on error.
 */
static int Sys_WaitNoPID(struct Interrupt_State *state) {
    /* not required for Spring 2016 */
    TODO_P(PROJECT_SIGNALS, "Sys_WaitNoPID system call");
    return EUNSUPPORTED;
}

/*
 * Set the scheduling policy.
 * Params:
 *   state->ebx - policy,
 *   state->ecx - number of ticks in quantum
 * Returns: 0 if successful, -1 otherwise
 */
static int Sys_SetSchedulingPolicy(struct Interrupt_State *state) {
    TODO_P(PROJECT_SCHEDULING, "SetSchedulingPolicy system call");
    return 0;
}

/*
 * Get the time of day.
 * Params:
 *   state - processor registers from user mode
 *
 * Returns: value of the g_numTicks global variable
 */
static int Sys_GetTimeOfDay(struct Interrupt_State *state
                            __attribute__ ((unused))) {
    return g_numTicks;
}

/*
 * Mount a filesystem.
 * Params:
 * state->ebx - contains a pointer to the Mount_Syscall_Args structure
 *   which contains the block device name, mount prefix,
 *   and filesystem type
 *
 * Returns:
 *   0 if successful, error code if unsuccessful
 */
static int Sys_Mount(struct Interrupt_State *state) {
    int rc = 0;
    struct VFS_Mount_Request *args = 0;


    /* Allocate space for VFS_Mount_Request struct. */
    if((args =
        (struct VFS_Mount_Request *)
        Malloc(sizeof(struct VFS_Mount_Request))) == 0) {
        rc = ENOMEM;
        goto done;
    }

    /* Copy the mount arguments structure from user space. */
    if(!Copy_From_User
       (args, state->ebx, sizeof(struct VFS_Mount_Request))) {
        rc = EINVALID;
        goto done;
    }

    /*
     * Hint: use devname, prefix, and fstype from the args structure
     * and invoke the Mount() VFS function.  You will need to check
     * to make sure they are correctly nul-terminated.
     */
    TODO_P(PROJECT_FS, "Mount system call");
    rc = EUNSUPPORTED;
  done:
    if(args != 0)
        Free(args);
    return rc;
}

static int get_path_from_registers(uint_t addr, uint_t length,
                                   char **pPath) {
    if(length > 1024) {
        return ENAMETOOLONG;
    }
    *pPath = Malloc(length + 1);
    if(!*pPath) {
        return ENOMEM;
    }
    if(!Copy_From_User(*pPath, addr, length)) {
        Free(*pPath);
        return EINVALID;
    }
    (*pPath)[length] = '\0';
    return 0;
}

static int next_descriptor() {
    int descriptor;
    for(descriptor = 0;
        descriptor < USER_MAX_FILES &&
        CURRENT_THREAD->userContext->file_descriptor_table[descriptor] !=
        0; descriptor++) ;
    if(descriptor == USER_MAX_FILES) {
        return EMFILE;
    }
    return descriptor;
}

static int add_file_to_descriptor_table(struct File *file) {
    int descriptor = next_descriptor();
    if(descriptor >= 0) {
        CURRENT_THREAD->userContext->file_descriptor_table[descriptor] =
            file;
    }
    return descriptor;
}

/*
 * Open a file.
 * Params:
 *   state->ebx - address of user string containing path of file to open
 *   state->ecx - length of path
 *   state->edx - file mode flags
 *
 * Returns: a file descriptor (>= 0) if successful,
 *   or an error code (< 0) if unsuccessful
 */
static int Sys_Open(struct Interrupt_State *state) {
    char *path;
    struct File *file;
    int rc = 0;

    Enable_Interrupts();

    rc = get_path_from_registers(state->ebx, state->ecx, &path);
    if(rc != 0) {
        goto leave;
    }

    rc = next_descriptor();
    if(rc < 0) {
        Free(path);
        goto leave;
    }

    rc = Open(path, state->edx, &file);
    Free(path);

  leave:
    Disable_Interrupts();
    if(rc >= 0) {
        return add_file_to_descriptor_table(file);
    } else {
        return rc;
    }
}

/*
 * Open a directory.
 * Params:
 *   state->ebx - address of user string containing path of directory to open
 *   state->ecx - length of path
 *
 * Returns: a file descriptor (>= 0) if successful,
 *   or an error code (< 0) if unsuccessful
 */
static int Sys_OpenDirectory(struct Interrupt_State *state) {
    TODO_P(PROJECT_FS, "Open directory system call");
    return EUNSUPPORTED;
}

/*
 * Close an open file or directory.
 * Params:
 *   state->ebx - file descriptor of the open file or directory
 * Returns: 0 if successful, or an error code (< 0) if unsuccessful
 */
static int Sys_Close(struct Interrupt_State *state) {
    /* where is the file table? */
    if(state->ebx > USER_MAX_FILES) {
        Print("unable to close fd index %d, out of range.\n", state->ebx);
        return EINVALID;
    }
    if(CURRENT_THREAD->userContext->file_descriptor_table[state->ebx]) {
        Enable_Interrupts();
        Close(CURRENT_THREAD->userContext->
              file_descriptor_table[state->ebx]);
        Disable_Interrupts();
        CURRENT_THREAD->userContext->file_descriptor_table[state->ebx] =
            0;
        return 0;
    } else {
        // Print("unable to close fd index %d, nothing there.\n", state->ebx);
        return ENOTFOUND;
    }
}

/*
 * Delete a file.
 * Params:
 *   state->ebx - address of user string containing path to delete
 *   state->ecx - length of path
 *   state->edx - recursive flags
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_Delete(struct Interrupt_State *state) {
    TODO_P(PROJECT_FS, "Delete system call");
    return EUNSUPPORTED;
}

/*
 * Rename a file.
 * Params:
 *   state->ebx - address of user string containing old path 
 *   state->ecx - length of old path
 *   state->edx - address of user string containing new path 
 *   state->esix - length of new path
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_Rename(struct Interrupt_State *state) {
    TODO_P(PROJECT_FS, "Rename system call");
    return EUNSUPPORTED;
}

/*
 * Link a file.
 * Params:
 *   state->ebx - address of user string containing old path 
 *   state->ecx - length of old path
 *   state->edx - address of user string containing new path 
 *   state->esix - length of new path
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_Link(struct Interrupt_State *state) {
    TODO_P(PROJECT_FS, "Link system call");
    return EUNSUPPORTED;
}

/*
 * Link a file.
 * Params:
 *   state->ebx - address of user string containing old path 
 *   state->ecx - length of old path
 *   state->edx - address of user string containing new path 
 *   state->esix - length of new path
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_SymLink(struct Interrupt_State *state) {
    TODO_P(PROJECT_FS, "Link system call");
    return EUNSUPPORTED;
}

/*
 * Read from an open file.
 * Params:
 *   state->ebx - file descriptor to read from
 *   state->ecx - user address of buffer to read into
 *   state->edx - number of bytes to read
 *
 * Returns: number of bytes read, 0 if end of file,
 *   or error code (< 0) on error
 */
static int Sys_Read(struct Interrupt_State *state) {
    int bytes_read = 0;
    /* where is the file table? */
    if(state->ebx > USER_MAX_FILES) {
        return EINVALID;
    }
    if(CURRENT_THREAD->userContext->file_descriptor_table[state->ebx]) {
        void *data_buffer;
        Enable_Interrupts();
        data_buffer = Malloc(state->edx);
        if(!data_buffer) {
            return ENOMEM;
        }
        bytes_read =
            Read(CURRENT_THREAD->userContext->
                 file_descriptor_table[state->ebx], data_buffer,
                 state->edx);
        if(bytes_read > 0) {
            if(!Copy_To_User(state->ecx, data_buffer, bytes_read)) {
                bytes_read = EINVALID;
            }
        }
        Free(data_buffer);
        Disable_Interrupts();
        return bytes_read;
    } else {
        return ENOTFOUND;
    }
}

/*
 * Read a directory entry from an open directory handle.
 * Params:
 *   state->ebx - file descriptor of the directory
 *   state->ecx - user address of struct VFS_Dir_Entry to copy entry into
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_ReadEntry(struct Interrupt_State *state) {
    TODO_P(PROJECT_FS, "ReadEntry system call");
    return EUNSUPPORTED;
}

/*
 * Write to an open file.
 * Params:
 *   state->ebx - file descriptor to write to
 *   state->ecx - user address of buffer get data to write from
 *   state->edx - number of bytes to write
 *
 * Returns: number of bytes written,
 *   or error code (< 0) on error
 */
static int Sys_Write(struct Interrupt_State *state) {
    int bytes_written = 0;
    /* where is the file table? */
    if(state->ebx > USER_MAX_FILES) {
        return EINVALID;
    }
    if(CURRENT_THREAD->userContext->file_descriptor_table[state->ebx]) {
        Enable_Interrupts();
        void *data_buffer = Malloc(state->edx);
        if(!data_buffer) {
            Disable_Interrupts();
            return ENOMEM;
        }
        if(!Copy_From_User(data_buffer, state->ecx, state->edx)) {
            Free(data_buffer);
            Disable_Interrupts();
            return EINVALID;
        }
        bytes_written =
            Write(CURRENT_THREAD->userContext->
                  file_descriptor_table[state->ebx], data_buffer,
                  state->edx);
        Free(data_buffer);
        Disable_Interrupts();
        return bytes_written;
    } else {
        return ENOTFOUND;
    }
}

/*
 * Get file metadata.
 * Params:
 *   state->ebx - address of user string containing path of file
 *   state->ecx - length of path
 *   state->edx - user address of struct VFS_File_Stat object to store metadata in
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_Stat(struct Interrupt_State *state) {
    TODO_P(PROJECT_FS, "Stat system call");
    return EUNSUPPORTED;
}

/*
 * Get metadata of an open file.
 * Params:
 *   state->ebx - file descriptor to get metadata for
 *   state->ecx - user address of struct VFS_File_Stat object to store metadata in
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_FStat(struct Interrupt_State *state) {
    TODO_P(PROJECT_FS, "FStat system call");
    return EUNSUPPORTED;
}

/*
 * Change the access position in a file
 * Params:
 *   state->ebx - file descriptor
 *   state->ecx - position in file
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_Seek(struct Interrupt_State *state) {
    TODO_P(PROJECT_FS, "Seek system call");
    return EUNSUPPORTED;
}

/*
 * Create directory
 * Params:
 *   state->ebx - address of user string containing path of new directory
 *   state->ecx - length of path
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_CreateDir(struct Interrupt_State *state) {
    TODO_P(PROJECT_FS, "CreateDir system call");
    return EUNSUPPORTED;
}

/*
 * Flush filesystem buffers
 * Params: none
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_Sync(struct Interrupt_State *state) {
    TODO_P(PROJECT_FS, "Sync system call");
    return EUNSUPPORTED;
}

/*
 * Format a device
 * Params:
 *   state->ebx - address of user string containing device to format
 *   state->ecx - length of device name string
 *   state->edx - address of user string containing filesystem type 
 *   state->esi - length of filesystem type string

 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_Format(struct Interrupt_State *state) {
    int rc = 0;
    char *devname = 0, *fstype = 0;

    Enable_Interrupts();

    if((rc =
        Copy_User_String(state->ebx, state->ecx, BLOCKDEV_MAX_NAME_LEN,
                         &devname)) != 0 ||
       (rc =
        Copy_User_String(state->edx, state->esi, VFS_MAX_FS_NAME_LEN,
                         &fstype)) != 0)
        goto done;

    rc = Format(devname, fstype);

  done:
    if(devname != 0)
        Free(devname);
    if(fstype != 0)
        Free(fstype);
    Disable_Interrupts();
    return rc;
}

/*
 * Read a block from a device
 * Params:
 *   state->ebx - address of user string containing block device name
 *   state->ecx - length of block device name string
 *   state->edx - address of user buffer to read into
 *   state->esi - length to read into user buffer
 *   state->edi - block # to read from

 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_ReadBlock(struct Interrupt_State *state) {
    TODO_P(PROJECT_FS, "ReadBlock system call");
    return EUNSUPPORTED;
}

/*
 * Write a block to a device
 * Params:
 *   state->ebx - address of user string containing device name
 *   state->ecx - length of block device name string
 *   state->edx - address of user buffer to write
 *   state->esi - length to write to block
 *   state->edi - block # to write to

 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
static int Sys_WriteBlock(struct Interrupt_State *state) {
    TODO_P(PROJECT_FS, "WriteBlock system call");
    return EUNSUPPORTED;
}


static int Sys_GetUid(struct Interrupt_State *state) {
    TODO_P(PROJECT_USER, "Sys_GetUid system call");
    return EUNSUPPORTED;
}

static int Sys_SetSetUid(struct Interrupt_State *state) {
    TODO_P(PROJECT_USER, "Sys_SetSetUid system call");
    return EUNSUPPORTED;
}

static int Sys_SetEffectiveUid(struct Interrupt_State *state) {
    TODO_P(PROJECT_USER, "Sys_SetEffectiveUid system call");
    return EUNSUPPORTED;
}

static int Sys_SetAcl(struct Interrupt_State *state) {
    TODO_P(PROJECT_USER, "Sys_SetAcl system call");
    return EUNSUPPORTED;
}

extern void SB16_Play_File(const char *filename);
static int Sys_PlaySoundFile(struct Interrupt_State *state) {
    TODO_P(PROJECT_SOUND, "PlaySoundFile system call");
    return 0;
}

/* 
 * Create a pipe.
 * Params:
 *   state->ebx - address of file descriptor for the read side
 *   state->ecx - address of file descriptor for the write side
 */
static int Sys_Pipe(struct Interrupt_State *state) {
    TODO_P(PROJECT_PIPE, "Pipe system call");
    return EUNSUPPORTED;
}



static int Sys_Fork(struct Interrupt_State *state) {
    TODO_P(PROJECT_FORK, "Fork system call");
    return EUNSUPPORTED;
}

/* 
 * Exec a new program in this process.
 * Params:
 *   state->ebx - user address of name of executable
 *   state->ecx - length of executable name
 *   state->edx - user address of command string
 *   state->esi - length of command string
 * Returns: doesn't if successful, error code (< 0) otherwise
 */
static int Sys_Execl(struct Interrupt_State *state) {
    TODO_P(PROJECT_FORK, "Execl system call");
    return EUNSUPPORTED;
}

/* 
 * The following is a crude trigger for dumping kernel 
 * statistics to the console output.  It is not generally
 * how one should output kernel statistics to user space.
 */

static int Sys_Diagnostic(struct Interrupt_State *state) {
    (void)state;                /* warning appeasement */
    Enable_Interrupts();
    Dump_Blockdev_Stats();
    Disable_Interrupts();
    return 0;
}

/* 
 * Retrieve disk properties
 * Params:
 *   state->ebx - path name to mounted file system
 *   state->ecx - length of path name
 *   state->edx - user address for block size
 *   state->esi - user address for number of blocks on disk
 * Returns: doesn't if successful, error code (< 0) otherwise
 */
static int Sys_Disk_Properties(struct Interrupt_State *state) {
    char *path;
    unsigned int block_size, blocks_per_disk;
    int rc;
    Enable_Interrupts();
    Copy_User_String(state->ebx, state->ecx, 100, &path);
    rc = Disk_Properties(path, &block_size, &blocks_per_disk);
    if(rc == 0) {
        Copy_To_User(state->edx, &block_size, sizeof(unsigned int));
        Copy_To_User(state->esi, &blocks_per_disk, sizeof(unsigned int));
    }
    Disable_Interrupts();
    return 0;
}

/* 
 * Set Resource Limits
 * Params:
 *   state->ebx - resource to limit
 *   state->ecx - limit
 * Returns: doesn't if successful, error code (< 0) otherwise
 */
static int Sys_Limit(struct Interrupt_State *state) {
    TODO_P(PROJECT_LIMIT, "Limit system call");
    return EUNSUPPORTED;
}


/* 
 * Set Processor Affinity
 * Params:
 *   state->ebx - pid
 *   state->ecx - affinity
 * Returns: 0 on success, EINVALID for errors
 */
static int Sys_Set_Affinity(struct Interrupt_State *state) {
    return EUNSUPPORTED;
}


/* 
 * Get Processor Affinity
 * Params:
 *   state->ebx - pid
 * Returns: current affinity on success, EINVALID for errors
 */
static int Sys_Get_Affinity(struct Interrupt_State *state) {
    return EUNSUPPORTED;
}

/*
 * Sys_Clone - create a new LWP, shares text and heap with parent
 *
 * Params:
 *   state->ebx - address of thread function to run 
 *   state->ecx - address of top of child's stack 
 * Returns: pid for child on sucess or EINVALID for error
 */
static int Sys_Clone(struct Interrupt_State *state) {
    TODO_P(PROJECT_CLONE, "Clone system call");
    return EUNSUPPORTED;
}

static int Sys_Mmap(struct Interrupt_State *state) {
    TODO_P(PROJECT_MMAP, "Mmap system call");
    return EUNSUPPORTED;
}

static int Sys_Munmap(struct Interrupt_State *state) {
    extern int Munmap_Impl(void *);

    TODO_P(PROJECT_MMAP, "Munmap system call");
    return EUNSUPPORTED;
}

/*
 * Sys_Alarm - create an alarm signal at a point in the future
 *
 * Params:
 *   state->ebx - time in msecs to schedule alarm
 * Returns: 0 on sucess or EINVALID for error
 */
static int Sys_Alarm(struct Interrupt_State *state) {
    TODO_P(PROJECT_SIGNALS, "Alarm");
    return EUNSUPPORTED;
}

static int Sys_Sbrk(struct Interrupt_State *state) {
    TODO_P(PROJECT_MALLOC,
           "underlying system call that allows malloc to work");
    return EUNSUPPORTED;
}

/*
 * Global table of system call handler functions.
 */
const Syscall g_syscallTable[] = {
    Sys_Null,
    Sys_Exit,
    Sys_PrintString,
    Sys_GetKey,
    Sys_SetAttr,
    Sys_GetCursor,
    Sys_PutCursor,
    Sys_Spawn,
    Sys_Wait,
    Sys_GetPID,
    Sys_Kill,
    Sys_PS,
    Sys_Signal,
    Sys_RegDeliver,
    Sys_ReturnSignal,
    Sys_WaitNoPID,
    /* Scheduling and semaphore system calls. */
    Sys_SetSchedulingPolicy,
    Sys_GetTimeOfDay,
    Sys_Open_Semaphore,
    Sys_P,
    Sys_V,
    Sys_Close_Semaphore,
    /* File I/O system calls. */
    Sys_Mount,
    Sys_Open,
    Sys_OpenDirectory,
    Sys_Close,
    Sys_Delete,
    Sys_Read,
    Sys_ReadEntry,
    Sys_Write,
    Sys_Stat,
    Sys_FStat,
    Sys_Seek,
    Sys_CreateDir,
    Sys_Sync,
    Sys_Format,
    Sys_ShutDown,
    Sys_ReadBlock,
    Sys_WriteBlock,
    /* Networking calls */
    Sys_EthPacketSend,
    Sys_EthPacketReceive,
    Sys_Arp,
    Sys_RouteAdd,
    Sys_RouteDel,
    Sys_RouteGet,
    Sys_IPConfigure,
    Sys_IPGet,
    Sys_IPSend,
    /* Socket API */
    Sys_Socket,
    Sys_Bind,
    Sys_Listen,
    Sys_Accept,
    Sys_Connect,
    Sys_Send,
    Sys_Receive,
    Sys_SendTo,
    Sys_ReceiveFrom,
    Sys_CloseSocket,
    /* User related calls */
    Sys_Limit,
    Sys_GetUid,
    Sys_SetSetUid,
    Sys_SetEffectiveUid,
    Sys_SetAcl,
    /* sound */
    Sys_PlaySoundFile,
    /* unix interface */
    Sys_Pipe,
    Sys_Fork,
    Sys_Execl,
    /* diagnostics and debugging */
    Sys_Diagnostic,
    Sys_Disk_Properties,
    /* SMP functions */
    Sys_Set_Affinity,
    Sys_Get_Affinity,
    Sys_Clone,
    /* memory mapped files */
    Sys_Mmap,
    Sys_Munmap,
    Sys_Alarm,
    Sys_Rename,
    Sys_Link,
    Sys_SymLink,
    Sys_Sbrk
};

/*
 * Number of system calls implemented.
 */
const unsigned int g_numSyscalls =
    sizeof(g_syscallTable) / sizeof(Syscall);
