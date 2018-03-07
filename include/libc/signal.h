/*************************************************************************/
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
/*************************************************************************/
/*
 * User-mode signals
 * Copyright (c) 2005, Michael Hicks <mwh@cs.umd.edu>
 * $Rev$
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#ifndef SIGNAL_H
#define SIGNAL_H

#include <stddef.h>
#include <geekos/signal.h>

int Kill(int pid, int sig);
int Signal(signal_handler h, int sig);

/* For initialization of the signal subsystem */
void Def_Child_Handler(void);
int Sig_Init(void);

#endif /* SIGNAL_H */
