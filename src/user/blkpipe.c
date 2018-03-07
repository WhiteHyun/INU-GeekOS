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
#include <signal.h>
#include <geekos/errno.h>

int global = 0;
int sigpiped = 0;
unsigned char bigbuf[32768];
void sigpipe_handler(int signal) {
    Print("sigpipe taken!\n");
    sigpiped = 1;
}

void test_bigwrite(void) {
    int read_fd, write_fd;
    int pipe_retval;
    int child_pid;
    int child_alive = 1;
    unsigned char littlebuf[256];
    unsigned int i;
    int rc;

    pipe_retval = Pipe(&read_fd, &write_fd);
    assert(pipe_retval == 0);

    for(i = 0; i < sizeof(bigbuf); i++) {
        bigbuf[i] = i;          /* clips. */
    }

    child_pid = Fork();
    assert(child_pid >= 0);

    if(child_pid) {
        int read_bytes;
        int cumulative = 0;
        /* parent */
        Close(write_fd);
        while ((read_bytes = Read(read_fd, littlebuf, sizeof(littlebuf)))) {
            assert(read_bytes == sizeof(littlebuf));
            for(i = 0; i < sizeof(littlebuf); i++) {
                assert(littlebuf[i] == i % 256);
            }
            cumulative += read_bytes;
            Print("%d...\n", cumulative);       /* check that some stuff
                                                   is read before child
                                                   finishes writing, and 
                                                   some stuff after the child finishes writing */
            if(cumulative > 22000 && child_alive) {
                for(i = 0; i < 400 && child_alive; i++) {
                    int exitcode;
                    if(WaitNoPID(&exitcode) >= 0) {
                        child_alive = 0;
                    }
                }
            }
        }
        /* if read zero, that's eof. */
        assert(cumulative == sizeof(bigbuf));
        Close(read_fd);
    } else {
        /* child - does bigwrite */
        Close(read_fd);
        rc = Write(write_fd, bigbuf, sizeof(bigbuf));
        if(rc != sizeof(bigbuf)) {
            Print
                ("child expected to write %lu bytes, but Write returned %d\n",
                 (unsigned long)sizeof(bigbuf), rc);
        } else {
            Print("child returned\n");
        }
        assert(rc == sizeof(bigbuf));
        Close(write_fd);
        Print("child seems happy\n");
        Exit(0);
    }
    Print("parent waiting for child.\n");
    if(child_alive) {
        rc = Wait(child_pid);
    }
    Print("parent seems happy\n");
    Exit(0);
}

void test_littlewrite(void) {
    int read_fd, write_fd;
    int pipe_retval;
    int child_pid;
    unsigned char littlebuf[256];
    unsigned int i;

    pipe_retval = Pipe(&read_fd, &write_fd);
    assert(pipe_retval == 0);

    child_pid = Fork();
    assert(child_pid >= 0);

    if(child_pid) {
        int read_bytes;
        int cumulative = 0;
        /* parent */
        Close(write_fd);
        Print("parent reading...\n");
        while ((read_bytes =
                Read(read_fd, bigbuf + cumulative,
                     sizeof(littlebuf))) > 0) {
            cumulative += read_bytes;
        }
        if(read_bytes < 0) {
            if(read_bytes == EWOULDBLOCK) {
                Print
                    ("parent read returned EWOULDBLOCK, suggesting this kernel does not support blocking.\n");
            } else {
                Print("parent read encountered error %d\n", read_bytes);
            }
        }
        /* if read zero, that's eof. */
        assert(cumulative == sizeof(bigbuf));
        for(i = 0; i < sizeof(bigbuf); i++) {
            assert(bigbuf[i] == i % 256);
        }
        Close(read_fd);
    } else {
        int rc;
        unsigned char c;
        /* child - does write */
        Close(read_fd);
        Print("child writing...\n");
        for(i = 0; i < sizeof(bigbuf); i++) {
            c = i;
            rc = Write(write_fd, &c, 1);
            assert(rc == 1);
        }
        Close(write_fd);
        Print("child seems happy\n");
        Exit(0);
    }
    Print("parent waiting for child.\n");
    Wait(child_pid);
    Print("parent seems happy\n");
    Exit(0);
}

void test_dualwrite(void) {
    int read_fd, write_fd;
    int pipe_retval;
    int child_pid;
    unsigned char littlebuf[256];
    unsigned int i;

    pipe_retval = Pipe(&read_fd, &write_fd);
    assert(pipe_retval == 0);

    child_pid = Fork();
    assert(child_pid >= 0);

    if(child_pid) {
        int read_bytes;
        int cumulative = 0;
        /* parent */
        Close(write_fd);
        while ((read_bytes =
                Read(read_fd, bigbuf + cumulative, sizeof(littlebuf)))) {
            assert(read_bytes > 0);
            cumulative += read_bytes;
        }
        /* if read zero, that's eof. */
        if(cumulative != sizeof(bigbuf)) {
            Print("read only %d bytes instead of desired %lu\n",
                  cumulative, (unsigned long)sizeof(bigbuf));
        }
        assert(cumulative == sizeof(bigbuf));
        Close(read_fd);
    } else {

        Close(read_fd);
        int grandchild_pid = Fork();
        /* now two children write. */

        int rc;
        unsigned char c;
        /* child and grandchild - writes */
        for(i = 0; i < sizeof(bigbuf) / 2; i++) {
            c = i;
            rc = Write(write_fd, &c, 1);
            assert(rc == 1);
        }
        Close(write_fd);
        if(grandchild_pid) {
            Wait(grandchild_pid);
            Print("child seems happy\n");
        } else {
            Print("grandchild seems happy\n");
        }
        Exit(0);
    }
    Wait(child_pid);
    Print("parent seems happy\n");
    Exit(0);
}

int main(int argc, char **argv) {
    int n = 0;
    int child_pid = 0;
    int i;
    int read_fd, write_fd;
    int read_bytes, written_bytes, pipe_retval;
    char buf[256];

    int mode = 0;

    if(argc > 1) {
        mode = atoi(argv[1]);
        Print("mode: %d\n", mode);
    }

    switch (mode) {
        case 0:
            test_bigwrite();
            break;
        case 1:
            test_littlewrite();
            break;
        case 2:
            test_dualwrite();
            break;
        default:
            Print("unknown test %d\n", mode);
    }

    pipe_retval = Pipe(&read_fd, &write_fd);
    assert(pipe_retval == 0);

    child_pid = Fork();

    /* each process should increment these independently */
    n++;
    global ++;

    if(child_pid > 0) {
        /* am the parent, let's read. */
        int total_read_bytes = 0;
        Print("parent n=%d, global=%d, child_pid=%d, my_pid=%d\n",
              n, global, child_pid, Get_PID());
        /* this component (read while > 0) only works when the pipe is blocking,  */
        while ((read_bytes = Read(read_fd, bigbuf, 2048)) > 0 &&
               total_read_bytes < 16384) {
            Print("parent read %d bytes\n", read_bytes);
            total_read_bytes += read_bytes;
        }
        Close(read_fd);

        Signal(sigpipe_handler, SIGPIPE);

        Wait(child_pid);

        written_bytes = Write(write_fd, "nope", 4);
        if(written_bytes != EPIPE) {
            Print("wrote %d bytes into a closed pipe\n", written_bytes);
        }
        assert(written_bytes == EPIPE);
        assert(sigpiped);
        Close(write_fd);
    } else if(child_pid == 0) {
        unsigned long totally_written = 0;
        Close(read_fd);
        Print("child n=%d, global=%d, child_pid=%d, my_pid=%d\n",
              n, global, child_pid, Get_PID());
        for(i = 0; i < 256; i++)
            buf[i] = i;
        /* intended mostly for testing a blocking pipe, but should give ample opportunity to 
           sigpipe the child if the pipe is not yet blocking */
        for(totally_written = 0; totally_written <= 16384;
            totally_written += sizeof(buf)) {
            written_bytes = Write(write_fd, buf, sizeof(buf));
        }
        Print("Good so far, child could write a lot.");
        for(totally_written = 0; totally_written <= 8192;
            totally_written += sizeof(buf)) {
            written_bytes = Write(write_fd, buf, sizeof(buf));
        }
        written_bytes = Write(write_fd, buf, sizeof(buf));
        /* shouldn't get here */

        if(written_bytes != EPIPE) {
            Print("FAIL: Didn't even epipe.");
        } else {
            Print("FAIL: Didn't sigpipe to kill the child.");
        }
        Close(write_fd);
    } else {
        Print("FAIL: Error in Fork(): %d\n", child_pid);
        return 1;
    }

    return 0;
}
