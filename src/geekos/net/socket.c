/*
 * Socket Interface
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/net/socket.h>
#include <geekos/kassert.h>
#include <geekos/malloc.h>
#include <geekos/errno.h>
#include <geekos/string.h>
#include <geekos/net/netbuf.h>
#include <geekos/net/udp.h>
#include <geekos/net/tcp.h>
#include <geekos/alarm.h>
#include <geekos/screen.h>
#include <geekos/kassert.h>

#include <geekos/projects.h>


int Socket_Create(uchar_t type, int flags __attribute__ ((unused))) {
    TODO_P(PROJECT_SOCKETS, "Create");
    return -1;
}


int Socket_Connect(ulong_t id, ushort_t port, IP_Address * ipAddress) {
    int rc = 0;
    TODO_P(PROJECT_SOCKETS, "Connect");
    return rc;
}

int Socket_Bind(ulong_t id, ushort_t port, IP_Address * ipAddress) {
    TODO_P(PROJECT_SOCKETS, "Bind");
    return 0;
}

int Socket_Listen(ulong_t id, ulong_t backlog) {
    int rc = 0;
    TODO_P(PROJECT_SOCKETS, "Listen");
    return rc;
}


int Socket_Accept(ulong_t id, IP_Address * clientIpAddress,
                  ushort_t * clientPort) {
    int fd = -1;
    TODO_P(PROJECT_SOCKETS, "Accept (returns a socket!)");
    return fd;
}

int Socket_Receive(ulong_t id, uchar_t * buffer, ulong_t bufferSize) {
    TODO_P(PROJECT_SOCKETS, "Receive");
    return 0;
}

int Socket_Send(ulong_t id, uchar_t * buffer, ulong_t bufferSize) {
    TODO_P(PROJECT_SOCKETS, "Send");
    return 0;
}

int Socket_Send_To(ulong_t id, uchar_t * buffer, ulong_t bufferSize,
                   ushort_t port, IP_Address * ipAddress) {
    TODO_P(PROJECT_SOCKETS, "SendTo");
    return 0;
}

int Socket_Receive_From(ulong_t id, uchar_t * buffer, ulong_t bufferSize,
                        ushort_t * port, IP_Address * ipAddress) {
    TODO_P(PROJECT_SOCKETS, "ReceiveFrom");
    return 0;
}

int Socket_Destroy(ulong_t id) {
    TODO_P(PROJECT_SOCKETS, "Socket Destroy");
    return 0;
}



int Socket_Close(ulong_t id) {
    TODO_P(PROJECT_SOCKETS, "Socket Close");
    return EUNSUPPORTED;
}



int Socket_Dispatch(struct IP_Device *device, uchar_t type,
                    ushort_t destPort, ushort_t srcPort,
                    IP_Address * destAddress, IP_Address * srcAddress,
                    struct Net_Buf *nBuf, void *data) {
    int rc = 0;
    TODO_P(PROJECT_SOCKETS, "socket dispatch (received packet)");
    return rc;
}


void Init_Sockets(void) {
    TODO_P(PROJECT_SOCKETS, "initialization");
}
