/*
 * Network
 * Copyright (c) 2009, Calvin Grunewald
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.0 $
 *
 */

#ifndef _LIBC_NET_H_
#define _LIBC_NET_H_

#include <geekos/defs.h>
#include <geekos/ktypes.h>

#define ETH_MAX_DATA 1500
#define ETH_MIN_DATA 46


int EthPacketSend(const void *buffer, ulong_t length,
                  const uchar_t dest[], const char *device);
int EthPacketReceive(void *buffer, ulong_t length);
int Arp(uchar_t *, uchar_t *);

#endif
