/*
 * This is the device-driver interface to the interrupt system.
 * Copyright (c) 2001,2003 David H. Hovemeyer <daveho@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.11 $
 * 
 */

#ifndef GEEKOS_IRQ_H
#define GEEKOS_IRQ_H

#include <geekos/int.h>

void Install_IRQ(int irq, Interrupt_Handler handler);
ushort_t Get_IRQ_Mask(void);
void Set_IRQ_Mask(ushort_t mask);
void Enable_IRQ(int irq);
void Disable_IRQ(int irq);

/*
 * IRQ handlers should call these to begin and end the
 * interrupt.
 */
void Begin_IRQ(struct Interrupt_State *state);
void End_IRQ(struct Interrupt_State *state);

#endif /* GEEKOS_IRQ_H */
