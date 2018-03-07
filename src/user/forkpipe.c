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
#include <geekos/errno.h>

int global = 0;

int main(int argc, char **argv) {
    int n = 0;
    int child_pid = 0;
    int i;
    int read_fd, write_fd;
    int read_bytes, written_bytes, pipe_retval;
    char buf[256];


    Print("original pid=%d\n", Get_PID());
    pipe_retval = Pipe(&read_fd, &write_fd);
    assert(pipe_retval == 0);

    child_pid = Fork();

    /* each process should increment these independently */
    n++;
    global ++;

    if(child_pid > 0) {
        Print("parent n=%d, global=%d, child_pid=%d, my_pid=%d\n",
              n, global, child_pid, Get_PID());
        while ((read_bytes = Read(read_fd, buf, 4)) == EWOULDBLOCK) {
            Get_PID();
            Get_PID();
        }
        Print("child read %d bytes\n", read_bytes);
        assert(read_bytes == 4);
        assert(strncmp(buf, "boop", 4) == 0);
        Close(read_fd);

        Wait(child_pid);

        written_bytes = Write(write_fd, "nope", 4);
        assert(written_bytes == EPIPE);
        Close(write_fd);
    } else if(child_pid == 0) {
        Close(read_fd);
        Print("child n=%d, global=%d, child_pid=%d, my_pid=%d\n",
              n, global, child_pid, Get_PID());
        written_bytes = Write(write_fd, "boop", 4);
        if(written_bytes != 4) {
            Print("FAIL: Wrote the wrong number of bytes to fd %d: %d\n",
                  write_fd, written_bytes);
        }
        Close(write_fd);
    } else {
        Print("FAIL: Error in Fork(): %d\n", child_pid);
        return 1;
    }

    return 0;
}
