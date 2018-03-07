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
#include <geekos/errno.h>

int main(int argc, char **argv) {
    int child_pid = Fork();
    int rc;

    if(child_pid > 0) {
        Print("waiting for  %d\n", child_pid);
        rc = Wait(child_pid);
        Print("The child exited %d\n", rc);
    } else if(child_pid == 0) {
        rc = Execl("/c/b.exe", "b program argument");
        Print("exec failed: %d\n", rc);
    } else {
        Print("fork failed: %s (%d)\n", Get_Error_String(child_pid),
              child_pid);
    }
    return 0;
}
