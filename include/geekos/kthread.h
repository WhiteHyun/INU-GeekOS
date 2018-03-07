/*
 * Kernel threads
 * Copyright (c) 2001,2003 David H. Hovemeyer <daveho@cs.umd.edu>
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
 * $Revision: 1.34 $
 * 
 */

#ifndef GEEKOS_KTHREAD_H
#define GEEKOS_KTHREAD_H

#include <geekos/ktypes.h>
#include <geekos/list.h>


struct Kernel_Thread;
struct User_Context;
struct Interrupt_State;

/*
 * Queue of threads.
 * This is used for the run queue(s), and also for
 * thread synchronization and communication.
 */
DEFINE_LIST(Thread_Queue, Kernel_Thread);

/*
 * List which includes all threads.
 */
DEFINE_LIST(All_Thread_List, Kernel_Thread);

#define AFFINITY_ANY_CORE	-1

/*
 * Kernel thread context data structure.
 * NOTE: there is assembly code in lowlevel.asm that depends
 * on the offsets of the fields in this struct, so if you change
 * the layout, make sure everything gets updated.
 */
struct Kernel_Thread {
    ulong_t esp;                /* offset 0 */
    volatile ulong_t numTicks;  /* offset 4 */
    volatile ulong_t totalTime;
    int priority;
     DEFINE_LINK(Thread_Queue, Kernel_Thread);
    void *stackPage;
    struct User_Context *userContext;
    struct Kernel_Thread *owner;
    int affinity;               // prefered core = AFFINITY_ANY_CORE --> can run on any core
    int refCount;
    int detached;               // detached processes don't linger on Exit as zombies waiting for Wait to reap

    /* The kernel thread id; also used as process id */
    int pid;

    /* These fields are used to implement the Join() function */
    bool alive;
    struct Thread_Queue joinQueue;
    int exitCode;

    /* Link fields for list of all threads in the system. */
     DEFINE_LINK(All_Thread_List, Kernel_Thread);

    /* Array of MAX_TLOCAL_KEYS pointers to thread-local data. */
#define MAX_TLOCAL_KEYS 128
    const void *tlocalData[MAX_TLOCAL_KEYS];


    char threadName[20];

};

#ifdef GEEKOS
/*
 * Define Thread_Queue and All_Thread_List access and manipulation functions.
 */
IMPLEMENT_LIST(Thread_Queue, Kernel_Thread);
IMPLEMENT_LIST(All_Thread_List, Kernel_Thread);

static __inline__ void Enqueue_Thread(struct Thread_Queue *queue,
                                      struct Kernel_Thread *kthread) {
    Add_To_Back_Of_Thread_Queue(queue, kthread);
}

static __inline__ void Remove_Thread(struct Thread_Queue *queue,
                                     struct Kernel_Thread *kthread) {
    Remove_From_Thread_Queue(queue, kthread);
}
#endif

/*
 * Thread start functions should have this signature.
 */
typedef void (*Thread_Start_Func) (ulong_t arg);

/*
 * Thread priorities
 */
#define PRIORITY_IDLE    0
#define PRIORITY_USER    1
#define PRIORITY_LOW     2
#define PRIORITY_NORMAL  5
#define PRIORITY_HIGH   10

/*
 * Number of ready queue levels.
 */
#define MAX_QUEUE_LEVEL 4


/*
 * Scheduler operations.
 */
void Init_Scheduler(unsigned int CPUid, void *stack);
struct Kernel_Thread *Start_Kernel_Thread(Thread_Start_Func startFunc,
                                          ulong_t arg,
                                          int priority,
                                          bool detached,
                                          const char *name);
struct Kernel_Thread *Start_User_Thread(struct User_Context *userContext,
                                        bool detached);
void Make_Runnable(struct Kernel_Thread *kthread);
void Make_Runnable_Atomic(struct Kernel_Thread *kthread);
struct Kernel_Thread *Get_Current(void);
struct Kernel_Thread *Get_Next_Runnable(void);
void Schedule(void);
void Yield(void);
void Exit(int exitCode) __attribute__ ((noreturn));
int Join(struct Kernel_Thread *kthread);
struct Kernel_Thread *Lookup_Thread(int pid, int notOwner);

/*
 * Thread context switch function, defined in lowlevel.asm
 */
void Switch_To_Thread(struct Kernel_Thread *);

/*
 * Wait queue functions.
 */
void Wait(struct Thread_Queue *waitQueue);
void Wake_Up(struct Thread_Queue *waitQueue);
void Wake_Up_One(struct Thread_Queue *waitQueue);

/*
 * Pointer to currently executing thread.
 */
extern struct Kernel_Thread *g_currentThreads[];

/*
 * Boolean flag indicating that we need to choose a new runnable thread.
 */
extern int g_needReschedule[];

/*
 * Boolean flag indicating that preemption should be disabled.
 */
extern volatile int g_preemptionDisabled[];

/*
 * Thread-local data information
 */
#define MIN_DESTRUCTOR_ITERATIONS 4

typedef void (*tlocal_destructor_t) (void *);
typedef unsigned int tlocal_key_t;

extern int Tlocal_Create(tlocal_key_t *, tlocal_destructor_t);
extern void Tlocal_Put(tlocal_key_t, const void *);
extern void *Tlocal_Get(tlocal_key_t);

/* Print list of all threads, for debugging. */
extern void Dump_All_Thread_List(void);

extern void Wake_Up_Locked(struct Thread_Queue *waitQueue);

#endif /* GEEKOS_KTHREAD_H */
