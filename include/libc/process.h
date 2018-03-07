/*
 * Process creation and management
 * Copyright (c) 2004 David H. Hovemeyer <daveho@cs.umd.edu>
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
 * $Revision: 1.18 $
 *
 */

#ifndef PROCESS_H
#define PROCESS_H

#include <geekos/user.h>

int Null(void);
/* void declaration and noreturn annotation help with compiler warnings */
void Exit(int exitCode) __attribute__ ((noreturn));
int Spawn_Program(const char *program, const char *command,
                  int background);
int Spawn_With_Path(const char *program, const char *command,
                    const char *path, int background);
int Wait(int pid);
int Get_PID(void);
int PS(struct Process_Info *ptable, int len);
int WaitNoPID(int *status);

int Fork(void);
int Clone(void (*func) (void), void *stack);
int Execl(const char *program, const char *command);
int GetUid();

int ShutDown(void);
#endif /* PROCESS_H */
