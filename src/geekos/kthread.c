/*
 * Kernel threads
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
 */

#include <geekos/kassert.h>
#include <geekos/defs.h>
#include <geekos/screen.h>
#include <geekos/int.h>
#include <geekos/mem.h>
#include <geekos/symbol.h>
#include <geekos/string.h>
#include <geekos/kthread.h>
#include <geekos/malloc.h>
#include <geekos/user.h>
#include <geekos/alarm.h>
#include <geekos/projects.h>
#include <geekos/smp.h>
#include <geekos/synch.h>

extern Spin_Lock_t kthreadLock;

/* ----------------------------------------------------------------------
 * Private data
 * ---------------------------------------------------------------------- */

/*
 * List of all threads in the system.
 */
struct All_Thread_List s_allThreadList;

/*
 * Queue of runnable threads.
 */
struct Thread_Queue s_runQueue;
extern bool Kernel_Is_Locked();

/*
 * Current thread.
 */
struct Kernel_Thread *g_currentThreads[MAX_CPUS];
/*
 * Boolean flag indicating that we need to choose a new runnable thread.
 * It is checked by the interrupt return code (Handle_Interrupt,
 * in lowlevel.asm) before returning from an interrupt.
 */
int g_needReschedule[MAX_CPUS];

/*
 * Boolean flag indicating that preemption is disabled.
 * When set, external interrupts (such as the timer tick)
 * will not cause a new thread to be selected.
 */
volatile int g_preemptionDisabled[MAX_CPUS];

/*
 * Queue of finished threads needing disposal,
 * and a wait queue used for communication between exited threads
 * and the reaper thread.
 */
static struct Thread_Queue s_graveyardQueue;
static struct Mutex s_graveyardMutex;
static struct Thread_Queue s_reaperWaitQueue;

/*
 * Counter for keys that access thread-local data, and an array
 * of destructors for freeing that data when the thread dies.  This is
 * based on POSIX threads' thread-specific data functionality.
 */
static unsigned int s_tlocalKeyCounter = 0;
static tlocal_destructor_t s_tlocalDestructors[MAX_TLOCAL_KEYS];

static struct Kernel_Thread *Get_Next_Runnable_Locked(void);

/* ----------------------------------------------------------------------
 * Private functions
 * ---------------------------------------------------------------------- */


Spin_Lock_t pidLock;

int nextPid() {
    int ret;
    static int nextFreePid = 1;

    Spin_Lock(&pidLock);
    ret = nextFreePid++;
    Spin_Unlock(&pidLock);
    return ret;
}


/*
 * Initialize a new Kernel_Thread.
 */
static void Init_Thread(struct Kernel_Thread *kthread, void *stackPage,
                        int priority, bool detached) {
    struct Kernel_Thread *owner = CURRENT_THREAD;

    memset(kthread, '\0', sizeof(*kthread));
    kthread->stackPage = stackPage;
    KASSERT(stackPage);
    kthread->esp = ((ulong_t) kthread->stackPage) + PAGE_SIZE;
    kthread->numTicks = 0;
    kthread->detached = detached;
    kthread->priority = priority;
    kthread->userContext = 0;
    kthread->owner = owner;
    kthread->affinity = AFFINITY_ANY_CORE;
    kthread->totalTime = 0;

    /*
     * The thread has an implicit self-reference and 
     * has a reference from its parent.  
     * 
     * In some assignments, "detached" process may not
     * have a reference from a parent and cannot be 
     * Wait()ed on.
     */
    kthread->refCount = detached ? 1 : 2;
    kthread->alive = true;
    Clear_Thread_Queue(&kthread->joinQueue);
    kthread->pid = nextPid();
}

/*
 * Create a new raw thread object.
 * Returns a null pointer if there isn't enough memory.
 */
static struct Kernel_Thread *Create_Thread(int priority, bool detached) {
    struct Kernel_Thread *kthread;
    void *stackPage = 0;

    /*
     * For now, just allocate one page each for the thread context
     * object and the thread's stack.
     */
    kthread = Alloc_Page();
    if(kthread == 0)
        return 0;

    stackPage = Alloc_Page();
    if(stackPage == 0) {
        Free_Page(kthread);
        return 0;
    }

    /*Print("New thread @ %x, stack @ %x\n", kthread, stackPage); */

    /*
     * Initialize the stack pointer of the new thread
     * and accounting info
     */
    Init_Thread(kthread, stackPage, priority, detached);

    /* Add to the list of all threads in the system. */
    Add_To_Back_Of_All_Thread_List(&s_allThreadList, kthread);

    return kthread;
}

/*
 * Push a dword value on the stack of given thread.
 * We use this to set up some context for the thread before
 * we make it runnable.
 */
static __inline__ void Push(struct Kernel_Thread *kthread, ulong_t value) {
    kthread->esp -= 4;
    *((ulong_t *) kthread->esp) = value;
}

/*
 * Destroy given thread.
 * This function should perform all cleanup needed to
 * reclaim the resources used by a thread.
 * Called with kthread lock held
 */
static void Destroy_Thread(struct Kernel_Thread *kthread) {
    /*
     * Detach the thread's user context, if any.
     * This will reclaim pages used by user processes.
     */
    if(kthread->userContext != 0)
        Detach_User_Context(kthread);

    /* Remove from list of all threads */
    Remove_From_All_Thread_List(&s_allThreadList, kthread);

    /* Dispose of the thread's memory. */
    Free_Page(kthread->stackPage);
    kthread->stackPage = 0;
    Free_Page(kthread);
}

/*
 * Hand given thread to the reaper for destruction.
 * Must NOT be called with interrupts enabled! Because
 * that will cause grief if we are exiting and are 
 * interrupted and thus placed on the run queue and the
 * graveyard queue!
 */
static void Reap_Thread(struct Kernel_Thread *kthread) {
    KASSERT(kthread);
    KASSERT(kthread != CPUs[0].idleThread);
    KASSERT(kthread != CPUs[1].idleThread);

    /* Interrupts must be disabled for the Wake_Up,
       but also best to have interrupts disabled 
       completely while we exit and deposit ourselves
       on the graveyard queue so that we're not 
       put on the run queue */
    KASSERT(!Interrupts_Enabled());

    Mutex_Lock_Interrupts_Disabled(&s_graveyardMutex);
    Enqueue_Thread(&s_graveyardQueue, kthread);
    Mutex_Unlock_Interrupts_Disabled(&s_graveyardMutex);

    Wake_Up(&s_reaperWaitQueue);
}

/*
 * Called when a reference to the thread is broken.
 */
void Detach_Thread(struct Kernel_Thread *kthread) {
    int count;
    bool iflag;

    KASSERT(kthread);
    KASSERT(kthread->esp);      /* sanity */
    KASSERT(kthread->refCount > 0);

    /* called with interrupts enabled if the parent is
       detaching; called with interrupts disabled (so we
       don't get rescheduled) if the kthread itself is
       exiting; ensure interrupts are off for the queue
       manipulations. */

    iflag = Begin_Int_Atomic();
    count = --kthread->refCount;

    if(count == 0) {
        Reap_Thread(kthread);
    }
    End_Int_Atomic(iflag);
}

/*
 * This function performs any needed initialization before
 * a thread start function is executed.  Currently we just use
 * it to enable interrupts (since Schedule() always activates
 * a thread with interrupts disabled).
 */
static void Launch_Thread(void) {
    Enable_Interrupts();
}

/*
 * Push initial values for general purpose registers.
 */
static void Push_General_Registers(struct Kernel_Thread *kthread) {
    KASSERT0(kthread,
             "Setup_Kernel_Thread called with null kthread argument\n");
    /*
     * Push initial values for saved general-purpose registers.
     * (The actual values are not important.)
     */
    Push(kthread, 0);           /* eax */
    Push(kthread, 0);           /* ebx */
    Push(kthread, 0);           /* edx */
    Push(kthread, 0);           /* edx */
    Push(kthread, 0);           /* esi */
    Push(kthread, 0);           /* edi */
    Push(kthread, 0);           /* ebp */
}

/*
 * Shutdown a kernel thread.
 * This is called if a kernel thread exits by falling off
 * the end of its start function.
 */
static void Shutdown_Thread(void) {
    Exit(0);
}

/*
 * Set up the initial context for a kernel-mode-only thread.
 */
static void Setup_Kernel_Thread(struct Kernel_Thread *kthread,
                                Thread_Start_Func startFunc,
                                ulong_t arg) {
    KASSERT0(kthread,
             "Setup_Kernel_Thread called with null kthread argument\n");
    /*
     * Push the argument to the thread start function, and the
     * return address (the Shutdown_Thread function, so the thread will
     * go away cleanly when the start function returns).
     */
    Push(kthread, arg);
    Push(kthread, (ulong_t) & Shutdown_Thread);

    /* Push the address of the start function. */
    Push(kthread, (ulong_t) startFunc);

    /*
     * To make the thread schedulable, we need to make it look
     * like it was suspended by an interrupt.  This means pushing
     * an "eflags, cs, eip" sequence onto the stack,
     * as well as int num, error code, saved registers, etc.
     */

    /*
     * The EFLAGS register will have all bits clear.
     * The important constraint is that we want to have the IF
     * bit clear, so that interrupts are disabled when the
     * thread starts.
     */
    Push(kthread, 0UL);         /* EFLAGS */

    /*
     * As the "return address" specifying where the new thread will
     * start executing, use the Launch_Thread() function.
     */
    Push(kthread, KERNEL_CS);
    Push(kthread, (ulong_t) & Launch_Thread);

    /* Push fake error code and interrupt number. */
    Push(kthread, 0);
    Push(kthread, 0);

    /* Push initial values for general-purpose registers. */
    Push_General_Registers(kthread);

    /*
     * Push values for saved segment registers.
     * Only the ds and es registers will contain valid selectors.
     * The fs and gs registers are not used by any instruction
     * generated by gcc.
     */
    Push(kthread, KERNEL_DS);   /* ds */
    Push(kthread, KERNEL_DS);   /* es */
    Push(kthread, 0);           /* fs */
    Push(kthread, 0);           /* gs */
    TODO_P(PROJECT_PERCPU, "set gs to the per-cpu segment");
}

/*
 * Set up the a user mode thread.
 */
/*static*/ void Setup_User_Thread(
                                     struct Kernel_Thread *kthread,
                                     struct User_Context *userContext) {
    extern int userDebug;

    /*
     * Interrupts in user mode MUST be enabled.
     * All other EFLAGS bits will be clear.
     */
    ulong_t eflags = EFLAGS_IF;

    unsigned csSelector = userContext->csSelector;
    unsigned dsSelector = userContext->dsSelector;

    KASSERT(kthread != NULL);

    Attach_User_Context(kthread, userContext);

    /*
     * Make the thread's stack look like it was interrupted
     * while in user mode.
     */

    /* Stack segment and stack pointer within user mode. */
    Push(kthread, dsSelector);  /* user ss */
    Push(kthread, userContext->stackPointerAddr);       /* user esp */

    /* eflags, cs, eip */
    Push(kthread, eflags);
    Push(kthread, csSelector);
    Push(kthread, userContext->entryAddr);
    if(userDebug)
        Print("Entry addr=%lx\n", userContext->entryAddr);

    /* Push fake error code and interrupt number. */
    Push(kthread, 0);
    Push(kthread, 0);

    /*
     * Push initial values for general-purpose registers.
     * The only important register is esi, which we use to
     * pass the address of the argument block.
     */
    Push(kthread, 0);           /* eax */
    Push(kthread, 0);           /* ebx */
    Push(kthread, 0);           /* edx */
    Push(kthread, 0);           /* edx */
    Push(kthread, userContext->argBlockAddr);   /* esi */
    Push(kthread, 0);           /* edi */
    Push(kthread, 0);           /* ebp */

    /* Push initial values for the data segment registers. */
    Push(kthread, dsSelector);  /* ds */
    Push(kthread, dsSelector);  /* es */
    Push(kthread, dsSelector);  /* fs */
    Push(kthread, dsSelector);  /* gs */

    /* allow it to run on any core */
    kthread->affinity = -1;
}


/*
 * This is the body of the idle thread.  Its job is to preserve
 * the invariant that a runnable thread always exists,
 * i.e., the run queue is never empty.
 */
static void Idle(ulong_t arg __attribute__ ((unused))) {
    while (true) {
        /* 
         * The hlt instruction tells the CPU to wait until an interrupt is called.
         * We call this in this loop so the Idle process does not eat up 100% cpu,
         * and make our laptops catch fire.
         */
        __asm__("hlt");

    }
}

/*
 * The reaper thread.  Its job is to de-allocate memory
 * used by threads which have finished.
 */
static void Reaper(ulong_t arg __attribute__ ((unused))) {
    struct Kernel_Thread *kthread;

    //     Disable_Interrupts();

    while (true) {
        /* Interact directly with the queue lock, since we will take the entire list at once. */
        Mutex_Lock(&s_graveyardMutex);
        Spin_Lock(&s_graveyardQueue.lock);
        /* See if there are any threads needing disposal. */
        if((kthread = s_graveyardQueue.head) == 0) {
            /* Graveyard is empty, so wait for a thread to die. */
            Spin_Unlock(&s_graveyardQueue.lock);
            Mutex_Unlock(&s_graveyardMutex);
            Disable_Interrupts();
            Wait(&s_reaperWaitQueue);
            Enable_Interrupts();

        } else {
            /* Make the graveyard queue empty. */
            Clear_Thread_Queue(&s_graveyardQueue);

            /* lock again to check grave yard */
            Spin_Unlock(&s_graveyardQueue.lock);
            Mutex_Unlock(&s_graveyardMutex);

            /* Dispose of the dead threads. */
            while (kthread != 0) {
                struct Kernel_Thread *next =
                    Get_Next_In_Thread_Queue(kthread);
                // Print("Reaper on CPU%d: disposing of thread %s (pid %d) @ %x, stack @ %x\n",
                // Get_CPU_ID(), kthread->threadName, kthread->pid, kthread, kthread->stackPage);
                // Spin_Lock(&kthreadLock); /* Destroy_Thread alters the all thread list. */
                Destroy_Thread(kthread);        /* so needs the kthreadlock. */
                // Spin_Unlock(&kthreadLock); /* but grabbing it causes trouble. */
                KASSERT(kthread != next);
                kthread = next;
            }

            /* maybe needed if we spend all our time with interrupts
               disabled, but if we are sane, we should spend our
               time in Wait(). */
            //ns14 Enable_Interrupts();
            //ns14 Yield();
            //ns14 Disable_Interrupts();

        }
    }
}

/*
 * Find the best (highest priority) thread in given
 * thread queue.  Returns null if queue is empty.
 */
static __inline__ struct Kernel_Thread *Find_Best(struct Thread_Queue
                                                  *queue) {
    int cpuID;

    KASSERT(Is_Locked(&kthreadLock));

    cpuID = Get_CPU_ID();

    /* Pick the highest priority thread */
    struct Kernel_Thread *kthread = queue->head, *best = 0;
    while (kthread != 0) {
        if(kthread->affinity == AFFINITY_ANY_CORE ||
           kthread->affinity == cpuID) {
            if(best == 0 || kthread->priority > best->priority)
                // if (kthread->alive) - must finish exiting if not alive.
                best = kthread;
        }
        kthread = Get_Next_In_Thread_Queue(kthread);
    }

    if(!best) {
        best = CPUs[cpuID].idleThread;
    }

    return best;
}


/*
 * Acquires pointer to thread-local data from the current thread
 * indexed by the given key.  Assumes interrupts are off.
 */
static __inline__ const void **Get_Tlocal_Pointer(tlocal_key_t k) {
    struct Kernel_Thread *current = CURRENT_THREAD;

    KASSERT(k < MAX_TLOCAL_KEYS);

    return &current->tlocalData[k];
}

/*
 * Clean up any thread-local data upon thread exit.  Assumes
 * this is called with interrupts disabled.  We follow the POSIX style
 * of possibly invoking a destructor more than once, because a
 * destructor to some thread-local data might cause other thread-local
 * data to become alive once again.  If everything is NULL by the end
 * of an iteration, we are done.
 */
static void Tlocal_Exit(struct Kernel_Thread *curr) {
    int i, j, called = 0;


    for(j = 0; j < MIN_DESTRUCTOR_ITERATIONS; j++) {

        for(i = 0; i < MAX_TLOCAL_KEYS; i++) {

            void *x = (void *)curr->tlocalData[i];
            if(x != NULL && s_tlocalDestructors[i] != NULL) {

                curr->tlocalData[i] = NULL;
                called = 1;

                Enable_Interrupts();
                s_tlocalDestructors[i] (x);
                Disable_Interrupts();
            }
        }
        if(!called)
            break;
    }
}


/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

void Init_Scheduler(unsigned int cpuID, void *stack) {
    struct Kernel_Thread *mainThread =
        (struct Kernel_Thread *)Alloc_Page();

    memcpy(mainThread, (void *)KERN_THREAD_OBJ,
           sizeof(struct Kernel_Thread));

    /*
     * Create initial kernel thread context object and stack,
     * and make them current.
     */
    Init_Thread(mainThread, stack, PRIORITY_NORMAL, true);
    g_currentThreads[Get_CPU_ID()] = mainThread;
    TODO_P(PROJECT_PERCPU, "set the current thread now that we have one");
    Add_To_Back_Of_All_Thread_List(&s_allThreadList, mainThread);
    strcpy(mainThread->threadName, "{Main}");

    /*
     * Create the idle thread.
     */
    /*Print("starting idle thread\n"); */
    char name[30];
    strcpy(name, "{Idle-#?}");
    name[7] = cpuID + '0';
    CPUs[cpuID].idleThread =
        Start_Kernel_Thread(Idle, 0, PRIORITY_IDLE, true, name);
    CPUs[cpuID].idleThread->owner = NULL;
    CPUs[cpuID].idleThread->affinity = cpuID;

    if(!cpuID) {
        /*
         * Create the reaper thread.
         */
        /*Print("starting reaper thread\n"); */
        struct Kernel_Thread *reaper;

        reaper =
            Start_Kernel_Thread(Reaper, 0, PRIORITY_NORMAL, true,
                                "{Reaper}");
    }
}

/*
 * Start a kernel-mode-only thread, using given function as its body
 * and passing given argument as its parameter.  Returns pointer
 * to the new thread if successful, null otherwise.
 *
 * startFunc - is the function to be called by the new thread
 * arg - is a paramter to pass to the new function
 * priority - the priority of this thread (use PRIORITY_NORMAL) for
 *    most things
 * detached - use false for kernel threads
 */
struct Kernel_Thread *Start_Kernel_Thread(Thread_Start_Func startFunc,
                                          ulong_t arg,
                                          int priority,
                                          bool detached,
                                          const char *name) {
    struct Kernel_Thread *kthread = Create_Thread(priority, detached);
    if(kthread != 0) {
        /*
         * Create the initial context for the thread to make
         * it schedulable.
         */
        Setup_Kernel_Thread(kthread, startFunc, arg);

        /* Atomically put the thread on the run queue. */
        Make_Runnable_Atomic(kthread);

        strcpy(kthread->threadName, name);
    }

    return kthread;
}

/*
 * Start a user-mode thread (i.e., a process), using given user context.
 * Returns pointer to the new thread if successful, null otherwise.
 */
struct Kernel_Thread *Start_User_Thread(struct User_Context *userContext,
                                        bool detached) {
    struct Kernel_Thread *kthread =
        Create_Thread(PRIORITY_USER, detached);
    if(kthread != 0) {
        /* Set up the thread, and put it on the run queue */
        Setup_User_Thread(kthread, userContext);
        Make_Runnable_Atomic(kthread);
    }

    return kthread;
}

/*
 * Add given thread to the run queue, so that it
 * may be scheduled.  Must be called with interrupts disabled!
 */
void Make_Runnable(struct Kernel_Thread *kthread) {
    KASSERT(!Interrupts_Enabled());

    KASSERT0(kthread->inThread_Queue == NULL,
             "attempting to make runnable a thread that is in another list.");

    if(kthread->priority == PRIORITY_IDLE)
        return;                 /* idle handled oob ns14 */

    Enqueue_Thread(&s_runQueue, kthread);
    TODO_P(PROJECT_SCHEDULING, "replace make runnable as needed");
}



/*
 * Atomically make a thread runnable.
 * Assumes interrupts are currently enabled.
 */
void Make_Runnable_Atomic(struct Kernel_Thread *kthread) {
    Disable_Interrupts();
    Spin_Lock(&kthreadLock);
    Make_Runnable(kthread);
    Spin_Unlock(&kthreadLock);
    Enable_Interrupts();
}

/*
 * Get the thread that currently has the CPU.
 */
struct Kernel_Thread *Get_Current(void) {
    return CURRENT_THREAD;
}

/*
 * Get the next runnable thread from the run queue.
 * This is the scheduler.
 */
struct Kernel_Thread *Get_Next_Runnable_Locked(void) {
    struct Kernel_Thread *best = 0;
    int cpuID = Get_CPU_ID();

    best = Find_Best(&s_runQueue);
    KASSERT(best != 0);
    if(best->priority != PRIORITY_IDLE) {       /* ns14 oob idle */
        Remove_Thread(&s_runQueue, best);
    }

    KASSERT(Is_Locked(&kthreadLock));
    TODO_P(PROJECT_SCHEDULING, "fix Get_Next_Runnable");
    return best;
}

/* called by lowlevel.asm in handle_interrupt. */
struct Kernel_Thread *Get_Next_Runnable(void) {
    struct Kernel_Thread *ret;

    /* ns14 */
    //Enable_Interrupts();
    if(!Try_Spin_Lock(&kthreadLock)) {
        /* attempt at dealing with the potential problem of waiting for the
           kthread lock while holding the global lock... prefer to schedule
           the idle thread instead of deadlocking. */
        int cpuID = Get_CPU_ID();
        Print
            ("GNR returning Idle thread on cpu %d to %p due to kthreadlock\n",
             cpuID, __builtin_return_address(0));
        return CPUs[cpuID].idleThread;
    }

    /* Spin_Lock(&kthreadLock); */
    /* ns14 - hacking at getting this right, since we will need the kthreadlock
       for a little while, try to keep the processor longer */
    g_preemptionDisabled[Get_CPU_ID()] = true;

    ret = Get_Next_Runnable_Locked();
    // Print("about to run %d, esp = %x\n", ret->pid, ret->esp);

    Spin_Unlock(&kthreadLock);

    /* ns14 */
    //Disable_Interrupts();
    g_preemptionDisabled[Get_CPU_ID()] = false;

    /* at least could be the idle thread */
    KASSERT(ret);

    /* ensure the new thread has a valid kernel stack pointer. */
    KASSERT(((unsigned long)(ret->esp - 1) & ~0xfff) ==
            ((unsigned long)ret->stackPage));

    return ret;
}

/*
 * Schedule a thread that is waiting to run.
 * Must be called with interrupts off!
 * The current thread should already have been placed
 * on whatever queue is appropriate (i.e., either the
 * run queue if it is still runnable, or a wait queue
 * if it is waiting for an event to occur).
 */
void Schedule(void) {
    struct Kernel_Thread *runnable;

    /* Make sure interrupts really are disabled */
    KASSERT(!Interrupts_Enabled());
    KASSERT0(Kernel_Is_Locked(),
             "kernel should be locked, since interrupts are disabled.\n");

    /* Preemption should not be disabled. */
    // ns15 KASSERT(!g_preemptionDisabled[Get_CPU_ID()]);
    g_preemptionDisabled[Get_CPU_ID()] = false;

    /* Get next thread to run from the run queue */
    runnable = Get_Next_Runnable();

    // Print("switching to %d, %s (core %d)\n", runnable->pid, runnable->userContext? runnable->userContext->name : runnable->threadName, Get_CPU_ID());

    /*
     * Activate the new thread, saving the context of the current thread.
     * Eventually, this thread will get re-activated and Switch_To_Thread()
     * will "return", and then Schedule() will return to wherever
     * it was called from.
     */
    Switch_To_Thread(runnable);
}

/*
 * Voluntarily give up the CPU to another thread.
 * Does nothing if no other threads are ready to run.
 */
void Yield(void) {
    Disable_Interrupts();
    Make_Runnable(get_current_thread(0));
    Schedule();
    Enable_Interrupts();
}


/*
 * Exit the current thread.
 * Calling this function initiates a context switch.
 */
void Exit(int exitCode) {
    extern int Munmap_Impl(uint_t addr);

    bool iflag;
    struct Kernel_Thread *current = CURRENT_THREAD;

    /* ns14 old form disabled interrupts entirely; now narrower int_atomic sections. */
    KASSERT0(Interrupts_Enabled(),
             "believe exit should be called with interrupts enabled");

    /* Thread is dead */
    current->exitCode = exitCode;
    current->alive = false;



    /* Remove timer references this thread has held */
    iflag = Begin_Int_Atomic(); /* seems less than necessary, but appeases. */
    Alarm_Cancel_For_Thread(current);

    /* Clean up any thread-local memory */
    Tlocal_Exit(CURRENT_THREAD);

/* Notify the thread's owner, if any */ Wake_Up(&current->joinQueue);
    End_Int_Atomic(iflag);


    /* Remove the thread's implicit reference to itself. */
    iflag = Begin_Int_Atomic(); /* fundamentally necessary to detach and schedule uninterrupted! */
    Detach_Thread(CURRENT_THREAD);

    /*
     * Schedule a new thread.
     * Since the old thread wasn't placed on any
     * thread queue, it won't get scheduled again.
     */
    Schedule();
    End_Int_Atomic(iflag);

    /* Shouldn't get here */
    KASSERT(false);
}

/*
 * Wait for given thread to die.
 * Interrupts must be enabled.
 * Returns the thread exit code.
 */
int Join(struct Kernel_Thread *kthread) {
    int exitCode;

    KASSERT(Interrupts_Enabled());

    Disable_Interrupts();

    /* It is only legal for the owner to join */
    KASSERT(kthread->owner == CURRENT_THREAD);

    /* Wait for it to die */
    while (kthread->alive) {
        Wait(&kthread->joinQueue);
    }

    /* Get thread exit code. */
    exitCode = kthread->exitCode;

    /* once joined we are effectively detached - prevents Exit from this thread trying to double detach */
    /* do this before detach since deatch can free the thread */
    kthread->detached = 1;


    Enable_Interrupts();

    /* Release our (the parent's) reference to the thread */
    Detach_Thread(kthread);

    return exitCode;
}


/*
 * Look up a thread by its process id.
 * notOwner is true if the thread is assumed to be the caller's
 * owner.  If the thread is NOT the owner, then it is assumed
 * the caller will not retain the extra reference with interrupts
 * enabled, or else the thread could die and create a dangling pointer.
 */
struct Kernel_Thread *Lookup_Thread(int pid, int notOwner) {
    struct Kernel_Thread *result = 0;


    bool iflag = Begin_Int_Atomic();

    struct Kernel_Thread *current = get_current_thread(0);      /* interrupts disabled, may use fast */

    /*
     * TODO: we could remove the requirement that the caller
     * needs to be the thread's owner by specifying that another
     * reference is added to the thread before it is returned.
     */

    Spin_Lock(&kthreadLock);
    result = Get_Front_Of_All_Thread_List(&s_allThreadList);
    while (result != 0) {
        if(result->pid == pid) {
            if(current != result->owner && !notOwner)
                result = 0;
            break;
        }
        result = Get_Next_In_All_Thread_List(result);
    }
    Spin_Unlock(&kthreadLock);

    End_Int_Atomic(iflag);


    return result;
}

/*
 * Wait on given wait queue.
 * Must be called with interrupts disabled!
 * Note that the function will return with interrupts
 * disabled.  This is desirable, because it allows us to
 * atomically test a condition that can be affected by an interrupt
 * and wait for it to be satisfied (if necessary).
 * See the Wait_For_Key() function in keyboard.c
 * for an example.
 */

void Wait(struct Thread_Queue *waitQueue) {
    struct Kernel_Thread *current = get_current_thread(0);      /* interrupts disabled, may use fast */

    KASSERT(!Interrupts_Enabled());
    KASSERT0(Kernel_Is_Locked(),
             "kernel should be locked, since interrupts are disabled.\n");
    KASSERT(waitQueue != NULL); /* try to protect against passing uninitialized pointers in */

    /* Add the thread to the wait queue. */
    Enqueue_Thread(waitQueue, current);

    /* Find another thread to run. */
    Schedule();
}

void Wake_Up_Locked(struct Thread_Queue *waitQueue) {
    struct Kernel_Thread *kthread = waitQueue->head, *next;

    KASSERT(Is_Locked(&kthreadLock));
    KASSERT(waitQueue != NULL);

    /*
     * Walk throught the list of threads in the wait queue,
     * transferring each one to the run queue.
     */
    while ((kthread =
            Remove_From_Front_Of_Thread_Queue(waitQueue)) != NULL) {
        Make_Runnable(kthread);
    }

    /* The wait queue is now empty. */
}


/*
 * Wake up all threads waiting on given wait queue.
 * Must be called with interrupts disabled!
 * See Keyboard_Interrupt_Handler() function in keyboard.c
 * for an example.
 */
void Wake_Up(struct Thread_Queue *waitQueue) {
    KASSERT(!Interrupts_Enabled());
    KASSERT(waitQueue != NULL);
#ifdef SKETCHY_DEADLOCK_AVOIDANCE
    do {
        if(Try_Spin_Lock(&kthreadLock)) {
            Wake_Up_Locked(waitQueue);
            Spin_Unlock(&kthreadLock);
            return;
        } else {
            Print
                ("failed to acquire kthreadlock while interrupts are disabled; "
                 "yielding to avoid deadlock.\n");
            /* probably yields to the idle thread, since kthreadlock is held. */
            /* make runnable should not be run, since it expects the 
               kthreadlock to be held. */
            Make_Runnable(get_current_thread(0));
            Schedule();
        }
    } while (1);
#else
    Spin_Lock(&kthreadLock);
    Wake_Up_Locked(waitQueue);
    Spin_Unlock(&kthreadLock);
#endif
}


/*
 * Wake up a single thread waiting on given wait queue
 * (if there are any threads waiting).  Chooses the first thread in the queue.
 * Interrupts must be disabled!
 */
void Wake_Up_One(struct Thread_Queue *waitQueue) {
    struct Kernel_Thread *first;

    KASSERT(!Interrupts_Enabled());
    KASSERT(Kernel_Is_Locked());
    KASSERT(waitQueue != NULL);

    first = Remove_From_Front_Of_Thread_Queue(waitQueue);

    if(first != 0) {
        do {
            if(Try_Spin_Lock(&kthreadLock)) {
                Make_Runnable(first);
                /*Print("Wake_Up_One: waking up %x from %x\n", best, CURRENT_THREAD); */
                Spin_Unlock(&kthreadLock);
                return;
            } else {
                Print
                    ("failed to acquire kthreadlock while interrupts are disabled; "
                     "yielding to avoid deadlock in wake_up_one.\n");
                Make_Runnable(get_current_thread(0));
                Schedule();
            }
        } while (1);
    }
}

/*
 * Allocate a key for accessing thread-local data.
 */
int Tlocal_Create(tlocal_key_t * key, tlocal_destructor_t destructor) {
    bool iflag = Begin_Int_Atomic();

    KASSERT(key);

    if(s_tlocalKeyCounter == MAX_TLOCAL_KEYS)
        return -1;
    s_tlocalDestructors[s_tlocalKeyCounter] = destructor;
    *key = s_tlocalKeyCounter++;

    End_Int_Atomic(iflag);

    return 0;
}

/*
 * Store a value for a thread-local item
 */
void Tlocal_Put(tlocal_key_t k, const void *v) {
    const void **pv;

    KASSERT(k < s_tlocalKeyCounter);

    pv = Get_Tlocal_Pointer(k);
    *pv = v;
}

/*
 * Acquire a thread-local value
 */
void *Tlocal_Get(tlocal_key_t k) {
    const void **pv;

    KASSERT(k < s_tlocalKeyCounter);

    pv = Get_Tlocal_Pointer(k);
    return (void *)*pv;
}

/*
 * Print list of all threads in system.
 * For debugging.
 */
void Dump_All_Thread_List(void) {
    struct Kernel_Thread *kthread;
    int count = 0;
    bool iflag = Begin_Int_Atomic();

    Spin_Lock(&kthreadLock);

    kthread = Get_Front_Of_All_Thread_List(&s_allThreadList);

    Print("[");
    while (kthread != 0) {
        ++count;
        Print("<%lx,%lx,%lx>",
              (ulong_t) Get_Prev_In_All_Thread_List(kthread),
              (ulong_t) kthread,
              (ulong_t) Get_Next_In_All_Thread_List(kthread));
        KASSERT(kthread != Get_Next_In_All_Thread_List(kthread));
        kthread = Get_Next_In_All_Thread_List(kthread);
    }
    Print("]\n");
    Print("%d threads are running\n", count);

    Spin_Unlock(&kthreadLock);
    End_Int_Atomic(iflag);
}
