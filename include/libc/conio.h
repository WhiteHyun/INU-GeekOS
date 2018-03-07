/*
 * User-mode Console I/O
 * Copyright (c) 2003, David H. Hovemeyer <daveho@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.20 $
 * 
 */

#ifndef CONIO_H
#define CONIO_H

#include <stddef.h>
#include <geekos/ktypes.h>
#include <geekos/keyboard.h>    /* key codes */
#include <geekos/screen.h>      /* key codes */

void Print(const char *fmt, ...) __attribute__ ((format(printf, 1, 2)));
int Print_String(const char *msg);

#ifndef GEEKOS
int Put_Char(int ch);
int Get_Key(void);
int Set_Attr(int attr);
int Get_Cursor(int *row, int *col);
int Put_Cursor(int row, int col);
#endif

void Echo(bool enable);
void Read_Line(char *buf, size_t bufSize);
const char *Get_Error_String(int errno);

#ifndef assert
#define assert(exp)						\
do {								\
    if (!(exp)) {						\
      extern void Exit(int) __attribute__((noreturn));  \
	Print("\x1b[1;37;41m"					\
	    "Failed assertion: %s: at %s, line %d\x1B[37;40m\n",\
	    #exp, __FILE__, __LINE__);				\
	Exit(1);						\
    }								\
} while (0)
#endif


#endif /* CONIO_H */
