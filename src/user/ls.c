/*
 * ls - List file or directory
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
#include <fileio.h>
#include <process.h>

static void List_File(const char *filename, struct VFS_File_Stat *stat) {
    struct VFS_ACL_Entry owner = stat->acls[0];
    bool isDir = stat->isDirectory;

    Print("%c%c%c  % 6d  % 10d  %s%s%s\n",
          isDir ? 'd' : '-',
          owner.permission & O_READ ? 'r' : '-',
          owner.permission & O_WRITE ? 'w' : '-',
          owner.uid,
          stat->size, isDir ? "[" : "", filename, isDir ? "]" : "");
}

int main(int argc, char **argv) {
    int rc;
    struct VFS_File_Stat stat;
    const char *filename;

    if(argc != 2) {
        Print("Usage: ls <filename>\n");
        return 1;
    }

    filename = argv[1];

    rc = Stat(filename, &stat);
    if(rc != 0) {
        Print("Could not stat %s: %s\n", argv[1], Get_Error_String(rc));
        return 1;
    }

    if(!stat.isDirectory) {
        List_File(argv[1], &stat);
    } else {
        int fd = Open_Directory(argv[1]);
        struct VFS_Dir_Entry dirent;

        if(fd < 0) {
            Print("Could not open %s: %s\n", filename,
                  Get_Error_String(fd));
            Exit(1);
        }

        Print("Directory %s\n", filename);
        for(;;) {
            int r = Read_Entry(fd, &dirent);
            if(r > 0)
                break;
            else if(r == 0)
                List_File(dirent.name, &dirent.stats);
            else {
                Print("Could not read directory entry: %s\n",
                      Get_Error_String(r));
                Exit(1);
            }
        }

        if((rc = Close(fd)) < 0) {
            Print("Could not close directory %s: %s\n", filename,
                  Get_Error_String(rc));
            Exit(1);
        }
    }

    return 0;
}
