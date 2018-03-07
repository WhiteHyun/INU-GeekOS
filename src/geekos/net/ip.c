/*
 * Internet Protocol
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */
#include <geekos/net/ip.h>
#include <geekos/string.h>
#include <geekos/errno.h>
#include <geekos/kassert.h>
#include <geekos/malloc.h>
#include <geekos/net/arp.h>
#include <geekos/net/routing.h>
#include <geekos/net/ethernet.h>
#include <geekos/int.h>
#include <geekos/net/udp.h>
#include <geekos/net/tcp.h>
#include <geekos/net/socket.h>
#include <geekos/net/net.h>

#include <geekos/projects.h>

static uchar_t s_baseIpAddress[] = { 169, 254, 0, 0 };
static uchar_t s_baseSubnet[] = { 255, 255, 255, 0 };

static union IP_Address s_inaddrAny = {.address = INADDR_ANY };
static union IP_Address s_inaddrBroadcast = {.address =
        INADDR_BROADCAST };

static int IP_Device_Get_By_IP(struct IP_Device **device,
                               IP_Address * address);

/* dispatch table declaration and definition. (callbacks for each protocol type) */
struct IP_Dispatch_Table_Entry;
DEFINE_LIST(IP_Dispatch_Table, IP_Dispatch_Table_Entry);
struct IP_Dispatch_Table_Entry {
    ushort_t type;
    int (*dispatcher) (struct IP_Device *,
                       IP_Address *, IP_Address *, struct Net_Buf *);
     DEFINE_LINK(IP_Dispatch_Table, IP_Dispatch_Table_Entry);
};
IMPLEMENT_LIST(IP_Dispatch_Table, IP_Dispatch_Table_Entry);
static struct IP_Dispatch_Table s_ipDispatchTable;


static struct IP_Device_List s_ipDeviceList;


int IP_Dispatch_Table_Add(ushort_t type,
                          int (*dispatcher) (struct IP_Device *,
                                             IP_Address *, IP_Address *,
                                             struct Net_Buf *)) {
    struct IP_Dispatch_Table_Entry *ipType =
        Malloc(sizeof(struct IP_Dispatch_Table_Entry));
    if(ipType == NULL)
        return ENOMEM;

    ipType->type = type;
    ipType->dispatcher = dispatcher;

    Add_To_Back_Of_IP_Dispatch_Table(&s_ipDispatchTable, ipType);
    return 0;

}

static int IP_Deliver(struct IP_Device *device, ushort_t protocol,
                      IP_Address * destAddress, IP_Address * srcAddress,
                      struct Net_Buf *nBuf) {
    struct IP_Dispatch_Table_Entry *curr;
    for(curr = Get_Front_Of_IP_Dispatch_Table(&s_ipDispatchTable);
        curr != NULL && curr->type != protocol;
        curr = Get_Next_In_IP_Dispatch_Table(curr)) ;
    if(curr) {
        curr->dispatcher(device, destAddress, srcAddress, nBuf);
        return 0;
    }
    Print("IP packet received - Destroying netbuf in ip layer\n");
    return -1;
}

/* construct our IP address implicitly from the last two bytes of the hardware address */
static int IP_Get_Address(struct Net_Device *device,
                          IP_Address * ipAddress) {
    /* we're going to use the autoconfig addresses to start, maybe dhcp provided ones later. */
    ipAddress->ptr[0] = s_baseIpAddress[0];
    ipAddress->ptr[1] = s_baseIpAddress[1];
    ipAddress->ptr[2] = device->devAddr[device->addrLength - 2];
    ipAddress->ptr[3] = device->devAddr[device->addrLength - 1];
    return 0;
}

static int IP_Broadcast(struct IP_Header *ipHeader, struct Net_Buf *nBuf) {
    // Send the packet out on all local interfaces
    // note that broadcast IP is sent with ttl=1 to prevent badness 
    // and that broadcast IP is not forwarded (redundant safety measures)
    TODO_P(PROJECT_IP,
           "broadcast helper function (send on all interfaces if needed)");
    return 0;
}




static void Forwarding_Thread(ulong_t arg __attribute__ ((unused))) {
    TODO_P(PROJECT_IP,
           "take packets from forward/transmit queue and send em");
}


int IP_Device_Register(struct Net_Device *device, IP_Address * address,
                       Netmask * subnet) {
    struct IP_Device *ipDevice = NULL;
    ipDevice = Malloc(sizeof(struct IP_Device));
    if(ipDevice == NULL)
        return ENOMEM;

    ipDevice->netDevice = device;
    ipDevice->ipAddress = *address;
    ipDevice->subnet = *subnet;

    Add_To_Back_Of_IP_Device_List(&s_ipDeviceList, ipDevice);
    return 0;
}


int IP_Device_Get_By_Name(struct IP_Device **device, char *name) {
    struct IP_Device *curr;
    for(curr = Get_Front_Of_IP_Device_List(&s_ipDeviceList);
        curr != NULL && strcmp(curr->netDevice->devName, name);
        curr = Get_Next_In_IP_Device_List(curr)) ;
    if(curr) {
        *device = curr;
        return 0;
    }
    return -1;
}

static int IP_Device_Get_By_IP(struct IP_Device **device,
                               IP_Address * address) {
    struct IP_Device *curr;
    for(curr = Get_Front_Of_IP_Device_List(&s_ipDeviceList);
        curr != NULL && curr->ipAddress.address != address->address;
        curr = Get_Next_In_IP_Device_List(curr)) ;
    if(curr) {
        *device = curr;
        return 0;
    }
    return -1;
}

int IP_Device_Configure(char *name, IP_Address * ipAddress,
                        Netmask * netmask) {
    struct IP_Device *device = NULL;
    int rc = IP_Device_Get_By_Name(&device, name);
    if(rc != 0) {
        return rc;
    }

    KASSERT(device != NULL);

    if(ipAddress != NULL) {
        device->ipAddress = *ipAddress;
    }

    if(netmask != NULL) {
        device->subnet = *netmask;
    }

    return 0;
}

int IP_Device_Stat(struct IP_Device_Info *info, ulong_t deviceCount,
                   char *name) {
    unsigned int counter = 0;
    TODO_P(PROJECT_IP, "ifconfig device statistics");
    return counter;
}

void Init_IP(void) {
    struct Net_Device *curr = NULL;
    int rc = 0;

    for(curr = Get_Front_Of_Net_Device_List(Get_Net_Device_List());
        curr != NULL; curr = Get_Next_In_Net_Device_List(curr)) {
        IP_Address ipAddress;
        Netmask subnet;

        IP_Get_Address(curr, &ipAddress);
        memcpy(subnet.ptr, s_baseSubnet, 4);

        rc = IP_Device_Register(curr, &ipAddress, &subnet);
        KASSERT0(rc == 0, "unable to register IP device");      // only will get here if malloc fails
    }

    TODO_P(PROJECT_IP, "add to ethernet dispatch table");

    TODO_P(PROJECT_UDP, "add UDP to IP dispatch table, if doing UDP");
    TODO_P(PROJECT_TCP, "add TCP to IP dispatch table, if doing TCP");

    Start_Kernel_Thread(Forwarding_Thread, 0, PRIORITY_NORMAL, false,
                        "{Forwarding}");

}
