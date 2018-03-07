/*
 * Alarm / timer support
 * Copyright (c) 2009 Calvin Grunewald
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.00 $
 *
 */

#ifndef ALARM_H
#define ALARM_H

#include <geekos/defs.h>
#include <geekos/ktypes.h>
#include <geekos/list.h>

typedef void (*Alarm_Callback) (void *);

struct Alarm_Event;

DEFINE_LIST(Alarm_Handler_Queue, Alarm_Event);

struct Alarm_Event {
    int timerId;
    Alarm_Callback callback;
    void *data;
    struct Kernel_Thread *thread;

     DEFINE_LINK(Alarm_Handler_Queue, Alarm_Event);
};

int Alarm_Cancel_For_Thread(struct Kernel_Thread *thread);
int Alarm_Create(Alarm_Callback callback, void *data,
                 unsigned int milliSeconds);
int Alarm_Destroy(int id);
void Init_Alarm(void);

IMPLEMENT_LIST(Alarm_Handler_Queue, Alarm_Event);

#endif
