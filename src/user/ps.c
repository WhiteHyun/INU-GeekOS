/*
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

#include <conio.h>
#include <process.h>
#define entries 32

int main(int argc __attribute__((unused)), char **argv
         __attribute__((unused)))
{

    struct Process_Info ptable[entries];
    char state, core, affi;
    int i = 0;
    PS(ptable, entries);
    for (i = 0; i < entries; i++)
    {
        if (ptable[i].pid == 0)
            return 0;
        //setting status code
        if (ptable[i].status == STATUS_RUNNABLE)
        {
            state = 'R';
        }
        else if (ptable[i].status == STATUS_BLOCKED)
        {
            state = 'B';
        }
        else if (ptable[i].status == STATUS_ZOMBIE)
        {
            state = 'Z';
        }
        else
        {
            state = 'U'; //Unknown
        }
        //setting current core
        if (state == 'R')
        {
            core = ptable[i].currCore + '0';
        }
        else
        {
            core = ' ';
        }

        //setting Affinity
        if (ptable[i].affinity == -1)
        {
            affi = 'A';
        }
        else
        {
            affi = ptable[i].affinity + '0';
        }
        // format string for one process line should be "%3d %4d %4d %2c%2c %3c %4d %s\n"
        Print("%3d %4d %4d %2c%2c %3c %4d %s\n", ptable[i].pid, ptable[i].parent_pid, ptable[i].priority, core, state, affi, ptable[i].totalTime, ptable[i].name);
    }

    return 1;
}
