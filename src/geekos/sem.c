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
#define MAX_SEMAPHORE_SIZE 20
struct Semaphore
{
    char *name; //semaphore name
    int count;  //semaphore value
    int user;
    struct Thread_Queue waitQueue;
};

static struct Semaphore *sem_list[MAX_SEMAPHORE_SIZE] = {
    0,
};

int __Copy_User_String(ulong_t uaddr, ulong_t len, ulong_t maxLen, char **pStr)
{
    int rc = 0;
    char *str;

    /* Ensure that string isn't too long. */
    if (len > maxLen)
        return EINVALID;

    /* Allocate space for the string. */
    str = (char *)Malloc(len + 1);
    if (str == 0)
    {
        rc = ENOMEM;
        goto done;
    }

    /* Copy data from user space. */
    if (!Copy_From_User(str, uaddr, len))
    {
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
    char *name = 0;
    int i = 0;
    KASSERT(state); // may be removed; just to avoid compiler warnings in distributed code.

    /* Open_Semaphore system call */
    if (state->ecx > MAX_LENGTH_NAME || (int)state->ecx <= 0 || (int)state->edx < 0)
        return EINVALID;

    //Check Semaphore
    ret = __Copy_User_String(state->ebx, state->ecx, MAX_LENGTH_NAME, &name);
    if (ret != 0) //error
        return ret;

    while (sem_list[sid] != 0)
    {
        //Find existed name
        if (strncmp(sem_list[sid]->name, name, state->ecx) == 0)
        {
            Free(name);
            ret = sid;
            sem_list[sid]->user++;
            goto done;
        }
        sid++;
        //No more semaphore IDs available and no semaphore with name exists
        if (sid == MAX_SEMAPHORE_SIZE)
        {
            Free(name);
            ret = ENOSPACE;
            goto done;
        }
    }
    sem_list[sid] = (struct Semaphore *)Malloc(sizeof(struct Semaphore));
    sem_list[sid]->name = name;
    sem_list[sid]->count = state->edx;
    Clear_Thread_Queue(&sem_list[sid]->waitQueue);
    ret = sid;
done:
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
    if ((int)state->ebx < 0 || state->ebx >= MAX_SEMAPHORE_SIZE || sem_list[state->ebx] == 0)
        return EINVALID;
    bool iflag = Begin_Int_Atomic();
    sem_list[state->ebx]->count--;
    if (sem_list[state->ebx]->count < 0)
    {
        Wait(&sem_list[state->ebx]->waitQueue);
        //KASSERT(sem_list[state->ebx]->count == 1);
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
    if ((int)state->ebx < 0 || state->ebx >= MAX_SEMAPHORE_SIZE || sem_list[state->ebx] == 0)
        return EINVALID;
    bool iflag = Begin_Int_Atomic();
    sem_list[state->ebx]->count++;
    if (!Is_Thread_Queue_Empty(&sem_list[state->ebx]->waitQueue))
    {
        Wake_Up_One(&sem_list[state->ebx]->waitQueue);
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
    if ((int)state->ebx < 0 || state->ebx >= MAX_SEMAPHORE_SIZE || sem_list[state->ebx] == 0)
        return EINVALID;
    bool iflag = Begin_Int_Atomic();
    sem_list[state->ebx]->user--;
    if (sem_list[state->ebx]->user == 0)
    {
        Free(sem_list[state->ebx]->name);
        Free(sem_list[state->ebx]);
    }
    End_Int_Atomic(iflag);
    return 0;
}
