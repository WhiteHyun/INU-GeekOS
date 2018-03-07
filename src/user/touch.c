/*
 * touch - Create an empty file
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
 * $Revision: 1.3 $
 *
 */

#include <conio.h>
#include <fileio.h>
#include <process.h>

static void Print_Error(const char *msg, int err) {
    Print("%s: %s\n", msg, Get_Error_String(err));
    Exit(1);
}

int main(int argc, char **argv) {
    int fd;

    if(argc != 2) {
        Print("Usage: touch <filename>\n");
        Exit(1);
    }

    fd = Open(argv[1], O_CREATE | O_WRITE | O_RECURSIVE);
    if(fd < 0)
        Print_Error("Could not open file", fd);
    if(Close(fd) < 0)
        Print_Error("Could not close file", fd);

    return 0;
}
