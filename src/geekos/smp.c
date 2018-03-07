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

#include <geekos/string.h>
#include <geekos/screen.h>
#include <geekos/kassert.h>
#include <geekos/apic.h>
#include <geekos/smp.h>
#include <geekos/list.h>
#include <geekos/int.h>
#include <geekos/idt.h>
#include <geekos/malloc.h>
#include <geekos/kthread.h>
#include <geekos/io.h>
#include <geekos/timer.h>
#include <geekos/tss.h>
#include <geekos/trap.h>
#include <geekos/mem.h>
#include <geekos/gdt.h>
#include <geekos/kassert.h>
#include <geekos/projects.h>

/*
 * Information on Intel MP spec from:
 *	http://www.osdever.net/tutorials/view/multiprocessing-support-for-hobby-oses-explained
 *    
 */

/*
 * cpuid code from http://wiki.osdev.org/CPUID
 */
enum cpuid_requests {
    CPUID_GETVENDORSTRING,
    CPUID_GETFEATURES,
    CPUID_GETTLB,
    CPUID_GETSERIAL,

    CPUID_INTELEXTENDED = 0x80000000,
    CPUID_INTELFEATURES,
    CPUID_INTELBRANDSTRING,
    CPUID_INTELBRANDSTRINGMORE,
    CPUID_INTELBRANDSTRINGEND,
};

/*
 *  issue a single request to CPUID. Fits 'intel features', for instance
 *  note that even if only "eax" and "edx" are of interest, other registers
 *  will be modified by the operation, so we need to tell the compiler about it.
 */
static inline void cpuid(int code, int *a, int *d) {
    asm volatile ("cpuid":"=a" (*a), "=d"(*d):"a"(code):"ecx", "ebx");
}

/* address of local APIC - generally at the default address */
static char *APIC_Addr = (char *)0xFEE00000;

/* address of IO APIC - generally at the default address */
static volatile unsigned int *IO_APIC_Addr =
    (volatile unsigned int *)0xFEC00000;

static Spin_Lock_t globalLock;

/*
 * These next few data structures are defined as part of the Intel MP spec.
 *   See - http://www.intel.com/design/archives/processors/pro/docs/242016.htm
 *
 */
typedef struct MP_Config_Table {
    char signature[4];
    short length;
    char rev;
    char checksum;
    int OEM_Low;
    int OEM_High;
    char product_ID[12];
    void *OEM_Table_Ptr;
    short OEM_Table_Size;
    short entry_Count;
    void *local_APIC_Addr;
    short extended_Table_Length;
    char extended_Table_Checksum;
} MP_Config_Table;

typedef struct MP_Floating_Table {
    char signature[4];
    MP_Config_Table *MP_Config_PTR;
    char length;
    char version;
    char checksum;
    char MP_Features1;
    char MP_Features2;
    char MP_Reserved1;
    char MP_Reserved2;
    char MP_Reserved3;
} MP_Floating_Table;

enum mp_table_types {
    MP_CONFIG_ENTRY_PROCESSOR = 0,
    MP_CONFIG_ENTRY_BUS,
    MP_CONFIG_ENTRY_IO_APIC,
    MP_CONFIG_ENTRY_IO_INTERRUPT_ASSIGNMENT,
    MP_CONFIG_ENTRY_LOCAL_INTERRUPT_ASSIGNMENT
};

typedef struct MP_Processor {
    char type;
    char APIC_Id;
    char APIC_Version;
    char CPU_Enabled:1;
    char Is_Bootstrap_CPU:1;
    int CPU_Signature;
    int CPU_Flags;
    int reserved1;
    int reserved2;
} MP_Processor;

typedef struct MP_IO_APIC {
    char type;
    char APIC_Id;
    char APIC_Version;
    char enabled:1;
    void *address;
} MP_IO_APIC;

typedef struct MP_IO_Interrupt {
    unsigned char entry_Type;   // should be 3
    unsigned char interrupt_Type;
    unsigned short flag_PO:1;
    unsigned short flag_EL:1;
    unsigned short reserved:14;
    unsigned char bus_ID;
    unsigned char src_Bus_IRQ;
    unsigned char destIO_APIC;
    unsigned char destIO_APIC_IRQ;
} MP_IO_Interrupt;

int CPU_Count;
MP_Processor *Processor;

/* compute byte total of a length byte region begining at start - if it is zero checksum is OK */
static int MP_Checksum(unsigned char *start, int length) {
    int i;
    unsigned int total;

    total = 0;
    for(i = 0; i < length; i++)
        total += start[i];

    return total & 0xff;
}

static MP_Floating_Table *Scan_For_Floating_Table(int *start, int *end) {
    int *curr;
    MP_Floating_Table *try;
    for(curr = start; curr < end; curr++) {
        try = (MP_Floating_Table *) curr;
        if(!strncmp(try->signature, "_MP_", 4) &&
           !MP_Checksum((unsigned char *)curr,
                        sizeof(MP_Floating_Table))) {
            return try;
        }
    }
    return 0;
}

static int Get_MP_Tables() {

    MP_Config_Table *ct;
    MP_Floating_Table *ft;

    /* First scan to find MP Floating table */
    ft = Scan_For_Floating_Table((int *)0xEBDA, (int *)0xEBDA + 1024);
    if(!ft) {
        /* not found, look at 639KB to 640KB */
        ft = Scan_For_Floating_Table((int *)(639 * 1024),
                                     (int *)(640 * 1024));
    }
    if(!ft) {
        /* still not found, look in BIOS ROM 0xf0000 to 0x100000 */
        ft = Scan_For_Floating_Table((int *)0xF0000, (int *)0x100000);
    }

    if(!ft || !ft->MP_Config_PTR) {
        /* give up and assume we are a single core/cpu system */
        return 0;
    }

    ct = ft->MP_Config_PTR;
    if(strncmp(ct->signature, "PCMP", 4) ||
       MP_Checksum((unsigned char *)ct + ct->length,
                   ct->extended_Table_Length)) {
        /* Signature not as expected, just give up */
        Print("MP Configuration Table not valid\n");
        return 0;
    }

    char *curr = ((char *)ct) + sizeof(MP_Config_Table);
    MP_Processor *proc;
    MP_IO_APIC *ioAPIC;
    MP_IO_Interrupt *intAss;
    int i;
    for(i = 0; i < ct->entry_Count; i++) {
        switch (*curr) {
            case MP_CONFIG_ENTRY_PROCESSOR:
                proc = (MP_Processor *) curr;
                Print("found CPU#%d with APIC id #%d\n", CPU_Count,
                      proc->APIC_Id);
                /* Processors are grouped together, so the first is start of array */
                if(!CPU_Count)
                    Processor = proc;
                CPU_Count++;
                curr += sizeof(MP_Processor);
                break;

            case MP_CONFIG_ENTRY_IO_APIC:
                ioAPIC = (MP_IO_APIC *) curr;
                Print("found IO APIC ID=%d at %x\n", ioAPIC->APIC_Id,
                      (unsigned int)ioAPIC->address);
                IO_APIC_Addr = ioAPIC->address;
                curr += sizeof(MP_IO_APIC);
                break;

            case MP_CONFIG_ENTRY_LOCAL_INTERRUPT_ASSIGNMENT:
            case MP_CONFIG_ENTRY_IO_INTERRUPT_ASSIGNMENT:
                intAss = (MP_IO_Interrupt *) curr;
                curr += 8;
                break;

            case MP_CONFIG_ENTRY_BUS:
                /* skip over records we don't care about */
                curr += 8;
                break;

            default:
                Print("Unknown entry type %d\n", *curr);
        }
    }
    return 1;
}

inline int APIC_Read(int reg) {
    int ret;

    __asm("pushfl");
    __asm("cli");               // disable interrupts
    ret = *((volatile unsigned int *)(APIC_Addr + reg));
    __asm("popfl");             // enable interrupts if previously enabled

    return ret;
}

inline void APIC_Write(int reg, unsigned int value) {
    __asm("pushfl");
    __asm("cli");               // disable interrupts
    *((volatile unsigned int *)(APIC_Addr + reg)) = value;
    __asm("popfl");             // enable interrupts if previously enabled

}

inline void IOAPIC_Write(const int offset, const int val) {
    __asm("pushfl");
    __asm("cli");               // disable interrupts
    /* tell IOREGSEL where we want to write to */
    *IO_APIC_Addr = offset;
    /* write the value to IOWIN */
    *(unsigned int *)(IO_APIC_Addr + 0x10 / 4) = val;
    __asm("popfl");             // enable interrupts if previously enabled
}

inline int IOAPIC_Read(const unsigned char offset) {
    int ret;

    __asm("pushfl");
    __asm("cli");               // disable interrupts
    *(unsigned int *)IO_APIC_Addr = offset;
    ret = *(unsigned int *)(IO_APIC_Addr + 0x10 / 4);
    __asm("popfl");             // enable interrupts if previously enabled

    return (ret);
}

/*
 * Send an interprocessor interrupt to the processor associated with APIC_Id.
 *
 */
static int send_IPI(int APIC_Id, int mask) {
    int try;
    int status = 1;

    APIC_Write(APIC_ICR + 0x10, (APIC_Id << 24));
    APIC_Write(APIC_ICR, mask);

    for(try = 0; try < 100; try++) {
        Micro_Delay(100);
        status = APIC_Read(APIC_ICR) & APIC_ICR_STATUS_PEND;
        if(!status)
            return 0;
    }

    Print("Send IPI timeout\n");
    return (1);
}

static void send_INIT(int APIC_Id) {
    /* assert INIT IPI */
    send_IPI(APIC_Id,
             APIC_ICR_TM_LEVEL | APIC_ICR_LEVELASSERT | APIC_ICR_DM_INIT);

    Micro_Delay(10000);

    /* de-assert INIT IPI */
    send_IPI(APIC_Id, APIC_ICR_TM_LEVEL | APIC_ICR_DM_INIT);

    Micro_Delay(10000);
}

static void Spurious_Interrupt_Handler(struct Interrupt_State *state) {
    int CPUid = Get_CPU_ID();
    CPUs[CPUid].spuriousCount++;
}

//
// Code adapted from http://wiki.osdev.org/APIC_timer#Enabling_APIC_Timer
//    setup local APIC including calibrating its timer register
//
int Init_Local_APIC(int cpu) {
    int apicid;
    unsigned int cpubusfreq;
    unsigned int tmp;
    int quantum = 10;
    static int apicInitialCount = 0;

    extern void Timer_Interrupt_Handler();
    Install_Interrupt_Handler(39, Spurious_Interrupt_Handler);
    // only one global set of timer handlers
    Install_Interrupt_Handler(32, Timer_Interrupt_Handler);

    // init apic to known state
    APIC_Write(APIC_DFR, 0xFFFFFFFF);
    APIC_Write(APIC_LDR, (APIC_Read(APIC_LDR) & 0x00FFFFFF) | 1);
    APIC_Write(APIC_LVTT, APIC_DISABLE);
    APIC_Write(APIC_LVTPC, APIC_ICR_DM_NMI);
    APIC_Write(APIC_LVT0, APIC_DISABLE);
    APIC_Write(APIC_LVT1, APIC_DISABLE);
    APIC_Write(APIC_TPR, 0);

    // enable APIC 
    asm("movl $0x1b, %ecx");
    asm("rdmsr");
    asm("orl  $0x800, %eax");
    asm("wrmsr");

    // Set the Spourious Interrupt Vector Register bit 8 to start receiving interrupts 
    apicid = APIC_Read(APIC_SPIV);
    APIC_Write(APIC_SPIV, apicid | APIC_SPIV_ENABLE_APIC);
    apicid = GET_APIC_ID(APIC_Read(APIC_ID));

    // enable spurious  - required for APIC to work
    APIC_Write(APIC_SPIV, 39 | APIC_SW_ENABLE);

    if(!cpu) {
        // make timer a one shot
        APIC_Write(APIC_LVTT, 32);

        APIC_Write(APIC_TDCR, 0x03);

        Out_Byte(0x61, (In_Byte(0x61) & 0xFD) | 1);
        Out_Byte(0x43, 0xB2);   // Switch to Mode 0, if you get problems!!!
        //1193180/100 Hz = 11931 = 2e9bh
        Out_Byte(0x42, 0x9B);   //LSB
        In_Byte(0x60);          //short delay
        Out_Byte(0x42, 0x2E);   //MSB

        //reset PIT one-shot counter (start counting)
        tmp = In_Byte(0x61) & 0xFE;
        Out_Byte(0x61, (unsigned char)tmp);     //gate low
        Out_Byte(0x61, (unsigned char)tmp | 1); //gate high

        //reset APIC timer (set counter to -1)
        APIC_Write(APIC_TICR, 0xFFFFFFFF - 1);

        //now wait until PIT counter reaches zero
        tmp = -1;
        while (!(In_Byte(0x61) & 0x20) && --tmp) ;
        if(tmp == 0) {
            Print("PIT failed to decrement in APIC initialization\n");
        }
        //stop APIC timer
        APIC_Write(APIC_LVTT, APIC_DISABLE);

        //now do the math...
        Print("apic counted down to %x\n", APIC_Read(APIC_TCCR));
        cpubusfreq = (0xFFFFFFFF - (APIC_Read(APIC_TCCR) + 1)) * 16 * 100;
        Print("cpu freq = %d\n", cpubusfreq);
        apicInitialCount = cpubusfreq / quantum / 16;

        // sanity check, now tmp holds appropriate number of ticks, use it as APIC timer counter initializer
    }

    APIC_Write(APIC_TICR, apicInitialCount < 16 ? 16 : apicInitialCount);

    // finally re-enable timer in periodic mode
    APIC_Write(APIC_LVTT, 32 | TMR_PERIODIC);

    // setting divide value register again not needed by the manuals
    // although I have found buggy hardware that required it
    APIC_Write(APIC_TDCR, 0x03);

    return apicid;
}

// This is really the apic, id but is often the same as the cpuid
int Get_CPU_ID(void) {
    int apicid;

    apicid = GET_APIC_ID(APIC_Read(APIC_ID));

    return apicid;
}

CPU_Info CPUs[MAX_CPUS];

/* 
 * START_SECONDARY_FUNC Must be updated if you change:
 *		STARTSEG in defs.asm
 *		alignment before start_secondary_cpu in setup.asm 
 * SECONDARY_STACK is the address to store the initial value of the stack
 */
#define START_SECONDARY_FUNC	((0x9020<<4) + 4096)

void *Secondary_Stack;

void Init_SMP(void) {
    int i;
    int irq;
    int count;
    int apicid;

    Print("Initializing SMP...\n");

    Get_MP_Tables();
    apicid = Get_CPU_ID();

    KASSERT0(apicid == 0,
             "After local APIC init, APIC is not expected value");

    for(i = 0; i < CPU_Count; i++) {
        if(!Processor[i].Is_Bootstrap_CPU) {
            // create an initial stack page for core.  Stacks grow down so this is really the end of the stack
            // assembly code adds 4096 to the when loading esp
            void *ptr;
            ptr = Alloc_Page();
            CPUs[i].stack = ptr;
            send_INIT(Processor[i].APIC_Id);
        } else {
            // CPUs[i].stack = CURRENT_THREAD->stackPage + 4096;
            CPUs[i].initDone = 1;
        }
    }
    Micro_Delay(10000);

    for(i = 0; i < CPU_Count; i++) {
        if(!Processor[i].Is_Bootstrap_CPU) {
            Secondary_Stack = CPUs[i].stack;
            send_IPI(i,
                     APIC_ICR_DM_SIPI |
                     ((((unsigned int)START_SECONDARY_FUNC) >> 12) &
                      0xFF));
            Micro_Delay(10000);
            while (!CPUs[i].initDone)
                Micro_Delay(10000);
        }
    }
}


/*
 * C Entry point for newly booted secondary CPUs
 */
void Secondary_Start(int stack) {
    int i;
    int CPUid;
    int APICid;

    CPUid = Get_CPU_ID();

    // let boot CPU know we are done!
    CPUs[CPUid].initDone = 1;

    // wait for boot processor to finish init
    while (!CPUs[CPUid].running) {
        Micro_Delay(1000);
    }

    Init_GDT(CPUid);

    TODO_P(PROJECT_PERCPU, "Initialize PERCPU");

    Init_TSS();

    /* by modifying begin_int_atomic to autolock if not
       locked when interrupts are disabled, this
       lockKernel() became duplicative */
    /* lockKernel(); */
    Init_Interrupts(CPUid);

#ifdef USE_VM
    extern void Init_Secondary_VM();
    Init_Secondary_VM();
#endif

    Init_Scheduler(CPUid, CPUs[CPUid].stack);

    Init_Traps();

    APICid = Init_Local_APIC(CPUid);

    Init_Timer_Interrupt();

    Print("Init done\n");

    // let boot smp know we are done with init
    CPUs[CPUid].running = 2;

    KASSERT0(APICid == Get_CPU_ID(), "Apic id doesn't match cpuid");

    extern void Send_Timer_INT();

    // this init thread is done, calling exit forces us into the scheduler 
    Exit(0);
}

void Release_SMP(void) {
    int i;
    int irq;

    for(i = 0; i < CPU_Count; i++) {
        CPUs[i].running = 1;
        if(i)
            while (CPUs[i].running != 2) ;
    }
}

// global spin lock for all list operations
Spin_Lock_t listLock;

Spin_Lock_t kthreadLock;

int Is_Locked(Spin_Lock_t * lock) {
    return lock->lock;
}

void Spin_Lock_Init(Spin_Lock_t * lock) {
    KASSERT(lock);

    lock->lock = 0;
    lock->lastLocker = NULL;
    lock->locker = NULL;
}

void Spin_Lock(Spin_Lock_t * lock) {
    extern void Spin_Lock_INTERNAL(Spin_Lock_t * lock);
    struct Kernel_Thread *current = get_current_thread(0);      /* don't want to disable interrupts 
                                                                   for an advisory variable. */
    KASSERT(lock);
    Spin_Lock_INTERNAL(lock);
    lock->locker = current;
    // Print("   %p by %p\n", lock, CURRENT_THREAD);
}

/* returns zero if failed to acquire, 1 if acquired. */
/* intended to avoid deadlock conditions if an operation can
   proceed differently (or just fail) without a lock */
int Try_Spin_Lock(Spin_Lock_t * lock) {
    int was_held = 1;
    KASSERT(lock);
    /* might be able to avoid the movl instruction by adding
       '"0" (was_held)' as an input operand to the asm */
    __asm__ __volatile__("movl $0x1, %0\n\t"
                         "xchg %0, %1\n\t":"=a"(was_held),
                         "=m"(lock->lock));
    if(was_held) {
        return 0;
    } else {
        lock->locker = get_current_thread(0);
        return 1;
    }
}

void Spin_Unlock(Spin_Lock_t * lock) {
    extern void Spin_Unlock_INTERNAL(Spin_Lock_t * lock);
    KASSERT(lock);
    KASSERT(lock->lock);
    // KASSERT(lock->locker == g_currentThreads[Get_CPU_ID()]);
    lock->lastLocker = lock->locker;
    lock->locker = (void *)0xdead1000;  /* clearly invalid. */
    Spin_Unlock_INTERNAL(lock);
}

// map pic interrupt to be delivered through IOAPIC
//    xxxx - for now send them all to cpu0
void Map_IO_APIC_IRQ(int irq, void *handler) {
    // low seven bits are the irq# to pass to cpu
    IOAPIC_Write(0x10 + 2 * irq, 0x00000000 | irq);
    IOAPIC_Write(0x10 + 2 * irq + 1, 0x00000000);

    Install_Interrupt_Handler(irq, handler);
}

/*
 * Implement coarse grained kernel locks.  The functions lockKernel and unlockKernel will be called whenever
 *   interrupts are enabled or disabled.  This includes in interrupt and iret (see lowlevel.asm).
 */

void lockKernel() {
    Spin_Lock(&globalLock);
}

void unlockKernel() {
    /* early test so that return address may be helpful. */
    KASSERT0(globalLock.lock,
             "expected global lock to be held when calling unlockKernel");
    Spin_Unlock(&globalLock);
}

bool Kernel_Is_Locked(void) {
    /* pretty typically transferred. */
    /* if(globalLock.lock && globalLock.locker != g_currentThreads[Get_CPU_ID()]) {
       Print("kernel locked by another\n");
       }
     */
    return (globalLock.lock);
}


struct Kernel_Thread *get_current_thread(int atomic) {
    TODO_P(PROJECT_PERCPU, "Replace get_current_thread");
    int i = atomic ? Begin_Int_Atomic() : 0;    /* an interrupt could break us between the cpuid get and the subscript */
    struct Kernel_Thread *ret = g_currentThreads[Get_CPU_ID()];
    if(atomic)
        End_Int_Atomic(i);
    return ret;
}
