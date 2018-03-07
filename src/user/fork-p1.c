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
#include <fileio.h>

int global = 0;

int main(int argc, char **argv) {
    int n = 0;
    int child_pid = 0;
    Print("original\n");
    child_pid = Fork();
    n++;
    global ++;
    if(child_pid > 0) {
        Print("parent n=%d, global=%d, child_pid=%d, my_pid=%d\n", n,
              global, child_pid, Get_PID());
    } else if(child_pid == 0) {
        Print("child n=%d, global=%d, child_pid=%d, my_pid=%d\n", n,
              global, child_pid, Get_PID());
    } else {
        Print("fork failed: %s (%d)\n", Get_Error_String(child_pid),
              child_pid);
    }

    return 0;
}
