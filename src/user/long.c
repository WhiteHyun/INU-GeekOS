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
#include "libuser.h"
#include "process.h"

int main() {
    int i, j;                   /* loop index */
    int scr_sem;                /* id of screen semaphore */
    int now, start, elapsed;

    start = Get_Time_Of_Day();
    scr_sem = Open_Semaphore("screen", 1);      /* register for screen use */

    for(i = 0; i < 200; i++) {
        for(j = 0; j < 10000; j++) ;
        now = Get_Time_Of_Day();
    }
    elapsed = Get_Time_Of_Day() - start;
    P(scr_sem);
    Print("Process Long is done at time: %d\n", elapsed);
    V(scr_sem);


    return 0;
}
