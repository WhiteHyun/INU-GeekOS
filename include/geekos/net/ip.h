/*
 * Internet Protocol
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#ifndef IP_H
#define IP_H

#include <geekos/defs.h>
#include <geekos/ktypes.h>
#include <geekos/list.h>
#include <geekos/net/net.h>
#include <geekos/net/netbuf.h>
#include <geekos/net/ipdefs.h>

#define IP_UDP_PROTOCOL 17

// extern IP_Address * INADDR_ANY;
// extern IP_Address * INADDR_BROADCAST;

#define INADDR_BROADCAST 0xffffffff
#define INADDR_ANY 0x0

struct IP_Header {
    /* we are little endian, which means that hlen is declared first, even though it will appear last */
    uchar_t hLen:4;
    uchar_t version:4;
    uchar_t tos;
    ushort_t length;
    ushort_t ident;
    // ushort_t flags:3;
    // ushort_t offset:13;
    ushort_t frag_off;
    uchar_t ttl;
    uchar_t protocol;
    ushort_t checksum;
    uint_t sourceAddr;
    uint_t destAddr;

    // we may have options

};

struct IP_Device;

DEFINE_LIST(IP_Device_List, IP_Device);

struct IP_Device {
    struct Net_Device *netDevice;
    IP_Address ipAddress;
    Netmask subnet;

     DEFINE_LINK(IP_Device_List, IP_Device);
};

#ifdef GEEKOS

IMPLEMENT_LIST(IP_Device_List, IP_Device);

extern int IP_Dispatch(struct Net_Device *, struct Net_Buf *);
extern int IP_Transmit(IP_Address * src, IP_Address * destination,
                       struct Net_Buf *, uchar_t tos, ushort_t protocol);


extern int IP_Device_Register(struct Net_Device *, IP_Address *,
                              Netmask *);
extern int IP_Device_Unregister(struct IP_Device *);
extern int IP_Device_Get_By_Name(struct IP_Device **, char *);
extern int IP_Device_Configure(char *name, IP_Address * ipAddress,
                               Netmask * netmask);
extern int IP_Device_Stat(struct IP_Device_Info *info,
                          ulong_t deviceCount, char *name);
extern int IP_Dispatch_Table_Add(ushort_t type,
                                 int (*dispatcher) (struct IP_Device *,
                                                    IP_Address *,
                                                    IP_Address *,
                                                    struct Net_Buf *));
extern struct IP_Device_List *IP_Get_Device_List(void);


extern void Init_IP(void);

#endif
#endif
