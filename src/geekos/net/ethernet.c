/*
 * Ethernet Protocol
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/screen.h>
#include <geekos/int.h>
#include <geekos/malloc.h>
#include <geekos/errno.h>
#include <geekos/net/ethernet.h>
#include <geekos/projects.h>
// #include <geekos/net/protocol.h>

/* table that maps ethernet types to protocol handlers.  should map 0x800 to ip, 0x806 to arp. */
/* dispatch upper layer protocols based on ethernet type (was length) field */
struct Ethernet_Dispatch_Entry;
DEFINE_LIST(Ethernet_Dispatch_Table, Ethernet_Dispatch_Entry);
struct Ethernet_Dispatch_Entry {
    ushort_t type;
    int (*dispatcher) (struct Net_Device * received_on,
                       struct Net_Buf * packet_received);
     DEFINE_LINK(Ethernet_Dispatch_Table, Ethernet_Dispatch_Entry);
};
IMPLEMENT_LIST(Ethernet_Dispatch_Table, Ethernet_Dispatch_Entry);
static struct Ethernet_Dispatch_Table s_ethDispatchTable;
/* end dispatch table declaration and definition */

#define DEBUG_ETH(x...) Print("Eth: " x)
// #define DEBUG_ETH(x...) 


int Eth_Transmit(struct Net_Device *device, struct Net_Buf *nBuf,
                 uchar_t * destAddr, ushort_t type) {
    struct Ethernet_Header header;
    int rc;

    KASSERT(Interrupts_Enabled());

    /* all you have to do in this function is fill in the header. */
    TODO_P(PROJECT_RAW_ETHERNET,
           "construct the ethernet header for the destination, this device's address, and the type.");

    rc = Net_Buf_Prepend(nBuf, &header, sizeof(header),
                         NET_BUF_ALLOC_COPY);
    if(rc != 0)
        return rc;

    ulong_t size = MAX(NET_BUF_SIZE(nBuf), ETH_MIN_DATA);       /* buffer size must be at least ETH_MIN_DATA, 
                                                                   even if we don't use it. */

    KASSERT0(size >= ETH_MIN_DATA, "input to Eth_Transmit should be at least ETH_MIN_DATA long");       /* paranoia. */

    void *buffer = Malloc(size);
    if(buffer == 0)
        return ENOMEM;

    rc = Net_Buf_Extract_All(nBuf, buffer);
    if(rc != 0) {
        Free(buffer);
        return rc;
    }

    Disable_Interrupts();
    device->transmit(device, buffer, size);
    Enable_Interrupts();

    return 0;
}

int Eth_Dispatch(struct Net_Device *device, struct Net_Buf *nBuf) {
    struct Ethernet_Header header;
    int rc;

    rc = Net_Buf_Extract(nBuf, 0x00, &header, sizeof(header));
    if(rc != 0)
        return rc;

    DEBUG_ETH("Trying to remove the header... \n");

    rc = Net_Buf_Remove(nBuf, 0x00, sizeof(header));
    if(rc != 0) {
        DEBUG_ETH("Failed to remove the header.\n");
        return rc;
    }

    if(ntohs(header.type) <= 1500) {
        /* treat as raw ethernet; this is the largest part of this assignment, approx ten lines. */
        TODO_P(PROJECT_RAW_ETHERNET,
               "awaken a thread waiting for this frame, if any");
        return 0;
    } else {
        struct Ethernet_Dispatch_Entry *curr;
        for(curr =
            Get_Front_Of_Ethernet_Dispatch_Table(&s_ethDispatchTable);
            curr != NULL && curr->type != ntohs(header.type);
            curr = Get_Next_In_Ethernet_Dispatch_Table(curr)) ;
        if(curr) {
            DEBUG_ETH("Ethernet dispatching packet to %d protocol\n",
                      ntohs(header.type));
            curr->dispatcher(device, nBuf);
            return 0;
        }

        Print("Destroying netbuf in ethernet layer\n");
        Net_Buf_Destroy(nBuf);

        return -1;
    }

}

/* called by the system call to read raw ethernet.  not really needed except 
   as driver (testing) code. */
int Eth_Receive(struct Net_Device *device, /*@out@ */
                struct Net_Buf **nBuf) {
    /* This is the complementary part to the receive path. */
    TODO_P(PROJECT_RAW_ETHERNET,
           "sleep until a frame arrives, ensure that an nBuf is put in the second argument");
    return 0;
}

int Eth_Dispatch_Table_Add(ushort_t type,
                           int (*dispatcher) (struct Net_Device *,
                                              struct Net_Buf *)) {
    struct Ethernet_Dispatch_Entry *ethType =
        Malloc(sizeof(struct Ethernet_Dispatch_Entry));

    if(ethType == NULL)
        return ENOMEM;

    ethType->type = type;
    ethType->dispatcher = dispatcher;

    Add_To_Back_Of_Ethernet_Dispatch_Table(&s_ethDispatchTable, ethType);
    return 0;
}

void Init_Ethernet() {
    TODO_P(PROJECT_RAW_ETHERNET, "initialization");
}
