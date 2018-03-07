/*
 * A test program for GeekOS user mode
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
#include <geekos/syscall.h>

int main() {
    int badsys = -1, rc;

    Print_String("I am the c program\n");

    /* Make an illegal system call */
    __asm__ __volatile__(SYSCALL:"=a"(rc)
                         :"a"(badsys)
        );

    return 0;
}
