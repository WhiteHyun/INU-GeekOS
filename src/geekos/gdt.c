/*
 * Initialize kernel GDT.
 * Copyright (c) 2001,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.18 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/kassert.h>
#include <geekos/segment.h>
#include <geekos/int.h>
#include <geekos/tss.h>
#include <geekos/gdt.h>
#include <geekos/smp.h>
#include <geekos/projects.h>
#include <geekos/kthread.h>

/*
 * This is defined in lowlevel.asm.
 */
extern void Load_GDTR(ushort_t * limitAndBase);

/* ----------------------------------------------------------------------
 * Data
 * ---------------------------------------------------------------------- */

/*
 * Number of entries in the kernel GDT.
 * MWH 2/2/2007: bumped up from 16, to allow more processes to run.
 */
#define NUM_GDT_ENTRIES 32

/*
 * This is the kernel's global descriptor table.
 */
static struct Segment_Descriptor s_GDT[8][NUM_GDT_ENTRIES];

/*
 * Number of allocated GDT entries.
 */
static int s_numAllocated = 0;

/* ----------------------------------------------------------------------
 * Functions
 * ---------------------------------------------------------------------- */

/*
 * Allocate a descriptor from the GDT.
 * Returns null if there are none left.
 */
static struct Segment_Descriptor *Allocate_Segment_Descriptor_From(struct
                                                                   Segment_Descriptor
                                                                   *gdt_base) 
{
    struct Segment_Descriptor *result = 0;
    int i;
    bool iflag;

    iflag = Begin_Int_Atomic();

    /* Note; entry 0 is unused (thus never allocated) */
    for(i = 1; i < NUM_GDT_ENTRIES; ++i) {
        struct Segment_Descriptor *desc = &gdt_base[i];
        if(desc->avail) {
            ++s_numAllocated;
            desc->avail = 0;
            result = desc;
            break;
        }
    }

    End_Int_Atomic(iflag);

    return result;
}

struct Segment_Descriptor *Allocate_Segment_Descriptor(void) {
    return Allocate_Segment_Descriptor_From(s_GDT[0]);
}

struct Segment_Descriptor *Allocate_Segment_Descriptor_On_CPU(int cpu) {
    TODO_P(PROJECT_PERCPU, "use the per-cpu GDT");
    return Allocate_Segment_Descriptor_From(s_GDT[0]);
}

/*
 * Free a segment descriptor.
 */
void Free_Segment_Descriptor(struct Segment_Descriptor *desc) {
    bool iflag = Begin_Int_Atomic();

    KASSERT(!desc->avail);

    Init_Null_Segment_Descriptor(desc);
    desc->avail = 1;
    --s_numAllocated;

    End_Int_Atomic(iflag);
}

/*
 * Get the index (int the GDT) of given segment descriptor.
 */
int Get_Descriptor_Index(struct Segment_Descriptor *desc) {
    return ((int)(desc - s_GDT[0]) % NUM_GDT_ENTRIES);  /* ns - maybe */
}


/*
 * Initialize the kernel's GDT.
 */
void Init_GDT(int cpuid) {
    ushort_t limitAndBase[3];
    struct Segment_Descriptor *desc;
    int i;

    KASSERT(sizeof(struct Segment_Descriptor) == 8);

    if(cpuid == 0) {            /* Clear out entries. */
        for(i = 0; i < NUM_GDT_ENTRIES; ++i) {
            desc = &s_GDT[cpuid][i];
            Init_Null_Segment_Descriptor(desc);
            desc->avail = 1;
        }

        /* Kernel code segment. */
        desc = Allocate_Segment_Descriptor_From(s_GDT[cpuid]);
        Init_Code_Segment_Descriptor(desc, 0,   /* base address */
                                     0x100000,  /* num pages (== 2^20) */
                                     0  /* privilege level (0 == kernel) */
            );
        KASSERT(Get_Descriptor_Index(desc) == (KERNEL_CS >> 3));

        /* Kernel data segment. */
        desc = Allocate_Segment_Descriptor_From(s_GDT[cpuid]);
        Init_Data_Segment_Descriptor(desc, 0,   /* base address */
                                     0x100000,  /* num pages (== 2^20) */
                                     0  /* privilege level (0 == kernel) */
            );
        KASSERT(Get_Descriptor_Index(desc) == (KERNEL_DS >> 3));

        TODO_P(PROJECT_PERCPU,
               "Allocate a segment descriptor for the per-cpu region for this cpu.");

    }
    cpuid = 0;                  /* use the cpu 0 GDT (not per cpu) */
    /* Activate the kernel GDT. */
    limitAndBase[0] = sizeof(struct Segment_Descriptor) * NUM_GDT_ENTRIES;
    limitAndBase[1] = ((ulong_t) s_GDT[cpuid]) & 0xffff;
    limitAndBase[2] = ((ulong_t) s_GDT[cpuid]) >> 16;
    Load_GDTR(limitAndBase);
}
