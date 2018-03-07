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
#include <sched.h>
#include <sema.h>
#include <string.h>

int main(int argc __attribute__ ((unused)), char **argv
         __attribute__ ((unused))) {
    int i, j;                   /* loop index */
    int scr_sem;                /* id of screen semaphore */
    int time;                   /* current and start time */
    int ping, pong;             /* id of semaphores to sync processes b & c */

    time = Get_Time_Of_Day();
    scr_sem = Open_Semaphore("screen", 1);      /* register for screen use */
    ping = Open_Semaphore("ping", 1);
    pong = Open_Semaphore("pong", 0);

    for(i = 0; i < 5; i++) {
        P(ping);

        P(scr_sem);
        Print("Pong\n");
        V(scr_sem);

        for(j = 0; j < 35; j++) ;
        V(pong);
    }

    time = Get_Time_Of_Day() - time;
    P(scr_sem);
    Print("Process Pong is done at time: %d\n", time);
    V(scr_sem);





    return (0);
}
