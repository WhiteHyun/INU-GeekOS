/*
 * mkdir - Create a directory
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
 * $Revision: 1.10 $
 *
 */

#include <conio.h>
#include <process.h>
#include <fileio.h>

int main(int argc, char *argv[]) {
    int rc;

    if(argc != 2) {
        Print("Usage: mkdir <directory>\n");
        Exit(1);
    }

    rc = Create_Directory(argv[1]);
    if(rc != 0)
        Print("Could not create directory: %s\n", Get_Error_String(rc));

    return !(rc == 0);
}
