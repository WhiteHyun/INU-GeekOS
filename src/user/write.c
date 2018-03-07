/* A test program for GeekOS user mode
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

int main(int argc __attribute__ ((unused)), char **argv
         __attribute__ ((unused))) {
    Print("Formatting...\n");
    int pid;
    pid = Spawn_With_Path("gfs2f.exe", "gfs2f.exe ide1 10", "/c:/a", 0);
    if(Wait(pid) >= 0) {
        Print("Mounting...\n");
        if(Mount("ide1", "/d", "gosfs") >= 0) {
            Print("Writing...\n");
            int fd = Open("/d/testWrite", O_WRITE | O_CREATE);
            if(fd >= 0) {
                char buffer[100] =
                    "Hello.  If you see this your write works.\n";
                if(Write(fd, buffer, 100) == 100)
                    Print("Wrote file /d/testWrite\n");
            }
            Print("Sync...\n");
            Sync();
            Print("Done sync\n");
        }
    }

    return 0;
}
