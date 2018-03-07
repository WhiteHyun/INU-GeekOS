/*
 * Definition of KASSERT() macro, and other useful debug macros
 * Copyright (c) 2001, David H. Hovemeyer <daveho@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.16 $
 * 
 */

#ifndef GEEKOS_KASSERT_H
#define GEEKOS_KASSERT_H

#include <geekos/screen.h>
// #include <geekos/projects.h>

#ifndef NDEBUG

struct Kernel_Thread;
extern int submitTesting;
extern int Get_CPU_ID(void);
extern void Hardware_Shutdown();
extern struct Kernel_Thread *g_currentThreads[];
extern struct Kernel_Thread *get_current_thread(int atomic);

#ifndef KASSERT
#define KASSERT(cond) 					\
do {							\
    if (!(cond)) {					\
	Set_Current_Attr(ATTRIB(RED, GRAY|BRIGHT));	\
	Print("Failed assertion in %s: %s at %s, line %d, RA=%lx, thread=%p\n",\
		__func__, #cond, __FILE__, __LINE__,	\
		(ulong_t) __builtin_return_address(0),	\
          get_current_thread(0));                \
	if (submitTesting) Hardware_Shutdown();		\
	while (1)					\
	   ; 						\
    }							\
} while (0)
#endif

#ifndef KASSERT0
#define KASSERT0(cond, message)         \
do {							\
    if (!(cond)) {					\
	Set_Current_Attr(ATTRIB(RED, GRAY|BRIGHT));	\
	Print("Failed assertion in %s: %s at %s, line %d, RA=%lx, thread=%p\nNote: %s\n",\
		__func__, #cond, __FILE__, __LINE__,	\
		(ulong_t) __builtin_return_address(0),	\
          get_current_thread(0), message);      \
	if (submitTesting) Hardware_Shutdown();		\
	while (1)					\
	   ; 						\
    }							\
} while (0)
#endif

#define TODO(message)					\
do {							\
    Set_Current_Attr(ATTRIB(BLUE, GRAY|BRIGHT));	\
    Print("Unimplemented feature: %s\n", (message));	\
    if (submitTesting) Hardware_Shutdown();		\
    while (1);						\
    Exit(-1); /* achieve noreturn status */ \
} while (0)

/*
 * Spin for some number of iterations.
 * This is useful for slowing down things that go by too
 * quickly.
 */
#define PAUSE(count)			\
do {					\
    ulong_t i;				\
    for (i = 0; i < (count); ++i)	\
	;				\
} while (0)

#else

/*
 * The debug macros are no-ops when NDEBUG is defined.
 */
#define KASSERT(cond)
#define TODO(message)
#define PAUSE(count)

#endif

/*
 * Stop dead.
 * Its behavior does not depend on whether or not this
 * is a debug build.
 */
#define STOP() while (1)

/*
 * Panic function.
 */
#define Panic(args...)				\
do {						\
    Set_Current_Attr(ATTRIB(RED, GRAY|BRIGHT));	\
    Print(args);				\
    if (submitTesting) Hardware_Shutdown();	\
    while (1) ;					\
} while (0)

#endif /* GEEKOS_KASSERT_H */
