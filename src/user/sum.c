/*
 * A test program for GeekOS user mode
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

#include <fileio.h>
#include <conio.h>
#include <process.h>

char buffer[1024];

// compute the BSD check sum value
//   code adapted from http://en.wikipedia.org/wiki/BSD_checksum
int main(int argc, char *argv[]) {
    int i;
    int fd;
    int ch;                     /* Each character read. */
    int checksum = 0;           /* The checksum mod 2^16. */
    int count;
    int blocksRead = 0;

    fd = Open(argv[1], O_READ);
    if(fd < 0) {
        Print("ERROR: unnable to open %s\n", argv[1]);
        Exit(-1);
    }
    while (1) {
        count = Read(fd, buffer, 1024);
        if(count <= 0)
            break;
        blocksRead++;
        for(i = 0; i < count; i++) {
            ch = buffer[i];
            checksum = (checksum >> 1) + ((checksum & 1) << 15);
            checksum += ch;
            checksum &= 0xffff; /* Keep it within bounds. */
        }
    }

    Print("%d %d\n", checksum, blocksRead);
    return 0;
}
