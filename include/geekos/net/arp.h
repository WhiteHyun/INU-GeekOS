/*
 * Address Resolution Protocol
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#ifndef _ARP_H_
#define _ARP_H_

#include <geekos/defs.h>
#include <geekos/ktypes.h>
#include <geekos/list.h>
#include <geekos/net/net.h>
#include <geekos/net/netbuf.h>
#include <geekos/string.h>


/* ARP Hardware types */
#define ARP_HTYPE_ETH 1

/* ARP Protocol types */
#define ARP_PTYPE_IPV4 0x0800

/* ARP Operation types */
#define ARP_OPER_REQUEST 1
#define ARP_OPER_REPLY 2
#define ARP_OPER_RARP_REQUEST 3
#define ARP_OPER_RARP_REPLY 4

#define ARP_PROT_ADDR_SIZE 4
#define ARP_HWARE_ADDR_SIZE 6

typedef uchar_t ARP_Protocol_Address[ARP_PROT_ADDR_SIZE];
typedef uchar_t ARP_Hardware_Address[ARP_HWARE_ADDR_SIZE];

struct ARP_Table_Element;

DEFINE_LIST(ARP_Table, ARP_Table_Element);

struct ARP_Table_Element {
    ushort_t htype;
    ushort_t ptype;
    uchar_t hardwareAddress[ARP_HWARE_ADDR_SIZE];
    uchar_t protocolAddress[ARP_PROT_ADDR_SIZE];
    ulong_t insertTime;

     DEFINE_LINK(ARP_Table, ARP_Table_Element);
};


struct ARP_Packet {
    ushort_t htype;             /* Hardware type - 16 bits */
    ushort_t ptype;             /* Protocol type - 16 bits */
    uchar_t hlen;               /* Hardware address length - 8 bits */
    uchar_t plen;               /* Protocol address length - 8 bits */
    ushort_t oper;              /* Operation sender is performing - 16 bits */
    uint_t sha0;                /* Sender Hardware Address - first 32 bits */
    ushort_t sha1;              /* Sender Hardware Address - last 16 bits */
    ushort_t spa0;              /* Sender Protocol Address - first 16 bits */
    ushort_t spa1;              /* Sender Protocol Address - last 16 bits */
    ushort_t tha0;              /* Target Hardware Address - first 16 bits */
    uint_t tha1;                /* Target Hardware Address - last 32 bits */
    uint_t tpa;                 /* Target Protocol Address - 32 bits */
};

IMPLEMENT_LIST(ARP_Table, ARP_Table_Element);

/* Arp networking */
/* 
extern int ARP_Dispatch(struct Net_Device *, struct Net_Buf * nBuf);
extern int ARP_Transmit(struct Net_Device *, struct ARP_Packet * packet, uchar_t *);
extern int ARP_Send_Request(struct Net_Device *, ushort_t htype,
			    ushort_t ptype,
			    const uchar_t protocolAddress[ARP_PROT_ADDR_SIZE]);
extern int ARP_Send_Request_And_Wait(struct Net_Device *, ushort_t htype,
				     ushort_t ptype,
				     ARP_Protocol_Address protocolAddress);
extern int ARP_Send_Reply(struct Net_Device *, 
			  struct ARP_Packet *,
			  const uchar_t protocolAddress[ARP_PROT_ADDR_SIZE]);
extern int ARP_Wait_For_Reply(struct Net_Device *, ARP_Protocol_Address address);
*/

/* The main call to be used to do an arp request */
extern int ARP_Resolve_Address(struct Net_Device *,
                               ushort_t htype,
                               ushort_t ptype,
                               ARP_Protocol_Address protocolAddress,
                               ARP_Hardware_Address hardwareAddress);

extern void Init_ARP_Protocol(void);

#endif
