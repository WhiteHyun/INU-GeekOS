/*
 * Alarm / timer support
 * Copyright (c) 2009 Calvin Grunewald
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/timer.h>
#include <geekos/alarm.h>
#include <geekos/malloc.h>
#include <geekos/errno.h>
#include <geekos/int.h>
#include <geekos/kthread.h>
#include <geekos/smp.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

Spin_Lock_t alarmLock;

/* #define DEBUG_ALARM(x...) Print(x) */
#define DEBUG_ALARM(x...)

static struct Alarm_Handler_Queue s_alarmWaitingQueue;  /* not yet fired. */
static struct Alarm_Handler_Queue s_alarmPendingQueue;  /* fired, not yet run */
static struct Thread_Queue s_threadQueue;       /* queue for the alarm handler thread */

static inline int Calc_Ticks_Per_MS(int milliseconds) {
    float ticks = TICKS_PER_MS * milliseconds;
    float trunc = (float)(unsigned int)ticks;
    return (((ticks - trunc) > 0) ? trunc + 1 : trunc);
}

struct Alarm_Event *Alarm_Find_In_Queue_By_ID(struct Alarm_Handler_Queue
                                              *queue, int id) {
    struct Alarm_Event *alarm;
    for(alarm = Get_Front_Of_Alarm_Handler_Queue(queue);
        alarm != 0; alarm = Get_Next_In_Alarm_Handler_Queue(alarm)) {
        if(alarm->timerId == id) {
            return alarm;
        }
    }
    return NULL;
}

extern void *_end;              /* can be defined as the last symbol in the code segment. */
static void Alarm_Handler(ulong_t arg __attribute__ ((unused))) {
    struct Alarm_Event *alarm = NULL;
    while (1) {
        Disable_Interrupts();
        //Print("Running alarm handler\n");
        if(!Is_Alarm_Handler_Queue_Empty(&s_alarmPendingQueue)) {

            alarm =
                Remove_From_Front_Of_Alarm_Handler_Queue
                (&s_alarmPendingQueue);
            Enable_Interrupts();

            DEBUG_ALARM("al: %p ", alarm);
            KASSERT0(alarm, "first alarm handler in queue is null");
            KASSERT0((void *)alarm > (void *)0x10,
                     "first alarm handler in queue is near null");
            DEBUG_ALARM("alcb: %p(%p) Core = %d\n", alarm->callback,
                        alarm->data, Get_CPU_ID());
            if(_end) {          /* somehow not being recognized on mac */
                KASSERT0((void *)alarm->callback < _end,
                         "callback not in range");
            }
            alarm->callback(alarm->data);

            Free(alarm);
        } else {
            Wait(&s_threadQueue);
            Enable_Interrupts();
        }
    }
}

static void System_Timer_Callback(int id) {
    KASSERT(!Interrupts_Enabled());
    struct Alarm_Event *alarm =
        Alarm_Find_In_Queue_By_ID(&s_alarmWaitingQueue, id);
    if(alarm == 0)
        return;

    KASSERT0((void *)alarm->callback, "alarm callback was null");
    if(_end) {                  /* somehow not being used */
        KASSERT0((void *)alarm->callback < _end,
                 "to be stored alarm callback not in range");
    }

    Remove_From_Alarm_Handler_Queue(&s_alarmWaitingQueue, alarm);
    Cancel_Timer(id);

    /* ns, unsure whether calvin meant for entries to be added twice, 
       but it would bollocks the list class if it were. */
    if(!Is_Member_Of_Alarm_Handler_Queue(&s_alarmPendingQueue, alarm)) {
        Add_To_Front_Of_Alarm_Handler_Queue(&s_alarmPendingQueue, alarm);
    }
    Wake_Up(&s_threadQueue);
}

void Init_Alarm(void) {
    Start_Kernel_Thread(Alarm_Handler, 0, PRIORITY_NORMAL, false,
                        "{Alarm}");
}

int Alarm_Create(Alarm_Callback callback, void *data,
                 unsigned int milliSeconds) {
    struct Alarm_Event *alarmEvent = Malloc(sizeof(struct Alarm_Event));
    if(alarmEvent == 0)
        return ENOMEM;

    int id;

    alarmEvent->callback = callback;
    alarmEvent->data = data;
    alarmEvent->thread = CURRENT_THREAD;

    Disable_Interrupts();

    id = Start_Timer(Calc_Ticks_Per_MS(milliSeconds),
                     System_Timer_Callback);
    if(id < 0) {
        Enable_Interrupts();
        DEBUG_ALARM("In Alarm_Create, failed to Start_Timer\n");
        return -1;
    }

    alarmEvent->timerId = id;
    // registeredAlarms[id] = alarmEvent;
    Add_To_Front_Of_Alarm_Handler_Queue(&s_alarmWaitingQueue, alarmEvent);
    Enable_Interrupts();
    return id;

}

int Alarm_Cancel_For_Thread(struct Kernel_Thread *thread) {
    struct Alarm_Event *alarm, *next;

    KASSERT(!Interrupts_Enabled());

    for(alarm = Get_Front_Of_Alarm_Handler_Queue(&s_alarmWaitingQueue);
        alarm != 0; alarm = next) {
        next = Get_Next_In_Alarm_Handler_Queue(alarm);
        if(alarm->thread == thread) {
            Remove_From_Alarm_Handler_Queue(&s_alarmWaitingQueue, alarm);
            Enable_Interrupts();
            Free(alarm);
            Disable_Interrupts();
        }
    }

    return 0;
}

int Alarm_Destroy(int id) {
    Disable_Interrupts();

    struct Alarm_Event *alarm =
        Alarm_Find_In_Queue_By_ID(&s_alarmWaitingQueue, id);
    if(alarm) {
        Remove_From_Alarm_Handler_Queue(&s_alarmWaitingQueue, alarm);
        Cancel_Timer(id);
        Enable_Interrupts();
        Free(alarm);
    } else {
        alarm = Alarm_Find_In_Queue_By_ID(&s_alarmPendingQueue, id);
        if(alarm) {
            Remove_From_Alarm_Handler_Queue(&s_alarmPendingQueue, alarm);
            Enable_Interrupts();
            Free(alarm);
        } else {
            Enable_Interrupts();
        }
    }
    return 0;
}
