/*
 * Ethernet Protocol
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#ifndef _ETHERNET_H_
#define _ETHERNET_H_

#include <geekos/net/net.h>
#include <geekos/net/netbuf.h>
#include <geekos/kthread.h>

/* Ether Types */
#define ETH_IPV4 0x0800
#define ETH_ARP 0x0806
#define ETH_IPV6 0x86DD

#define ETH_TYPE_COUNT 3

#define ETH_MIN_DATA 46U
#define ETH_MAX_DATA 1500U

struct Ethernet_Protocol_Element;
DEFINE_LIST(Ethernet_Wait_Queue, Ethernet_Protocol_Element);

struct Ethernet_Protocol_Element {
    struct Kernel_Thread *kthread;
    struct Net_Buf **nBuf;

     DEFINE_LINK(Ethernet_Wait_Queue, Ethernet_Protocol_Element);
};

struct Ethernet_Header {
    uchar_t destAddr[6];
    uchar_t srcAddr[6];
    ushort_t type;
};

extern int Eth_Transmit(struct Net_Device *dev,
                        struct Net_Buf *nBuf,
                        uchar_t * destAddr, ushort_t type);

extern int Eth_Dispatch(struct Net_Device *dev, struct Net_Buf *nBuf);


extern int Eth_Receive(struct Net_Device *dev, struct Net_Buf **nBuf);

extern int Eth_Dispatch_Table_Add(ushort_t,
                                  int (*)(struct Net_Device *,
                                          struct Net_Buf *));

IMPLEMENT_LIST(Ethernet_Wait_Queue, Ethernet_Protocol_Element);

#endif
