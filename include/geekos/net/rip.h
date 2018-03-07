/*
 * RIP Protocol
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#ifndef RIP_H
#define RIP_H

#include <geekos/defs.h>
#include <geekos/ktypes.h>
#include <geekos/net/socket.h>

#define RIP_REQUEST 1
#define RIP_RESPONSE 2

#define RIP_VERSION 2

#define RIP_INFINITY 16
#define RIP_SOCKET_PORT 520

#define RIP_PACKET_MAX_SIZE 130

struct RIP_Entry {
    ushort_t afi;
    ushort_t routeTag;
    ulong_t ipAddress;
    ulong_t subnet;
    ulong_t nextHop;
    ulong_t metric;
};

struct RIP_Header {
    uchar_t command;
    uchar_t version;
    ushort_t unused;
    ushort_t mask;
    ushort_t authType;
    uchar_t auth[16];
};

void Init_RIP(void);

#endif
