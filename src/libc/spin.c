/* Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2003,2013,2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * 
 * All rights reserved.
 * 
 * This code may not be resdistributed without the permission of the
 * copyright holders.  Any student solutions using any of this code base
 * constitute derviced work and may not be redistributed in any form.
 * This includes (but is not limited to) posting on public forums or web
 * sites, providing copies to past, present, or future students enrolled
 * in similar operating systems courses to the University of Maryland's
 * CMSC412 course. 
 * 
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 */

/*
 * Define interface to user space spin locks used in GeekOS
 *
 */

#include "geekos/projects.h"

typedef struct {
    volatile int lock;
} User_Spin_Lock_t;

int Is_Locked(User_Spin_Lock_t * lock) {
    TODO_P(PROJECT_CLONE, "Is_Locked");
    return 0;
}

void Spin_Lock_Init(User_Spin_Lock_t * lock) {
    TODO_P(PROJECT_CLONE, "Is_Locked");
}

void Spin_Lock(User_Spin_Lock_t * lock) {
    TODO_P(PROJECT_CLONE, "Spin_Lock");
    /* put the address of lock (which is the first field holding the lock) into ebx */
  asm("mov %0, %%ebx": : "r"(lock):"%ebx");
}

int Spin_Unlock(User_Spin_Lock_t * lock) {
    TODO_P(PROJECT_CLONE, "Spin_Unlock");
    return 0;
}
