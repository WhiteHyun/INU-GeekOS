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
#ifndef _INCLUDED_SEM_H
#define _INCLUDED_SEM_H
#ifdef GEEKOS
int Sys_Open_Semaphore(struct Interrupt_State *state);
int Sys_P(struct Interrupt_State *state);
int Sys_V(struct Interrupt_State *state);
int Sys_Close_Semaphore(struct Interrupt_State *state);
#endif
#endif
