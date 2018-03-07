/*
 * UDP Protocol
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#ifndef UDP_H
#define UDP_H

#include <geekos/defs.h>
#include <geekos/ktypes.h>
#include <geekos/net/ip.h>
#include <geekos/net/netbuf.h>
#include <geekos/net/socket.h>

struct UDP_Header {
    ushort_t srcPort;
    ushort_t destPort;
    ushort_t length;
    ushort_t checksum;
};

extern int UDP_Transmit(IP_Address * src,
                        IP_Address * destination,
                        ushort_t srcPort,
                        ushort_t destPort, struct Net_Buf *nBuf);
extern int UDP_Dispatch(struct IP_Device *device,
                        IP_Address * destAddress,
                        IP_Address * srcAddress, struct Net_Buf *nBuf);


#endif
