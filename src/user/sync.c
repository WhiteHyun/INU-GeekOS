/*
 * sync - Flush all cached filesystem data to disk
 * Copyright (c) 2004, David H. Hovemeyer <daveho@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.

 * $Revision: 1.3 $
 *
 */

#include <conio.h>
#include <process.h>
#include <fileio.h>

int main(int argc __attribute__ ((unused)), char **argv
         __attribute__ ((unused))) {
    int rc;

    rc = Sync();
    if(rc != 0)
        Print("Could not sync filesystems: %s\n", Get_Error_String(rc));

    return !(rc == 0);
}
