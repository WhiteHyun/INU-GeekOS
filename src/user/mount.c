
/*
 * mount - Mount a filesystem on a block device
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
 *
 * $Revision: 1.12 $
 *
 */

#include <conio.h>
#include <process.h>
#include <fileio.h>

int main(int argc, char *argv[]) {
    int ret;

    if(argc != 4) {
        Print("usage: mount <blockdev> <directory> <fstype>\n");
        Exit(1);
    }

    ret = Mount(argv[1], argv[2], argv[3]);
    if(ret != 0) {
        Print("Mount failed: %s\n", Get_Error_String(ret));
        Exit(1);
    }

    return 0;
}
