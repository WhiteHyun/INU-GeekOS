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
#ifndef GEEKOS_LOCK_H
#define GEEKOS_LOCK_H

typedef struct {
    int lock;
    struct Kernel_Thread *locker;
    struct Kernel_Thread *lastLocker;
} Spin_Lock_t;

#define SPIN_LOCK_INITIALIZER { 0, NULL, NULL }

extern void Spin_Lock_Init(Spin_Lock_t *);
extern int Try_Spin_Lock(Spin_Lock_t *);
extern void Spin_Lock(Spin_Lock_t *);
extern void Spin_Unlock(Spin_Lock_t *);
extern int Is_Locked(Spin_Lock_t *);

#endif // GEEKOS_LOCK_H
