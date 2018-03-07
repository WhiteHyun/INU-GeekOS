/* A test program for semaphores
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
#include "conio.h"

int main(int argc __attribute__ ((unused)), char **argv
         __attribute__ ((unused))) {
    int semkey, result;

    Print("Open_Semaphore()...\n");
    semkey = Open_Semaphore("semtest", 3);
    Print("Open_Semaphore() returned %d\n", semkey);

    if(semkey < 0)
        return 0;

    Print("P()...\n");
    result = P(semkey);
    Print("P() returned %d\n", result);

    Print("P()...\n");
    result = P(semkey);
    Print("P() returned %d\n", result);

    Print("V()...\n");
    result = V(semkey);
    Print("V() returned %d\n", result);


    Print("Close_Semaphore()...\n");
    result = Close_Semaphore(semkey);
    Print("Close_Semaphore() returned %d\n", result);

    return 0;
}
