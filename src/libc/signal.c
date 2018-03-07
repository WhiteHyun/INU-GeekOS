/*************************************************************************/
/*
 * GeekOS master source distribution and/or project solution
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2003 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 *
 * This file is not distributed under the standard GeekOS license.
 * Publication or redistribution of this file without permission of
 * the author(s) is prohibited.
 */
/*************************************************************************/
/*
 * Signals
 * Copyright (c) 2005, Michael Hicks <mwh@cs.umd.edu>
 * $Rev$
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/syscall.h>
#include <geekos/signal.h>
#include <fmtout.h>
#include <process.h>
#include <conio.h>

/* System call wrappers */
DEF_SYSCALL(Kill, SYS_KILL, int, (int pid, int sig), int arg0 = pid;
            int arg1 = sig;
            , SYSCALL_REGS_2)
DEF_SYSCALL(Signal, SYS_SIGNAL, int, (signal_handler h, int sig),
            signal_handler arg0 = h;
            int arg1 = sig;
            , SYSCALL_REGS_2)
DEF_SYSCALL(RegDeliver, SYS_REGDELIVER, int, (void (*deliver) (void)),
            void (*arg0) (void) = deliver;
            , SYSCALL_REGS_1)

/* Trampoline for completing a signal handler
*/
extern void Return_Signal(void);

/* may be unused. */
void Def_Child_Handler(void) {
    int status;
    while (WaitNoPID(&status) >= 0) ;
    return;
}

/* Should be called when the program starts up */
int Sig_Init(void) {
    return RegDeliver(Return_Signal);
}
