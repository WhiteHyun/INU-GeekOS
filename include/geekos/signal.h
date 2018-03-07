/*************************************************************************/
/*
 * GeekOS master source distribution and/or project solution
 * Copyright (c) 2005 Michael Hicks <mwh@cs.umd.edu>
 * Copyright (c) 2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 */
/*************************************************************************/

#ifndef GEEKOS_SIGNAL_H
#define GEEKOS_SIGNAL_H

/* Signal numbers */
#define SIGKILL  1              /* can't be handled by users */
#define SIGUSR1  2
#define SIGUSR2  3
#define SIGCHLD  4
#define SIGALARM  5
#define SIGPIPE  6

/* The largest signal number supported */
#define MAXSIG   6

/* Macro to determine whether a number is a valid signal number */
#define IS_SIGNUM(n) (((n) > 0) && ((n) <= MAXSIG))

/* Definition of a signal handler */
typedef void (*signal_handler) (int);

/* Default handlers */
#define SIG_DFL  (signal_handler)1
#define SIG_IGN  (signal_handler)2

#ifdef GEEKOS

struct Interrupt_State;

int Check_Pending_Signal(struct Kernel_Thread *kthread,
                         struct Interrupt_State *esp);
void Complete_Handler(struct Kernel_Thread *kthread,
                      struct Interrupt_State *esp);

#endif

#endif
