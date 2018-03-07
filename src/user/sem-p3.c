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
    int scr_sem, holdp3_sem;
    scr_sem = Open_Semaphore("screen", 1);      /* register for screen use */
    holdp3_sem = Open_Semaphore("holdp3_sem", 0);

    P(holdp3_sem);

    P(scr_sem);
    Print("p3 executed\n");
    V(scr_sem);

    V(holdp3_sem);

    Close_Semaphore(scr_sem);
    Close_Semaphore(holdp3_sem);

    return 0;
}
