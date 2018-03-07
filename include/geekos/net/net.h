/*
 * General Network Device Interface
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#ifndef _NET_H_
#define _NET_H_

#define NET_NAME_SIZE              32
#define MAX_DEV_ADDR               32

#include <geekos/synch.h>
#include <geekos/list.h>
#include <geekos/defs.h>
#include <geekos/net/netbuf.h>
#include <geekos/kthread.h>

typedef uchar_t MAC_Address[6];

struct Net_Device;
struct Net_Device_Receive_State;

#define ntohs(x) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))
#define htons(x) ((((x) >> 8) & 0xff) | (((x) & 0xff) << 8))

#ifdef GEEKOS

/*
 * List of all devices registered on the system
 */
DEFINE_LIST(Net_Device_List, Net_Device);

/*
 * Queue of device receive states
 */
DEFINE_LIST(Net_Device_Receive_Packet_Queue, Net_Device_Packet);

DEFINE_LIST(Net_Device_Packet_Queue, Net_Device_Packet);

struct Net_Device_Packet {
    void *buffer;
    unsigned int bufferLen;
    struct Net_Device *device;

     DEFINE_LINK(Net_Device_Receive_Packet_Queue, Net_Device_Packet);
     DEFINE_LINK(Net_Device_Packet_Queue, Net_Device_Packet);
};

struct Net_Device_Header {
    uchar_t status;
    uchar_t next;
    ushort_t count;
};


/*struct Net_Device_Packet_Queue {
  struct Net_Device_Packet queue[32];
  unsigned int start, stop;
  };*/

struct Net_Device {
    char name[NET_NAME_SIZE];   /* Name device has */
    char devName[NET_NAME_SIZE];        /* Dev name assigned by OS */

    /* I/O stuff */
    ulong_t baseAddr;           /* Base I/O address */
    ulong_t irq;                /* IRQ number */

    /* Device address stuff */
    ulong_t addrLength;         /* Length of the hardware address */
    uchar_t devAddr[MAX_DEV_ADDR];      /* Hardware address of the device */

    /* Device status and flags stuff */
    ulong_t status;
    ulong_t flags;

    /* Error tracking */
    ulong_t rxPacketErrors;
    ulong_t txPacketErrors;
    ulong_t rxPackets;
    ulong_t txPackets;
    ulong_t rxBytes;
    ulong_t txBytes;

    /* Device function pointers */
    int (*init) (struct Net_Device *);
    void (*transmit) (struct Net_Device *, void *, ulong_t);
    void (*receive) (struct Net_Device *, void *, ulong_t, ulong_t);
    void (*reset) (struct Net_Device *);
    void (*getHeader) (struct Net_Device *, struct Net_Device_Header *,
                       ulong_t);
    void (*completeReceive) (struct Net_Device *,
                             struct Net_Device_Header *);

    /* Links for management in lists */
     DEFINE_LINK(Net_Device_List, Net_Device);

};

struct Net_Device_Capabilities {
    const char *name;
    int (*init) (struct Net_Device *);
    void (*transmit) (struct Net_Device *, void *, ulong_t);
    void (*receive) (struct Net_Device *, void *, ulong_t, ulong_t);
    void (*reset) (struct Net_Device *);
    void (*getHeader) (struct Net_Device *, struct Net_Device_Header *,
                       ulong_t);
    void (*completeReceive) (struct Net_Device *,
                             struct Net_Device_Header *);
};

/*
 * Network device list implementation
 */
IMPLEMENT_LIST(Net_Device_List, Net_Device);

/*
 * Network device receive packet queue
 */
IMPLEMENT_LIST(Net_Device_Receive_Packet_Queue, Net_Device_Packet);

IMPLEMENT_LIST(Net_Device_Packet_Queue, Net_Device_Packet);

/* Public functions */
int Register_Net_Device(struct Net_Device_Capabilities *, ulong_t,
                        ulong_t, const char *nameBase);
int Unregister_Net_Device(struct Net_Device *);
int Get_Net_Device_By_Name(const char *name, /*@out@ */
                           struct Net_Device **device);
struct Net_Device_List *Get_Net_Device_List(void);
int Get_Net_Device_By_IRQ(unsigned int irq, /*@out@ */
                          struct Net_Device **device);
int Net_Device_Receive(struct Net_Device *, ushort_t ringBufferPage);
int Get_Free_Net_Device(struct Net_Device **device);
void Init_Network_Devices();

#endif
#endif
