/*************************************************************************/
/*
 * GeekOS master source distribution and/or project solution
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
/*************************************************************************/
/*
 * This is the device-driver interface to the interrupt system.
 *
 */

#include <geekos/kassert.h>
#include <geekos/idt.h>
#include <geekos/io.h>
#include <geekos/irq.h>
#include <geekos/smp.h>

/* ----------------------------------------------------------------------
 * Private functions and data
 * ---------------------------------------------------------------------- */

/*
 * Current IRQ mask.
 * This should be kept up to date with setup.asm
 * (which does the initial programming of the PICs).
 */
static ushort_t s_irqMask = 0xff7b;     /* ns: disable irq7 which qemu may generate bizarrely */

/*
 * Get the master and slave parts of an IRQ mask.
 */
#define MASTER(mask) ((mask) & 0xff)
#define SLAVE(mask) (((mask)>>8) & 0xff)


/* ----------------------------------------------------------------------
 * Public functions
 * ---------------------------------------------------------------------- */

/*
 * Install a handler for given IRQ.
 * Note that we don't unmask the IRQ.
 */
void Install_IRQ(int irq, Interrupt_Handler handler) {
    Map_IO_APIC_IRQ(irq, handler);
}

/*
 * Get current IRQ mask.  Each bit position represents
 * one of the 16 IRQ lines.
 */
ushort_t Get_IRQ_Mask(void) {
    return s_irqMask;
}

/*
 * Set the IRQ mask.
 */
void Set_IRQ_Mask(ushort_t mask) {
    uchar_t oldMask, newMask;

    oldMask = MASTER(s_irqMask);
    newMask = MASTER(mask);
    if(newMask != oldMask) {
        Out_Byte(0x21, newMask);
    }

    oldMask = SLAVE(s_irqMask);
    newMask = SLAVE(mask);
    if(newMask != oldMask) {
        Out_Byte(0xA1, newMask);
    }

    s_irqMask = mask;
}

/*
 * Enable given IRQ.
 */
void Enable_IRQ(int irq) {
    bool iflag = Begin_Int_Atomic();
    ushort_t mask = Get_IRQ_Mask();

    KASSERT(irq >= 0 && irq <= 32);
    mask &= ~(1 << irq);
    Set_IRQ_Mask(mask);

    End_Int_Atomic(iflag);
}

/*
 * Disable given IRQ.
 */
void Disable_IRQ(int irq) {
    bool iflag = Begin_Int_Atomic();
    ushort_t mask = Get_IRQ_Mask();

    KASSERT(irq >= 0 && irq < 16);
    mask |= (1 << irq);
    Set_IRQ_Mask(mask);

    End_Int_Atomic(iflag);
}

/*
 * Called by an IRQ handler to begin the interrupt.
 * Currently a no-op.
 */
void Begin_IRQ(struct Interrupt_State *state __attribute__ ((unused))) {
}

/*
 * Called by an IRQ handler to end the interrupt.
 * Sends an EOI command to the appropriate PIC(s).
 */
void End_IRQ(struct Interrupt_State *state) {
    int irq = state->intNum - FIRST_EXTERNAL_INT;
    uchar_t command = 0x60 | (irq & 0x7);

    if(irq < 8) {
        /* Specific EOI to master PIC */
        Out_Byte(0x20, command);
    } else {
        /* Specific EOI to slave PIC, then to master (cascade line) */
        Out_Byte(0xA0, command);
        Out_Byte(0x20, 0x62);
    }
}
