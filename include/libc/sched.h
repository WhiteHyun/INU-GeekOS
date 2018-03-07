/*
 * Scheduling system calls
 * Copyright (c) 2003, Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2004, David H. Hovemeyer <daveho@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.8 $
 * 
 */

#ifndef SCHED_H
#define SCHED_H

int Set_Scheduling_Policy(int policy, int quantum);
int Get_Time_Of_Day(void);

int Set_Affinity(int pid, int core);
int Get_Affinity(int pid);

int Alarm(unsigned int microSeconds);

#endif /* SCHED_H */
