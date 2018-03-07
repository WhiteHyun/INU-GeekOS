/*
 * User mode context
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

#ifndef GEEKOS_USER_H
#define GEEKOS_USER_H

/* Information communicated by the PS system call */

#define MAX_PROC_NAME_SZB 128

struct Process_Info {
    char name[MAX_PROC_NAME_SZB];
    int pid;
    int parent_pid;             /* 0 if no parent */
    int priority;
#define STATUS_RUNNABLE 0
#define STATUS_BLOCKED  1
#define STATUS_ZOMBIE   2
    int status;
    int affinity;
    int currCore;
    int totalTime;
};

#ifdef GEEKOS

#include <geekos/ktypes.h>
#include <geekos/segment.h>
#include <geekos/elf.h>
#include <geekos/signal.h>
#include <geekos/paging.h>

struct File;

/* Number of files user process can have open. */
#define USER_MAX_FILES		10

/*
 * A user mode context which can be attached to a Kernel_Thread,
 * to allow it to execute in user mode (ring 3).  This struct
 * has all information needed to create and manage a user
 * memory space, as well as other kernel resources used by
 * the process (such as semaphores and files).
 */
struct User_Context {
    /* We need one LDT entry each for user code and data segments. */
#define NUM_USER_LDT_ENTRIES 2

    /*
     * Each user context contains a local descriptor table with
     * just enough room for one code and one data segment
     * describing the process's memory.
     */
    struct Segment_Descriptor ldt[NUM_USER_LDT_ENTRIES];
    struct Segment_Descriptor *ldtDescriptor;

    /* The memory space used by the process. */
    char *memory;
    ulong_t size;

    /* Selector for the LDT's descriptor in the GDT */
    ushort_t ldtSelector;

    /*
     * Selectors for the user context's code and data segments
     * (which reside in its LDT)
     */
    ushort_t csSelector;
    ushort_t dsSelector;

    /* Page directory for user address space. */
    pde_t *pageDir;

    /*! Open files. */
    struct File *file_descriptor_table[USER_MAX_FILES];

    /* Code entry point */
    ulong_t entryAddr;

    /* Address of argument block in user memory */
    ulong_t argBlockAddr;

    /* Initial stack pointer */
    ulong_t stackPointerAddr;

    /*
     * May use this in future to allow multiple threads
     * in the same user context
     */
    int refCount;

    char name[MAX_PROC_NAME_SZB];




    mappedRegion_t *mappedRegions;
};


struct Kernel_Thread;
struct Interrupt_State;

/*
 * Common routines: these are in user.c
 */

void Attach_User_Context(struct Kernel_Thread *kthread,
                         struct User_Context *context);
void Detach_User_Context(struct Kernel_Thread *kthread);
int Spawn(const char *program, const char *command,
          struct Kernel_Thread **pThread, bool background);
int Spawn_Foreground(const char *program, const char *command,
                     struct Kernel_Thread **pThread);
void Switch_To_User_Context(struct Kernel_Thread *kthread,
                            struct Interrupt_State *state);

/*
 * Implementation routines: these are in userseg.c or uservm.c
 */

void Destroy_User_Context(struct User_Context *context);
int Load_User_Program(char *exeFileData, ulong_t exeFileLength,
                      struct Exe_Format *exeFormat, const char *command,
                      struct User_Context **pUserContext);
bool Copy_From_User(void *destInKernel, ulong_t srcInUser,
                    ulong_t bufSize);
bool Copy_To_User(ulong_t destInUser, void *srcInKernel, ulong_t bufSize);
#define VUM_WRITING 1
#define VUM_READING 0
bool Validate_User_Memory(struct User_Context *userContext,
                          ulong_t userAddr, ulong_t bufSize,
                          int for_writing);
void *User_To_Kernel(struct User_Context *userContext, ulong_t userPtr);

void Switch_To_Address_Space(struct User_Context *userContext);


#endif /* GEEKOS */


#endif /* GEEKOS_USER_H */
