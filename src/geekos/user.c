/*
 * Common user mode functions
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
 * $Revision: 1.53 $
 * 
 */

#include <geekos/errno.h>
#include <geekos/ktypes.h>
#include <geekos/kassert.h>
#include <geekos/int.h>
#include <geekos/mem.h>
#include <geekos/malloc.h>
#include <geekos/kthread.h>
#include <geekos/vfs.h>
#include <geekos/tss.h>
#include <geekos/user.h>
#include <geekos/projects.h>
#include <geekos/smp.h>
#include <libc/string.h>
#include <geekos/segment.h>
#include <geekos/gdt.h>

/*
 * This module contains common functions for implementation of user
 * mode processes.
 */

extern Spin_Lock_t kthreadLock;

/*
 * Associate the given user context with a kernel thread.
 * This makes the thread a user process.
 */
void Attach_User_Context(struct Kernel_Thread *kthread,
                         struct User_Context *context) {
    KASSERT(context != 0);
    kthread->userContext = context;

    KASSERT(context->refCount >= 0);

    int iflag = Begin_Int_Atomic();     /* ns14; deadlock if kthreadlock held when preempted */
    Spin_Lock(&kthreadLock);
    ++context->refCount;
    Spin_Unlock(&kthreadLock);
    End_Int_Atomic(iflag);
}

/*
 * If the given thread has a user context, detach it
 * and destroy it.  This is called when a thread is
 * being destroyed.
 *	Called with kthreadLock held
 */
void Detach_User_Context(struct Kernel_Thread *kthread) {
    struct User_Context *old = kthread->userContext;

    kthread->userContext = NULL;

    if(old != 0) {
        --old->refCount;
        if(old->refCount == 0) {
            Destroy_User_Context(old);
        }
        KASSERT(old->refCount >= 0);
    }
}


/*
 * Spawn a user process.
 * Params:
 *   program - the full path of the program executable file
 *   command - the command, including name of program and arguments
 *   pThread - reference to Kernel_Thread pointer where a pointer to
 *     the newly created user mode thread (process) should be
 *     stored
 * Returns:
 *   Zero or an error code
 *   if the process couldn't be created.  Note that this function
 *   should return ENOTFOUND if the reason for failure is that
 *   the executable file doesn't exist.
 */
int Spawn(const char *program, const char *command,
          struct Kernel_Thread **pThread, bool background) {
    int rc = 0, rc2 = 0, rc3 = 0;
    char *exeFileData = 0;
    ulong_t exeFileLength;
    struct User_Context *userContext = 0;
    struct Kernel_Thread *process = 0;
    struct Exe_Format exeFormat;

    /*
     * Load the executable file data, parse ELF headers,
     * and load code and data segments into user memory.
     */

    rc = Read_Fully(program, (void **)&exeFileData, &exeFileLength);
    rc2 = Parse_ELF_Executable(exeFileData, exeFileLength, &exeFormat);
    rc3 = Load_User_Program(exeFileData, exeFileLength, &exeFormat, 
			command, &userContext);
   if(rc|rc2|rc3) {
        rc |= rc2 | rc3;        
     /* since all are zero, this sets rc to whichever one is nonzero.
        breakpoint here if you need to know which one failed. */
        goto fail;
    }

    /*
     * User program has been loaded, so we can free the
     * executable file data now.
     */
    Free(exeFileData);
    exeFileData = 0;

    strncpy(userContext->name, program, MAX_PROC_NAME_SZB);
    userContext->name[MAX_PROC_NAME_SZB - 1] = '\0';


    /* Start the process! */
    process = Start_User_Thread(userContext, background);
    if(process != 0) {
        /* Return Kernel_Thread pointer */
        *pThread = process;
    } else
        rc = ENOMEM;

    return rc;

  fail:
    if(exeFileData != 0)
        Free(exeFileData);
    if(userContext != 0)
        Destroy_User_Context(userContext);

    return rc;
}

/* 
 * Called from main -- don't alter main for this.
*/
int Spawn_Foreground(const char *program, const char *command,
                     struct Kernel_Thread **pThread) {
    return Spawn(program, command, pThread, false);
}

extern int Spawn_Program(char *exeFileData, struct Exe_Format *exeFormat);
extern void Hardware_Shutdown();

void Spawner() {
  const char *program = "/c/a.exe";
  char *exeFileData = 0;
  ulong_t exeFileLength;
  struct Exe_Format exeFormat;

  if (Read_Fully(program, (void**) &exeFileData, &exeFileLength) != 0)
    {
      Print("Read_Fully failed to read %s from disk\n", program);
      goto fail;
    }

  if (Parse_ELF_Executable(exeFileData, exeFileLength, &exeFormat) != 0)
    {
      Print("Parse_ELF_Executable failed\n");
      goto fail;
    }
  if (Spawn_Program(exeFileData, &exeFormat) != 0)
    {
      Print("Spawn_Program failed\n");
      goto fail;
    }

   for(;;);
fail:
  Hardware_Shutdown();
}

/*
 * If the given thread has a User_Context,
 * switch to its memory space.
 *
 * Params:
 *   kthread - the thread that is about to execute
 *   state - saved processor registers describing the state when
 *      the thread was interrupted
 */
void Switch_To_User_Context(struct Kernel_Thread *kthread,
                            struct Interrupt_State *state
                            __attribute__ ((unused))) {
    int cpuID;
    extern int userDebug;
    struct User_Context *userContext = kthread->userContext;

    /*
     * FIXME: could avoid resetting ss0/esp0 if not returning
     * to user space.
     */

    cpuID = Get_CPU_ID();

    KASSERT(!Interrupts_Enabled());

    if(CPUs[cpuID].s_currentUserContext && userContext == 0) {
        /* Kernel mode thread: muse switch kernel address space. 
           another core could delete a user context while this thread is 
           using it otherwise. */
        Set_PDBR((void *)Kernel_Page_Dir());
        CPUs[cpuID].s_currentUserContext = NULL;
        return;
    }

    /* Switch only if the user context is indeed different */
    if(userContext != CPUs[cpuID].s_currentUserContext) {

        if(userDebug)
            Print("A[%p]\n", kthread);

        /* Switch to address space of user context */
        Switch_To_Address_Space(userContext);

        /* New user context is active */
        CPUs[cpuID].s_currentUserContext = userContext;
    }

    /* Must do the rest of this regardless if the current context is changing since kernel stacks are per thread
     * context not per userContext - and user contexts are shared in Clone'd threads */

    /*
     * By definition, when returning to user mode there is no
     * context remaining on the kernel stack.
     */
    ulong_t esp0;

    esp0 = ((ulong_t) kthread->stackPage) + PAGE_SIZE;
    if(userDebug)
        Print("S[%lx]\n", esp0);

    /* Change to the kernel stack of the new process. */
    Set_Kernel_Stack_Pointer(esp0);
}
