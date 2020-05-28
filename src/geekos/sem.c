/*
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
#include <geekos/smp.h>

#define MAX_LENGTH_NAME 25
#define MAX_NUM_SEMAPHORE 20
struct Semaphore
{
    char name[MAX_LENGTH_NAME + 1]; //semaphore name
    bool available;
    int count; //semaphore value
    struct Thread_Queue waitQueue;
};

struct Semaphore g_Semaphores[MAX_NUM_SEMAPHORE];

static int getSemaphore()
{
    int ret = -1;
    int sid = 0;

    for (sid = 0; sid < MAX_NUM_SEMAPHORE; ++sid)
    {
        if (g_Semaphores[sid].available)
        {
            ret = sid;
            break;
        }
    }
    return ret;
}

void Init_Semaphores(void)
{
    uint_t sid = 0;
    for (sid = 0; sid < MAX_NUM_SEMAPHORE; ++sid)
    {
        g_Semaphores[sid].available = true;
        memset(g_Semaphores[sid].name, '\0', MAX_LENGTH_NAME + 1);
    }
}
/*
 * Create or find a semaphore.
 * Params:
 *   state->ebx - user address of name of semaphore
 *   state->ecx - length of semaphore name
 *   state->edx - initial semaphore count
 * Returns: the global semaphore id
 */

int Sys_Open_Semaphore(struct Interrupt_State *state)
{
    int sid = 0;
    int ret = -1;
    bool iflag;
    KASSERT(state); // may be removed; just to avoid compiler warnings in distributed code.

    /* Open_Semaphore system call */
    if (state->ecx > MAX_LENGTH_NAME || state->ecx <= 0 || state->edx < 0)
        return EINVALID;
    iflag = Begin_Int_Atomic();
    //Check Semaphore
    for (sid = 0; sid < MAX_NUM_SEMAPHORE; ++sid)
    {
        if (strncmp(g_Semaphores[sid].name, (char *)state->ebx, state->ecx) == 0)
        {
            break;
        }
    }

    if (sid == MAX_NUM_SEMAPHORE) //Semaphore not created
    {
        sid = getSemaphore();
        if (sid < 0)
            ret = EUNSPECIFIED;
        else
        {
            strncpy(g_Semaphores[sid].name, (char *)state->ebx, MAX_NUM_SEMAPHORE);
            //Copy_User_String(state->ebx, state->ecx, MAX_LENGTH_NAME, &g_Semaphores[sid].name);
            if (ret != 0)
                return ret;
            g_Semaphores[sid].count = state->edx;
            g_Semaphores[sid].available = false;
            Clear_Thread_Queue(&g_Semaphores[sid].waitQueue);
            ret = sid;
        }
    }
    else if (sid >= 0) //Already created semaphore
    {
        ret = sid;
    }

    End_Int_Atomic(iflag);
    return ret;
}

/*
 * Acquire a semaphore.
 * Assume that the process has permission to access the semaphore,
 * the call will block until the semaphore count is >= 0.
 * Params:
 *   state->ebx - the semaphore id
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
int Sys_P(struct Interrupt_State *state)
{
    KASSERT(state); // may be removed; just to avoid compiler warnings in distributed code.

    /* P (semaphore acquire) system call */
    if ((int)state->ebx >= 0 && state->ebx < MAX_NUM_SEMAPHORE && !g_Semaphores[state->ebx].available)
        return EINVALID;
    bool iflag = Begin_Int_Atomic();
    g_Semaphores[state->ebx].count--;
    if (g_Semaphores[state->ebx].count < 0)
    {
        Wait(&g_Semaphores[state->ebx].waitQueue);
        //KASSERT(g_Semaphores[state->ebx].count == 1);
    }
    End_Int_Atomic(iflag);

    return 0;
}

/*
 * Release a semaphore.
 * Params:
 *   state->ebx - the semaphore id
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
int Sys_V(struct Interrupt_State *state)
{
    KASSERT(state); // may be removed; just to avoid compiler warnings in distributed code.

    /* V (semaphore release) system call */
    if ((int)state->ebx >= 0 && state->ebx < MAX_NUM_SEMAPHORE && !g_Semaphores[state->ebx].available)
        return EINVALID;
    bool iflag = Begin_Int_Atomic();
    g_Semaphores[state->ebx].count++;
    if (!(g_Semaphores[state->ebx].count == 1))
    {
        Wake_Up_One(&g_Semaphores[state->ebx].waitQueue);
    }
    End_Int_Atomic(iflag);

    return 0;
}

/*
 * Destroy our reference to a semaphore.
 * Params:
 *   state->ebx - the semaphore id
 *
 * Returns: 0 if successful, error code (< 0) if unsuccessful
 */
int Sys_Close_Semaphore(struct Interrupt_State *state)
{
    KASSERT(state); // may be removed; just to avoid compiler warnings in distributed code.

    /* Close_Semaphore system call */
    if ((int)state->ebx >= 0 && state->ebx < MAX_NUM_SEMAPHORE && !g_Semaphores[state->ebx].available)
        return EINVALID;
    bool iflag = Begin_Int_Atomic();
    g_Semaphores[state->ebx].available = true;
    Wake_Up(&g_Semaphores[state->ebx].waitQueue);
    End_Int_Atomic(iflag);
    return 0;
}
