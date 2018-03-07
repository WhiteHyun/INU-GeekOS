/*
 * format - Format a filesystem on a block device
 * Copyright (c) 2008, Aaron Schulman <schulman@cs.umd.edu>
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
 */

#include <conio.h>
#include <process.h>
#include <fileio.h>
#include <geekos/gfs2.h>

int main(int argc, char *argv[]) {
    int rc = -1;

    if(argc != 4) {
        Print("Usage: gfs2f <devname> <size MB> <block size KB>\n");
        Exit(-1);
    }

    /* 
     * TODO Use the ReadBlock and WriteBlock system calls to write to the disk
     * ex. ReadBlock("ide1", buf, sizeof(buf));
     *
     * You will have to implement the ReadBlock and WriteBlock system calls in the kernel.
     *
     */

    if(rc != 0) {
        Print("Error: Could not format gfs2 on %s\n", argv[1]);
        Exit(-1);
    }

    return 0;
}
