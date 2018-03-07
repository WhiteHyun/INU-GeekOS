/*
 * String library
 * Copyright (c) 2001,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2013,2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.19 $
 * 
 */

#ifndef STRING_H
#define STRING_H

#include <stddef.h>

void *memset(void *s, int c, size_t n);
void *memcpy(void *dst, const void *src, size_t n);
void *memmove(void *dst, const void *src, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t maxlen);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t limit);
char *strcat(char *s1, const char *s2);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t limit);
char *strdup(const char *s1);
int atoi(const char *buf);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strpbrk(const char *s, const char *accept);

/* Note: The ISO C standard puts this in <stdio.h>, but we don't
 * have that header in GeekOS (yet). */
int snprintf(char *s, size_t size, const char *fmt, ...)
    __attribute__ ((__format__(__printf__, 3, 4)));

int tolower(int c);

#endif /* STRING_H */
