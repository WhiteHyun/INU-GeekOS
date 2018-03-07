/*
 * General Network Device Interface
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/net/net.h>
#include <geekos/string.h>
#include <geekos/screen.h>
#include <geekos/kassert.h>
#include <geekos/malloc.h>
#include <geekos/int.h>
#include <geekos/net/ne2000.h>
#include <geekos/io.h>
#include <geekos/errno.h>
#include <geekos/net/ethernet.h>
#include <geekos/syscall.h>     /* for checking the syscall table on init */
#include <geekos/sys_net.h>     /* for checking the syscall table on init */

/* Sorted list of devices */
static struct Net_Device_List s_deviceList;

/* packets waiting to be read */
static struct Net_Device_Receive_Packet_Queue s_receivePacketQueue;

/* threads blocked awaiting a packet */
static struct Thread_Queue s_receiveThreadQueue;

/* Private Functions */
static struct Net_Device *Allocate_Net_Device(void) {
    struct Net_Device *device = Malloc(sizeof(struct Net_Device));

    KASSERT(device != 0);

    memset(device, '\0', sizeof(struct Net_Device));
    return device;
}


static void Destroy_Net_Device(struct Net_Device *dev) {
    Free(dev);
}


static int Get_Next_Device_Number(const char *nameBase
                                  __attribute__ ((unused))) {
    static int s_nextDeviceNumber;
    /* so what if we have lo7 */
    return s_nextDeviceNumber++;
}

static void Net_Device_Receive_Thread(ulong_t arg
                                      __attribute__ ((unused))) {
    while (1) {
        Disable_Interrupts();
        if(!Is_Net_Device_Receive_Packet_Queue_Empty
           (&s_receivePacketQueue)) {
            struct Net_Buf *nBuf;
            struct Net_Device_Packet *packet;

            packet =
                Remove_From_Front_Of_Net_Device_Receive_Packet_Queue
                (&s_receivePacketQueue);
            Enable_Interrupts();

            Net_Buf_Create(&nBuf);
            Net_Buf_Prepend(nBuf, packet->buffer, packet->bufferLen,
                            NET_BUF_ALLOC_OWN);

            Eth_Dispatch(packet->device, nBuf);

            Free(packet);
        } else {
            Wait(&s_receiveThreadQueue);
            Enable_Interrupts();
        }
    }
}

void Init_Network_Devices(void) {
    KASSERT0(g_syscallTable[SYS_ETHPACKETSEND] == Sys_EthPacketSend,
             "Syscall table out of sync before net calls");
    KASSERT0(g_syscallTable[SYS_CLOSESOCKET] == Sys_CloseSocket,
             "Syscall table out of sync end of net calls");

    /* QEMU-emulated nics. */
    Register_Net_Device(&g_ne2000Capabilities, 0x300, 9, "eth");
    Register_Net_Device(&g_ne2000Capabilities, 0x320, 10, "eth");
    Register_Net_Device(&g_ne2000Capabilities, 0x340, 3, "eth");

    /* Start the receive kernel thread */
    Start_Kernel_Thread(Net_Device_Receive_Thread, 0, PRIORITY_NORMAL,
                        false, "{NetRecv}");
}


/* Public functions */
int Register_Net_Device(struct Net_Device_Capabilities *caps,
                        ulong_t baseAddr,
                        ulong_t irq, const char *nameBase) {
    int devNumber;
    int rc;
    struct Net_Device *device = NULL;

    device = Allocate_Net_Device();
    devNumber = Get_Next_Device_Number(nameBase);

    /* Copy name info */
    strcpy(device->name, caps->name);
    snprintf(device->devName, NET_NAME_SIZE, "%s%d", nameBase, devNumber);

    /* Base address and IRQ number */
    device->baseAddr = baseAddr;
    device->irq = irq;

    /* Copy over capabilities */
    device->init = caps->init;
    device->transmit = caps->transmit;
    device->receive = caps->receive;
    device->reset = caps->reset;
    device->getHeader = caps->getHeader;
    device->completeReceive = caps->completeReceive;

    /* Add the device to the device list */
    Add_To_Back_Of_Net_Device_List(&s_deviceList, device);

    Disable_Interrupts();

    /* initialize the device being registered */
    rc = device->init(device);

    Enable_Interrupts();

    if(rc != 0) {
        Remove_From_Net_Device_List(&s_deviceList, device);
        Destroy_Net_Device(device);
        return rc;
    }

    return 0;
}

int Unregister_Net_Device(struct Net_Device *dev) {

    /* Remove from device list */
    Remove_From_Net_Device_List(&s_deviceList, dev);

    /* Free memory */
    Destroy_Net_Device(dev);

    return 0;
}

int Get_Net_Device_By_Name(const char *name, struct Net_Device **device) {
    struct Net_Device *dev;

    for(dev = Get_Front_Of_Net_Device_List(&s_deviceList);
        dev != 0; dev = Get_Next_In_Net_Device_List(dev)) {
        if(strcmp(name, dev->devName) == 0) {
            *device = dev;
            return 0;
        }
    }

    return -1;
}

struct Net_Device_List *Get_Net_Device_List(void) {
    return &s_deviceList;
}

int Get_Net_Device_By_IRQ(unsigned int irq, /*@out@ */
                          struct Net_Device **device) {
    struct Net_Device *dev;
    for(dev = Get_Front_Of_Net_Device_List(&s_deviceList);
        dev != 0; dev = Get_Next_In_Net_Device_List(dev)) {
        if(dev->irq == irq) {
            *device = dev;
            return 0;
        }
    }
    return -1;
}

/* called by the device-specific code to notify of a ready packet */
int Net_Device_Receive(struct Net_Device *device, ushort_t ringBufferPage) {
    struct Net_Device_Packet *packet;
    struct Net_Device_Header hdr;
    ushort_t ringBufferOffset;
    ulong_t baseAddr = device->baseAddr;
    int rc = 0;

    KASSERT(!Interrupts_Enabled());

    //Print("Receiving packet in NET layer\n");

    device->getHeader(device, &hdr, ringBufferPage >> 8);

    packet = Malloc(sizeof(struct Net_Device_Packet));
    if(packet == 0)
        goto fail;

    memset(packet, '\0', sizeof(struct Net_Device_Packet));

    //Print("Packet addr on malloc: %p\n", packet);

    packet->device = device;
    packet->bufferLen = hdr.count - sizeof(struct Net_Device_Header);
    packet->buffer = Malloc(packet->bufferLen);
    if(packet->buffer == 0)
        goto fail;

    ringBufferOffset = ringBufferPage + sizeof(struct Net_Device_Header);

    device->receive(device, packet->buffer, packet->bufferLen,
                    ringBufferOffset);

    /* Add the packet to the back of the receive packet queue */
    Add_To_Back_Of_Net_Device_Receive_Packet_Queue(&s_receivePacketQueue,
                                                   packet);
    Wake_Up(&s_receiveThreadQueue);

    device->completeReceive(device, &hdr);
  fail:
    return rc;
}
