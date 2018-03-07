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

#if !defined (NULL)
#define NULL 0
#endif

int main(int argc __attribute__ ((unused)), char **argv
         __attribute__ ((unused))) {
    int scr_sem, holdp3_sem;    /* sid of screen semaphore */
    int id1, id2, id3;          /* ID of child process */

    holdp3_sem = Open_Semaphore("holdp3_sem", 0);
    scr_sem = Open_Semaphore("screen", 1);


    P(scr_sem);
    Print("Semtest1 begins\n");
    V(scr_sem);


    id3 = Spawn_Program("/c/p3.exe", "/c/p3.exe", 0);
    P(scr_sem);
    Print("p3 created\n");
    V(scr_sem);
    id1 = Spawn_Program("/c/p1.exe", "/c/p2.exe", 0);
    id2 = Spawn_Program("/c/p2.exe", "/c/p1.exe", 0);


    Wait(id1);
    Wait(id2);
    Wait(id3);

    Close_Semaphore(scr_sem);
    Close_Semaphore(holdp3_sem);
    return 0;
}
