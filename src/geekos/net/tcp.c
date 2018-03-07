/*
 * TCP Protocol
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/net/tcp.h>
#include <geekos/malloc.h>
#include <geekos/errno.h>
#include <geekos/screen.h>
#include <geekos/net/socket.h>
#include <geekos/projects.h>

int TCP_Dispatch(struct IP_Device *device, IP_Address * destAddress,
                 IP_Address * srcAddress, struct Net_Buf *nBuf) {
    struct TCP_Header header;
    int rc;
    TODO_P(PROJECT_TCP, "dispatch this packet's contents to socket");

    return 0;
}

int TCP_Transmit(IP_Address * srcAddress, IP_Address * destAddress,
                 ushort_t srcPort, ushort_t destPort, uchar_t flags,
                 ulong_t seqNum, ulong_t ackNum, ulong_t advertisedWindow,
                 struct Net_Buf *nBuf) {
    int rc = 0;
    TODO_P(PROJECT_TCP, "transmit with these params");
    return rc;
}
