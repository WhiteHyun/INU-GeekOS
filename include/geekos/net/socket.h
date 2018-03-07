/*
 * Socket Interface
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */


#ifndef GEEKOS_SOCKET_H_
#define GEEKOS_SOCKET_H_

#include <geekos/defs.h>
#include <geekos/ktypes.h>
#include <geekos/net/ip.h>
#include <geekos/kthread.h>
#include <geekos/synch.h>
#include <geekos/list.h>

#define SOCK_DGRAM 0
#define SOCK_STREAM 1

#define AF_INET 2

// U for unsigned
#define SOCK_BUFFER_SIZE 4096U

#define MSS 1024U

// Socket States
enum {
    SOCK_STATE_CONNECTING_CLIENT,
    SOCK_STATE_CONNECTING_SERVER,
    SOCK_STATE_ESTABLISHED,
    SOCK_STATE_BOUND,
    SOCK_STATE_LISTENING,
    SOCK_STATE_ERROR,

    // States for shutting down the connection
    SOCK_FIN_WAIT_1,
    SOCK_FIN_WAIT_2,
    SOCK_CLOSE_WAIT,
    SOCK_CLOSING,
    SOCK_LAST_ACK,
    SOCK_TIME_WAIT,
    SOCK_CLOSED
};

struct Socket {
    ulong_t id;
    ulong_t flags;
    int state;
    IP_Address localAddress;
    IP_Address remoteAddress;
    ushort_t localPort;
    ushort_t remotePort;
    ushort_t type;
    bool multihomed;
    bool bound;
    bool initialzed;
};

struct TCP_Connection {
    IP_Address address;
    ushort_t port;
    ulong_t sequenceNumber;
    ushort_t targetPort;
    IP_Address targetAddress;
};

struct TCP_Socket {
    ulong_t id;
    ulong_t flags;
    int state;
    IP_Address localAddress;
    IP_Address remoteAddress;
    ushort_t localPort;
    ushort_t remotePort;
    ushort_t type;
    bool multihomed;
    bool bound;
    bool initialzed;

    ulong_t advertisedWindow;

    // Transmit
    ulong_t lastByteAckedPtr;   // Buffer ptr
    ulong_t lastByteSentPtr;    // Buffer ptr
    ulong_t lastByteWrittenPtr; // Buffer ptr

    ulong_t lastByteAcked;      // Sequence number
    ulong_t lastByteSent;       // Sequence number
    ulong_t lastByteWritten;    // Sequence number

    int sendTimer;
    ulong_t maxSendBuffer;
    uchar_t sendBuffer[SOCK_BUFFER_SIZE];
    struct Mutex sendMutex;
    struct Condition sendCond;

    // Close mutex and semaphore
    struct Mutex closeMutex;
    struct Condition closeCond;

    // Receive Buffer
    ulong_t lastByteReadPtr;    // Buffer ptr
    ulong_t nextByteExpectedPtr;        // Buffer ptr
    ulong_t lastByteRcvdPtr;    // Buffer ptr

    ulong_t lastByteRead;       // Sequence number
    ulong_t nextByteExpected;   // Sequence number
    ulong_t lastByteRcvd;       // Sequence number
    ulong_t maxReceiveBuffer;
    uchar_t receiveBuffer[SOCK_BUFFER_SIZE];
    struct Mutex receiveMutex;
    struct Condition receiveCond;

    // Support for listening
    struct Mutex listenMutex;
    struct Condition listenCond;
    ulong_t backlogMaxSize;
    ulong_t backlogIndex;
    ulong_t backlogSize;
    struct TCP_Connection *backlog;
    bool listening;
    bool backlogOverflow;


};

struct UDP_Packet_Data;

DEFINE_LIST(UDP_Packet_Queue, UDP_Packet_Data);

struct UDP_Socket {
    ulong_t id;
    ulong_t flags;
    int state;
    IP_Address localAddress;
    IP_Address remoteAddress;
    ushort_t localPort;
    ushort_t remotePort;
    ushort_t type;
    bool multihomed;
    bool bound;
    bool initialzed;

    ulong_t bufferSize;
    uchar_t receiveBuffer[SOCK_BUFFER_SIZE];
    struct UDP_Packet_Queue queue;

    struct Mutex mutex;
    struct Condition condition;
};

/*
 * Socket Interface
 * Create - Create a socket with the specified type and return the socket id
 * Get - Get's the id of a socket that uses the associated bindings.
 * Bind - Bind a socket to a local interface (specified by IP) and port. If
 * 	the interface is NULL, listen on all interfaces.
 * Send - Send data out on a socket
 * Receive - Receive data from a socket
 */

int Socket_Create(uchar_t type, int flags);
int Socket_Connect(ulong_t id, ushort_t port, IP_Address * ipAddress);
int Socket_Accept(ulong_t id, IP_Address * clientIpAddress,
                  ushort_t * clientPort);
int Socket_Listen(ulong_t id, ulong_t backlog);
int Socket_Bind(ulong_t id, ushort_t port, IP_Address * ipAddress);
int Socket_Receive(ulong_t id, uchar_t * buffer, ulong_t bufferSize);
int Socket_Send(ulong_t id, uchar_t * buffer, ulong_t bufferSize);
int Socket_Send_To(ulong_t id, uchar_t * buffer, ulong_t bufferSize,
                   ushort_t port, IP_Address * ipAddress);
int Socket_Receive_From(ulong_t id, uchar_t * buffer, ulong_t bufferSize,
                        ushort_t * port, IP_Address * ipAddress);
int Socket_Close(ulong_t id);
int Socket_Destroy(ulong_t id);

int Socket_Dispatch(struct IP_Device *device, uchar_t type,
                    ushort_t destPort, ushort_t srcPort,
                    IP_Address * destAddress, IP_Address * srcAddress,
                    struct Net_Buf *nBuf, void *data);

void Init_Sockets(void);


#endif /* SOCKET_H_ */
