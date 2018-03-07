/*
 * Synchronization primitives
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
 * $Revision: 1.13 $
 * 
 */

#include <geekos/kthread.h>
#include <geekos/int.h>
#include <geekos/kassert.h>
#include <geekos/screen.h>
#include <geekos/synch.h>
#include <geekos/smp.h>

/*
 * NOTES:
 * - The GeekOS mutex and condition variable APIs are based on those
 *   in pthreads.
 * - Unlike disabling interrupts, mutexes offer NO protection against
 *   concurrent execution of interrupt handlers.  Mutexes and
 *   condition variables should only be used from kernel threads,
 *   with interrupts enabled.
 */

/* ----------------------------------------------------------------------
 * Private functions
 * ---------------------------------------------------------------------- */

/*
 * The mutex is currently locked.
 * Wait in the mutex's wait queue.
 */
static void Mutex_Wait(struct Mutex *mutex) {
    KASSERT(mutex->state == MUTEX_LOCKED);
    Wait(&mutex->waitQueue);
}


static __inline__ void Mutex_Lock_Imp_Interrupts_Disabled(struct Mutex
                                                          *mutex) {
    while (mutex->state == MUTEX_LOCKED) {
        Mutex_Wait(mutex);
    }

    KASSERT0(mutex->owner == 0,
             "Expect mutex owner to be clear before we lock it\n");

    /* Now it's ours! */
    mutex->state = MUTEX_LOCKED;
    mutex->owner = get_current_thread(0);       /* interrupts already disabled, can use fast impl. */
}

/*
 * Lock given mutex.
 */
static __inline__ void Mutex_Lock_Imp(struct Mutex *mutex) {
    /* Make sure we're not already holding the mutex */
    KASSERT(!IS_HELD(mutex));

    /* Wait until the mutex is in an unlocked state */
    Disable_Interrupts();

    Mutex_Lock_Imp_Interrupts_Disabled(mutex);

    Enable_Interrupts();
}

static __inline__ void Mutex_Unlock_Imp_Interrupts_Disabled(struct Mutex
                                                            *mutex) {
    /* Make sure mutex was actually acquired by this thread. */
    struct Kernel_Thread *current = get_current_thread(0);      /* interrupts already disabled. */
    KASSERT0((mutex)->state == MUTEX_LOCKED,
             "Attempting to unlock a mutex that is already unlocked");
    KASSERT0((mutex)->owner == current,
             "Attempting to unlock a mutex that this thread did not acquire");

    /* clear out the owner information, as we no longer need it. */
    mutex->owner = 0;

    /* Unlock the mutex. */

    /*
     * If there are threads waiting to acquire the mutex,
     * wake one of them up.  Note that it is legal to inspect
     * the queue with interrupts enabled because preemption
     * is disabled, and therefore we know that no thread can
     * concurrently add itself to the queue.
     */
    if(!Is_Thread_Queue_Empty(&mutex->waitQueue)) {
        mutex->state = MUTEX_UNLOCKED;
        Wake_Up_One(&mutex->waitQueue);
    } else {
        mutex->state = MUTEX_UNLOCKED;
    }
}

/*
 * Unlock given mutex.
 */
static __inline__ void Mutex_Unlock_Imp(struct Mutex *mutex) {
    Disable_Interrupts();       /* an interrupt once could move us from one processor to another as
                                   we figure out what the current thread id is.  This is no longer the 
                                   case, since CURRENT_THREAD disables interrupts, but it's safer to proceed
                                   with interrupts disabled, since we have to disable them anyway. */
    Mutex_Unlock_Imp_Interrupts_Disabled(mutex);
    Enable_Interrupts();
}


/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

/*
 * Initialize given mutex.
 */
void Mutex_Init(struct Mutex *mutex) {
    mutex->state = MUTEX_UNLOCKED;
    mutex->owner = 0;
    Clear_Thread_Queue(&mutex->waitQueue);
    Spin_Lock_Init(&mutex->waitQueue.lock);     /* ns15 */
}

/*
 * Lock given mutex.
 */
void Mutex_Lock(struct Mutex *mutex) {
    KASSERT(Interrupts_Enabled());

    // ns 15 debugging:
    KASSERT(!g_preemptionDisabled[Get_CPU_ID()]);

    Mutex_Lock_Imp(mutex);
}

/* If interrupts are already disabled, no sense enabling them for this.
   however, we may go to sleep allowing someone else to run, so don't assume 
   disabled interrupts means no one else runs. */
void Mutex_Lock_Interrupts_Disabled(struct Mutex *mutex) {
    KASSERT(!Interrupts_Enabled());
    Mutex_Lock_Imp_Interrupts_Disabled(mutex);
}

/*
 * Unlock given mutex.
 */
void Mutex_Unlock(struct Mutex *mutex) {
    KASSERT(Interrupts_Enabled());
    Mutex_Unlock_Imp(mutex);
}

void Mutex_Unlock_Interrupts_Disabled(struct Mutex *mutex) {
    KASSERT(!Interrupts_Enabled());
    Mutex_Unlock_Imp_Interrupts_Disabled(mutex);
}

/*
 * Initialize given condition.
 */
void Cond_Init(struct Condition *cond) {
    Clear_Thread_Queue(&cond->waitQueue);
    Spin_Lock_Init(&cond->waitQueue.lock);
}

/*
 * Wait on given condition (protected by given mutex).
 */
void Cond_Wait(struct Condition *cond, struct Mutex *mutex) {
    KASSERT(Interrupts_Enabled());

    /* Ensure mutex is held. */
    KASSERT(IS_HELD(mutex));

    /*
     * Release the mutex, but leave preemption disabled.
     * No other threads will be able to run before this thread
     * is able to wait.  Therefore, this thread will not
     * miss the eventual notification on the condition.
     */
    Disable_Interrupts();
    /* ns15, switched to disable interrupts before unlocking the 
       mutex and waiting on the condition wait queue. */
    Mutex_Unlock_Imp_Interrupts_Disabled(mutex);

    /*
     * Atomically reenable preemption and wait in the condition wait queue.
     * Other threads can run while this thread is waiting,
     * and eventually one of them will call Cond_Signal() or Cond_Broadcast()
     * to wake up this thread.
     * On wakeup, disable preemption again.
     */
    Wait(&cond->waitQueue);
    Enable_Interrupts();

    /* Reacquire the mutex. */
    Mutex_Lock_Imp(mutex);
}

/*
 * Wake up one thread waiting on the given condition.
 * The mutex guarding the condition should be held!
 */
void Cond_Signal(struct Condition *cond) {
    KASSERT(Interrupts_Enabled());
    Disable_Interrupts();       /* prevent scheduling */
    Wake_Up_One(&cond->waitQueue);
    Enable_Interrupts();        /* resume scheduling */
}

/*
 * Wake up all threads waiting on the given condition.
 * The mutex guarding the condition should be held!
 */
void Cond_Broadcast(struct Condition *cond) {
    KASSERT(Interrupts_Enabled());
    Disable_Interrupts();       /* prevent scheduling */
    Wake_Up(&cond->waitQueue);
    Enable_Interrupts();        /* resume scheduling */
}
