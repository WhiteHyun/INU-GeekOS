/*
 * x86 TSS data structure and routines
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

/*
 * Source: _Protected Mode Software Architecture_ by Tom Shanley,
 * ISBN 020155447X.
 */

#include <geekos/kassert.h>
#include <geekos/defs.h>
#include <geekos/gdt.h>
#include <geekos/segment.h>
#include <geekos/string.h>
#include <geekos/tss.h>
#include <geekos/smp.h>

/*
 * We use one TSS in GeekOS.
 */
static struct TSS s_theTSS[MAX_CPUS];
static struct Segment_Descriptor *s_tssDesc[MAX_CPUS];
static ushort_t s_tssSelector[MAX_CPUS];

__inline__ void Load_Task_Register(void) {
    int cpu = Get_CPU_ID();

    KASSERT(s_tssDesc[cpu]);

    /* Critical: TSS must be marked as not busy */
    s_tssDesc[cpu]->type = 0x09;

    /* Load the task register */
    __asm__ __volatile__("ltr %0"::"a"(s_tssSelector[cpu])
        );
}

/*
 * Initialize the kernel TSS.  This must be done after the memory and
 * GDT initialization, but before the scheduler is started.
 */
void Init_TSS(void) {
    int cpu = Get_CPU_ID();

    s_tssDesc[cpu] = Allocate_Segment_Descriptor_On_CPU(cpu);
    KASSERT(s_tssDesc[cpu] != 0);

    memset(&s_theTSS[cpu], '\0', sizeof(struct TSS));
    Init_TSS_Descriptor(s_tssDesc[cpu], &s_theTSS[cpu]);

    s_tssSelector[cpu] =
        Selector(0, true, Get_Descriptor_Index(s_tssDesc[cpu]));

    Load_Task_Register();
}

/*
 * Set kernel stack pointer.
 * This should be called before switching to a new
 * user process, so that interrupts occurring while executing
 * in user mode will be delivered on the correct stack.
 */
void Set_Kernel_Stack_Pointer(ulong_t esp0) {
    int cpu = Get_CPU_ID();

    s_theTSS[cpu].ss0 = KERNEL_DS;
    s_theTSS[cpu].esp0 = esp0;

    /*
     * NOTE: I read on alt.os.development that it is necessary to
     * reload the task register after modifying a TSS.
     * I haven't verified this in the IA32 documentation,
     * but there is certainly no harm in being paranoid.
     */
    Load_Task_Register();
}

int submitTesting;

/*
 * checkPaging function here, in a file that should not be
 * modified, so that test code can rely on it being unmodified.
 */

int checkPaging(void) {
    unsigned long reg = 0;
    int paging_on;
    __asm__ __volatile__("movl %%cr0, %0":"=a"(reg));
    paging_on = (reg & (1 << 31)) != 0;
    Print("Paging on ? : %d\n", paging_on);
    return paging_on;
}
