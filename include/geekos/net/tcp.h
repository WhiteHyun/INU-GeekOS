/*
 * TCP Protocol
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#ifndef TCP_H_
#define TCP_H_

#include <geekos/ktypes.h>
#include <geekos/defs.h>
#include <geekos/net/ip.h>
#include <geekos/net/netbuf.h>

#define IP_TCP_PROTOCOL 6

#define TCP_CWR 		1
#define TCP_ECE			2
#define TCP_URG			4
#define TCP_ACK			8
#define TCP_PSH			16
#define TCP_RST			32
#define TCP_SYN			64
#define TCP_FIN			128

struct TCP_Header {
    ulong_t srcPort:16;
    ulong_t destPort:16;
    ulong_t seqNum;
    ulong_t ackNum;
    ulong_t dataOffset:4;
    ulong_t reserved:4;
    ulong_t cwr:1;
    ulong_t ece:1;
    ulong_t urg:1;
    ulong_t ack:1;
    ulong_t psh:1;
    ulong_t rst:1;
    ulong_t syn:1;
    ulong_t fin:1;
    ulong_t windowSize:16;
    ulong_t checksum:16;
    ulong_t urgentPointer:16;
};

// Types of TCP Transmissions
extern int TCP_Transmit(IP_Address * srcAddress, IP_Address * destAddress,
                        ushort_t srcPort, ushort_t destPort,
                        uchar_t flags, ulong_t seqNum, ulong_t ackNum,
                        ulong_t advertisedWindow, struct Net_Buf *nBuf);

extern int TCP_Dispatch(struct IP_Device *device,
                        IP_Address * destAddress, IP_Address * srcAddress,
                        struct Net_Buf *nBuf);


#endif /* TCP_H_ */
