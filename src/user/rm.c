/*
 * rm - delete a file or directory
 * Copyright (c) 2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
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
#include <fileio.h>
#include <process.h>
#include <string.h>

int main(int argc, char **argv) {
    int rc;
    const char *filename;
    bool recursive = false;
    struct VFS_File_Stat stat;

    if(argc == 3 && !strcmp(argv[1], "-r")) {
        recursive = true;
        argv[1] = argv[2];
    } else if(argc != 2) {
        Print("Usage: rm [-r] <filename>\n");
        return 1;
    }

    filename = argv[1];

    rc = Stat(filename, &stat);
    if(rc != 0) {
        Print("File not found %s\n", argv[1]);
        return 1;
    }

    if(!stat.isDirectory || recursive) {
        rc = Delete(argv[1], recursive);
        if(rc < 0) {
            Print("Error deleting %s: %s\n", argv[1],
                  Get_Error_String(rc));
            return 1;
        }
    } else {
        Print("rm %s:can't delete Directory yet\n", filename);
    }

    return 0;
}
