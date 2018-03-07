/*
 * Trap handlers
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

 * $Revision: 1.21 $
 * 
 */

#include <geekos/idt.h>
#include <geekos/kthread.h>
#include <geekos/defs.h>
#include <geekos/syscall.h>
#include <geekos/trap.h>
#include <geekos/user.h>
#include <geekos/projects.h>
#include <geekos/smp.h>

/*
 * TODO: need to add handlers for other exceptions (such as bounds
 * check, debug, etc.)
 */

/*
 * Handler for general protection faults and other bad errors.
 * Kill the current thread (which caused the fault).
 */
static void GPF_Handler(struct Interrupt_State *state) {
    struct Kernel_Thread *current = CURRENT_THREAD;
    /* Send the thread to the reaper... */
    Print("Exception %d received, killing thread %p (pid %d)\n",
          state->intNum, current, current->pid);
    Dump_Interrupt_State(state);

    if((state->ebp & ~0xfff) != (unsigned long)current->stackPage) {
        /* the following message is OK / informational if it's
           user code that GPF'd. */
        Print("ebp %x is not on the current kernel stack %p\n",
              state->ebp, current->stackPage);
        /* KASSERT(false); */
    }

    Enable_Interrupts();        /* Exit will expects interrupts to be enabled. */
    Exit(-1);

    /* We will never get here */
    KASSERT(false);
}

/*
 * System call handler.
 */
static void Syscall_Handler(struct Interrupt_State *state) {
    /* The system call number is specified in the eax register. */
    uint_t syscallNum;
    struct User_Context *user;

    KASSERT(state);
    syscallNum = state->eax;
    g_preemptionDisabled[Get_CPU_ID()] = false; // ns15


    /* Make sure the the system call number refers to a legal value. */
    if(syscallNum >= g_numSyscalls) {
        Print("Illegal system call %d by process %d\n",
              syscallNum, CURRENT_THREAD->pid);
        Enable_Interrupts();
        Exit(-1);

        /* We will never get here */
        KASSERT(false);
    }

    /*
     * Call the appropriate syscall function.
     * Return code of system call is returned in EAX.
     */
    state->eax = g_syscallTable[syscallNum] (state);
}

/*
 * Initialize handlers for processor traps.
 */
void Init_Traps(void) {
    Install_Interrupt_Handler(12, &GPF_Handler);        /* stack exception */
    Install_Interrupt_Handler(13, &GPF_Handler);        /* general protection fault */
    Install_Interrupt_Handler(SYSCALL_INT, &Syscall_Handler);
}
