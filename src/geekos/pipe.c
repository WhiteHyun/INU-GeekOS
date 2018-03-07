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
#include <geekos/pipe.h>
#include <geekos/malloc.h>
#include <geekos/string.h>
#include <geekos/errno.h>
#include <geekos/projects.h>
#include <geekos/int.h>


struct File_Ops Pipe_Read_Ops =
    { NULL, Pipe_Read, NULL, NULL, Pipe_Close, NULL };
struct File_Ops Pipe_Write_Ops =
    { NULL, NULL, Pipe_Write, NULL, Pipe_Close, NULL };

int Pipe_Create(struct File **read_file, struct File **write_file) {
    TODO_P(PROJECT_PIPE, "Create a pipe");
    return EUNSUPPORTED;
}

int Pipe_Read(struct File *f, void *buf, ulong_t numBytes) {
    TODO_P(PROJECT_PIPE, "Pipe read");
    return EUNSUPPORTED;
}

int Pipe_Write(struct File *f, void *buf, ulong_t numBytes) {
    TODO_P(PROJECT_PIPE, "Pipe write");
    return EUNSUPPORTED;
}

int Pipe_Close(struct File *f) {
    TODO_P(PROJECT_PIPE, "Pipe close");
    return 0;
}
