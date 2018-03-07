/*
 * Copyright (c) 2013,2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 */

#ifndef GEEKOS_SMP_H
#define GEEKOS_SMP_H

// max is based on apic structure
#define	MAX_CPUS	256

// kernel visible state per cpu
typedef struct CPU_Info {
    int initDone;
    int spuriousCount;
    char *stack;
    int running;
    int ticks;
    struct Kernel_Thread *idleThread;
    struct User_Context *s_currentUserContext;
} CPU_Info;

extern CPU_Info CPUs[];

int Get_CPU_ID(void);

void Map_IO_APIC_IRQ(int irq, void *handler);
void Init_SMP();
int Init_Local_APIC(int cpu);
void Release_SMP();

struct Kernel_Thread *get_current_thread(int atomic);
#define CURRENT_THREAD  	(get_current_thread(1))

#endif
