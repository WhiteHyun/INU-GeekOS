/*
 * Synchronization primitives
 * Copyright (c) 2001, David H. Hovemeyer <daveho@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.13 $
 * 
 */

#ifndef GEEKOS_SYNCH_H
#define GEEKOS_SYNCH_H

#include <geekos/kthread.h>
#include <geekos/smp.h>

/*
 * mutex states
 */
enum { MUTEX_UNLOCKED, MUTEX_LOCKED };

struct Mutex {
    int state;
    struct Kernel_Thread *owner;
    struct Thread_Queue waitQueue;
};

#define MUTEX_INITIALIZER { MUTEX_UNLOCKED, 0, THREAD_QUEUE_INITIALIZER }

struct Condition {
    struct Thread_Queue waitQueue;
};

void Mutex_Init(struct Mutex *mutex);
void Mutex_Lock(struct Mutex *mutex);
void Mutex_Unlock(struct Mutex *mutex);
void Mutex_Lock_Interrupts_Disabled(struct Mutex *mutex);
void Mutex_Unlock_Interrupts_Disabled(struct Mutex *mutex);

void Cond_Init(struct Condition *cond);
void Cond_Wait(struct Condition *cond, struct Mutex *mutex);
void Cond_Signal(struct Condition *cond);
void Cond_Broadcast(struct Condition *cond);

#ifndef IS_HELD
#define IS_HELD(mutex) \
    ((mutex)->state == MUTEX_LOCKED && (mutex)->owner == CURRENT_THREAD)
#endif

#endif /* GEEKOS_SYNCH_H */
