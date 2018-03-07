/*
 * Scheduling system calls
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2004, David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.10 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/syscall.h>
#include <string.h>

DEF_SYSCALL(Set_Scheduling_Policy, SYS_SETSCHEDULINGPOLICY, int,
            (int policy, int quantum), int arg0 = policy;
            int arg1 = quantum;
            , SYSCALL_REGS_2)
DEF_SYSCALL(Get_Time_Of_Day, SYS_GETTIMEOFDAY, int, (void),,
            SYSCALL_REGS_0)

 DEF_SYSCALL(Alarm, SYS_ALARM, int, (unsigned int mseconds),
             unsigned int arg0 = mseconds;
             , SYSCALL_REGS_1)
