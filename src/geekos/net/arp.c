/*
 * Address Resolution Protocol
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/malloc.h>
#include <geekos/errno.h>
#include <geekos/kassert.h>
#include <geekos/kthread.h>
#include <geekos/list.h>
#include <geekos/int.h>
#include <geekos/timer.h>
#include <geekos/int.h>
#include <geekos/alarm.h>

#include <geekos/net/arp.h>
#include <geekos/net/ethernet.h>
#include <geekos/net/ip.h>

#include <geekos/projects.h>

#define ARP_TIMEOUT_MS 10000    /* 10 second timeout */

// #define DEBUG_ARP(x...) Print("ARP: " x)
#define DEBUG_ARP(x...)

static struct ARP_Table s_arpTable;
static struct Mutex s_arpTableMutex;


static int ARP_Table_Insert(ushort_t htype, ushort_t ptype,
                            const uchar_t
                            hardwareAddress[ARP_HWARE_ADDR_SIZE],
                            const uchar_t
                            protocolAddress[ARP_PROT_ADDR_SIZE]);

static int ARP_Send_Reply(struct Net_Device *device,
                          struct ARP_Packet *receivedPacket,
                          const uchar_t
                          protocolAddress[ARP_PROT_ADDR_SIZE]);
static int ARP_Send_Request(struct Net_Device *device, ushort_t htype,
                            ushort_t ptype,
                            const uchar_t
                            protocolAddress[ARP_PROT_ADDR_SIZE]);


static bool ARP_Compare_Protocol_Addresses(ushort_t ptype
                                           __attribute__ ((unused)),
                                           const uchar_t
                                           pAddr1[ARP_PROT_ADDR_SIZE],
                                           const uchar_t
                                           pAddr2[ARP_PROT_ADDR_SIZE]) {
    int i;
    for(i = 0; i < ARP_PROT_ADDR_SIZE; ++i) {
        if(pAddr1[i] != pAddr2[i])
            return false;
    }
    return true;
}

/* helper to fetch the protocol address associated with a
   (local) device */
static int ARP_Get_Protocol_Address(struct Net_Device *device, int ptype,
                                    uchar_t * protocolAddress) {
    KASSERT0(device,
             "device to ask about a protocol address must be not null\n");
    if(ptype == ARP_PTYPE_IPV4) {
        struct IP_Device *ipDevice = NULL;
        int rc = IP_Device_Get_By_Name(&ipDevice, device->devName);
        if(rc != 0) {
            DEBUG_ARP("failed to get IP address of device %s: %d\n",
                      device->devName, rc);
            return rc;
        }
        memcpy(protocolAddress, ipDevice->ipAddress.ptr, 4);
        return 0;
    } else {
        DEBUG_ARP("unsupported protocol type %d\n", ptype);
        return -1;
    }

}

/* Arp networking */
int ARP_Dispatch(struct Net_Device *device, struct Net_Buf *nBuf) {
    struct ARP_Packet packet;

    Net_Buf_Extract(nBuf, 0, &packet, sizeof(packet));
    int rc = 0;

    TODO_P(PROJECT_ARP,
           "handle a received arp packet which may be a request issued by someone else or a response to our query.");
    return EUNSUPPORTED;
}

int ARP_Transmit(struct Net_Device *device, struct ARP_Packet *packet,
                 uchar_t * ethDestAddr) {
    int paddingNeeded = sizeof(struct ARP_Packet) - ETH_MIN_DATA;
    struct Net_Buf *nBuf = NULL;
    int rc = Net_Buf_Create(&nBuf);
    if(rc != 0)
        goto fail;

    rc = Net_Buf_Prepend(nBuf, packet, sizeof(struct ARP_Packet),
                         NET_BUF_ALLOC_LEND);
    if(rc != 0)
        goto fail;

    Eth_Transmit(device, nBuf, ethDestAddr, ETH_ARP);

    Net_Buf_Destroy(nBuf);

    return rc;

  fail:
    DEBUG_ARP("failed to transmit");
    return rc;
}

static int ARP_Send_Request(struct Net_Device *device,
                            ushort_t htype,
                            ushort_t ptype,
                            const uchar_t
                            protocolAddress[ARP_PROT_ADDR_SIZE]) {
    uchar_t ethDestAddr[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    int rc = 0;

    struct ARP_Packet *packet = Malloc(sizeof(struct ARP_Packet));
    if(packet == NULL)
        return ENOMEM;

    TODO_P(PROJECT_ARP, "Fill in the arp request");

    rc = ARP_Transmit(device, packet, ethDestAddr);

  fail:
    Free(packet);
    return rc;
}

static int ARP_Send_Reply(struct Net_Device *device,
                          struct ARP_Packet *receivedPacket,
                          const uchar_t
                          protocolAddress[ARP_PROT_ADDR_SIZE]) {
    int rc = 0;
    TODO_P(PROJECT_ARP,
           "construct a reply to this received packet and transmit (you may reuse the packet)");

    return rc;
}

int ARP_Send_Request_And_Wait(struct Net_Device *device, ushort_t htype,
                              ushort_t ptype,
                              ARP_Protocol_Address protocolAddress) {
    int rc = 0;

    KASSERT(Interrupts_Enabled());

    TODO_P(PROJECT_ARP,
           "ready this thread to wait on a reply or timeout; construct and send the request.");

    return rc;
}

/* the arp table mutex should be locked */
static struct ARP_Table_Element *ARP_Find_By_Proto(ushort_t htype,
                                                   ushort_t ptype,
                                                   const uchar_t
                                                   protocolAddress
                                                   [ARP_PROT_ADDR_SIZE]) {
    struct ARP_Table_Element *curr;
    for(curr = Get_Front_Of_ARP_Table(&s_arpTable);
        curr != NULL; curr = Get_Next_In_ARP_Table(curr)) {
        if(curr->htype == htype &&
           curr->ptype == ptype &&
           ARP_Compare_Protocol_Addresses(ptype, curr->protocolAddress,
                                          protocolAddress)) {
            return curr;
        }
    }
    return NULL;
}

/* Arp Table */
static int ARP_Table_Lookup(ushort_t htype, ushort_t ptype,
                            const uchar_t
                            protocolAddress[ARP_PROT_ADDR_SIZE],
                            uchar_t
                            hardwareAddressDest[ARP_HWARE_ADDR_SIZE]) {
    struct ARP_Table_Element *curr;
    int ret;

    KASSERT0(hardwareAddressDest != NULL,
             "hardwareAddressDest not provided to ARP_Table_Lookup");

    Mutex_Lock(&s_arpTableMutex);
    curr = ARP_Find_By_Proto(htype, ptype, protocolAddress);
    if(curr) {
        memcpy(hardwareAddressDest, curr->hardwareAddress,
               ARP_HWARE_ADDR_SIZE);
        ret = 0;
    } else {
        ret = -1;
    }
    Mutex_Unlock(&s_arpTableMutex);
    return ret;
}

static int ARP_Table_Insert(ushort_t htype, ushort_t ptype,
                            const uchar_t
                            hardwareAddress[ARP_HWARE_ADDR_SIZE],
                            const uchar_t
                            protocolAddress[ARP_PROT_ADDR_SIZE]) {
    Mutex_Lock(&s_arpTableMutex);
    struct ARP_Table_Element *element =
        Malloc(sizeof(struct ARP_Table_Element));
    if(element == NULL)
        return ENOMEM;

    element->htype = htype;
    element->ptype = ptype;
    memcpy(element->hardwareAddress, hardwareAddress,
           ARP_HWARE_ADDR_SIZE);
    memcpy(element->protocolAddress, protocolAddress, ARP_PROT_ADDR_SIZE);

    Add_To_Front_Of_ARP_Table(&s_arpTable, element);

    Mutex_Unlock(&s_arpTableMutex);

    return 0;
}

static int ARP_Table_Delete(ushort_t htype, ushort_t ptype,
                            const uchar_t
                            protocolAddress[ARP_PROT_ADDR_SIZE]) {
    struct ARP_Table_Element *curr;
    int ret;

    Mutex_Lock(&s_arpTableMutex);
    curr = ARP_Find_By_Proto(htype, ptype, protocolAddress);
    if(curr) {
        Remove_From_ARP_Table(&s_arpTable, curr);
        Free(curr);
        ret = 0;
    } else {
        ret = -1;
    }
    Mutex_Unlock(&s_arpTableMutex);
    return ret;
}

static int ARP_Table_Cleanse(ushort_t htype, ushort_t ptype) {
    TODO_P(PROJECT_ARP,
           "Implement ARP_Table_Cleanse() to remove stale arp entries");
    return 0;
}

/* should be the main arp routine. check the cache, if not in cache, send
   request, add to wait queue for receipt of a reply or timeout. */
int ARP_Resolve_Address(struct Net_Device *device,
                        ushort_t htype,
                        ushort_t ptype,
                        ARP_Protocol_Address protocolAddress,
                        ARP_Hardware_Address hardwareAddress) {
    int rc = 0;

    KASSERT(Interrupts_Enabled());

    TODO_P(PROJECT_ARP,
           "Consult the cache, if not in cache, send request and wait");

    return rc;

}


void Init_ARP_Protocol(void) {
    Mutex_Init(&s_arpTableMutex);
    s_arpTable.head = s_arpTable.tail = 0;
    TODO_P(PROJECT_ARP, "Initialize other needed variables");

    Eth_Dispatch_Table_Add(ETH_ARP, ARP_Dispatch);
}
