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
/* Don't print an informative message when running an
   unimplemented function from a project with an incomplete
   TODO */
#include <geekos/screen.h>
#define PROJECT_VERBOSITY false
/* traditional project 0 */
#define PROJECT_LIMIT_SYSCALLS false
#define PROJECT_LIMIT_ACTIVE_PROCESSES false
/* traditional project 1 */
#define PROJECT_BACKGROUND_JOBS false
/* traditional project 2 */
#define PROJECT_SIGNALS false
/* traditional project 3 */
#define PROJECT_SEMAPHORES false
#define PROJECT_SCHEDULING false
/* traditional project 4 */
#define PROJECT_VIRTUAL_MEMORY_A false
#define PROJECT_VIRTUAL_MEMORY_B false

/* any file system project */
/* can enable multiple file systems at once */
#define PROJECT_FS false
/* neil's version of project 5 */
#define PROJECT_GFS2 false
/* neil's second version of project 5 */
#define PROJECT_GFS3 false
/* sound driver */
#define PROJECT_SOUND false
/* Jeff's traditional project #5 */
#define PROJECT_GOSFS false
/* New Camelion file system project #5 */
#define PROJECT_CFS false

#define PROJECT_PERCPU false

#define PROJECT_USER false

// Spring 2014 Project #0
#define PROJECT_LIMIT	false
// Spring 2014 Project #2
#define PROJECT_CLONE	false
// Spring 2014 Project #4
#define PROJECT_MMAP	false

/* networking projects */
#define PROJECT_ETHERNET false
#define PROJECT_TCP false
#define PROJECT_RIP false
#define PROJECT_SOCKETS false
#define PROJECT_RAW_ETHERNET false
#define PROJECT_IP false
#define PROJECT_UDP false
#define PROJECT_TCP false
#define PROJECT_ROUTING false
#define PROJECT_ARP false

/* revised intro projects */
#define PROJECT_PIPE false
#define PROJECT_FORK false

#define PROJECT_SERIAL false
#define PROJECT_MALLOC false

#ifndef TODO
#ifdef GEEKOS
#warning "projects.h file requires geekos/screen.h to be included first in kernel source files to define TODO"
#else
#include <conio.h>
#define TODO(x)  do { \
	Set_Current_Attr(ATTRIB(BLUE, GRAY|BRIGHT)); \
	Print(x); \
	while(1); \
} while(0);
#endif
#endif

#define TODO_P(proj, message)  do  { \
    if(proj) { Print(__FILE__ ": " message); }  \
    else if(PROJECT_VERBOSITY) { Print("Invoked function of project: " #proj ": " message "\n"); } \
   } while(0)
