/*
 * cp - Copy a file
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
#include <fileio.h>

int main(int argc, char *argv[]) {
    int ret;
    int read;
    int inFd;
    int outFd;
    struct VFS_File_Stat stat;
    char buffer[1024];

    if(argc != 3) {
        Print("usage: cp <file1> <file2>\n");
        Exit(1);
    }

    inFd = Open(argv[1], O_READ);
    if(inFd < 0) {
        Print("Error: Unable to open %s: %s\n", argv[1],
              Get_Error_String(inFd));
        Exit(1);
    }

    ret = FStat(inFd, &stat);
    if(ret != 0) {
        Print("Error: could not stat file %s: %s\n", argv[1],
              Get_Error_String(ret));
        Exit(1);
    }
    if(stat.isDirectory) {
        Print("Error: cp can not copy directories\n");
        Exit(1);
    }

    /* now open destination file */
    outFd = Open(argv[2], O_WRITE | O_CREATE);
    if(outFd < 0) {
        Print("Error: unable to open %s: %s", argv[2],
              Get_Error_String(outFd));
        Exit(1);
    }

    for(read = 0; read < stat.size; read += ret) {
        ret = Read(inFd, buffer, sizeof(buffer));
        if(ret < 0) {
            Print("Error reading file for copy: %s\n",
                  Get_Error_String(ret));
            Exit(1);
        }

        ret = Write(outFd, buffer, ret);
        if(ret < 0) {
            Print("Error writing file for copy: %s\n",
                  Get_Error_String(ret));
            Exit(1);
        }
    }

    Close(inFd);
    Close(outFd);

    return 0;
}
