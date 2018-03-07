/*
 * UDP Protocol
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/net/udp.h>
#include <geekos/kassert.h>
#include <geekos/malloc.h>
#include <geekos/errno.h>
#include <geekos/projects.h>

int UDP_Transmit(IP_Address * src,
                 IP_Address * destination,
                 ushort_t srcPort,
                 ushort_t destPort, struct Net_Buf *nBuf) {
    int rc = 0;
    TODO_P(PROJECT_TCP, "transmit with these params");
    return rc;
}


extern int UDP_Dispatch(struct IP_Device *device,
                        IP_Address * destAddress, IP_Address * srcAddress,
                        struct Net_Buf *nBuf) {
    int rc = 0;
    TODO_P(PROJECT_UDP, "dispatch this packet's contents to socket");

    return rc;
}
