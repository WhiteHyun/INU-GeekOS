/*
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
#include <sched.h>
#include <sema.h>
#include <string.h>
#include <fileio.h>

int main(int argc, char **argv) {
    int i;
    int read_fd, write_fd;
    int read_bytes, written_bytes, pipe_retval;
    char buf[256];

    /* Print("calling pipe"); */
    pipe_retval = Pipe(&read_fd, &write_fd);
    assert(pipe_retval == 0);

    for(i = 0; i < 5; i++) {
        written_bytes = Write(write_fd, "beep", 4);
        assert(written_bytes == 4);
        read_bytes = Read(read_fd, buf, 256);
        /* Print("read %d bytes\n", read_bytes); */
        assert(read_bytes == 4);
        assert(strncmp(buf, "beep", 4) == 0);
    }

    Close(write_fd);

    read_bytes = Read(read_fd, buf, 256);
    assert(read_bytes == 0);

    Close(read_fd);

    written_bytes = Write(write_fd, "beep", 4);
    assert(written_bytes <= -1);
    read_bytes = Read(read_fd, buf, 256);
    assert(read_bytes <= -1);

    return 0;
}
