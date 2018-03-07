#include <geekos/syscall.h>
#include <geekos/errno.h>
#include <geekos/kthread.h>
#include <geekos/int.h>
#include <geekos/elf.h>
#include <geekos/malloc.h>
#include <geekos/screen.h>
#include <geekos/keyboard.h>
#include <geekos/string.h>
#include <geekos/user.h>
#include <geekos/timer.h>
#include <geekos/vfs.h>
#include <geekos/signal.h>
#include <geekos/sem.h>

#include <geekos/sys_net.h>
#include <geekos/projects.h>

#include <geekos/net/ethernet.h>
#include <geekos/net/netbuf.h>
#include <geekos/net/net.h>
#include <geekos/net/arp.h>
#include <geekos/net/ip.h>
#include <geekos/net/routing.h>
#include <geekos/net/socket.h>

/* allocates space for *pStr, sets it to a copy of the string passed in from user space */
extern int Copy_User_String(ulong_t uaddr, ulong_t len, ulong_t maxLen,
                            char **pStr);

/*
 * Send an ethernet packet
 * Params:
 *   state->ebx - address of user buffer of packet data
 *   state->ecx - length of user buffer
 *   state->edx - address of destination buffer
 *   state->esi - name of device/interface to send on
 *   state->edi - length of name of device/interface to send on
 */
extern int Sys_EthPacketSend(struct Interrupt_State *state) {
    uchar_t destAddress[6];
    struct Net_Device *device;
    int rc = 0;
    struct Net_Buf *nBuf;
    void *buffer = 0;

    ulong_t bufLength = MAX(state->ecx, ETH_MIN_DATA);
    if(bufLength > ETH_MAX_DATA) {
        rc = -1;
        goto fail;
    }

    buffer = Malloc(bufLength);
    if(buffer == 0)
        return ENOMEM;

    memset(buffer, '\0', bufLength);

    Copy_From_User(destAddress, state->edx, 6);
    Copy_From_User(buffer, state->ebx, state->ecx);

    char *device_name;
    Copy_User_String(state->esi, state->edi, 10, &device_name);
    rc = Get_Net_Device_By_Name(device_name, &device);
    Free(device_name);
    if(rc != 0)
        goto fail;

    rc = Net_Buf_Create(&nBuf);
    if(rc != 0)
        goto fail;

    rc = Net_Buf_Prepend(nBuf, buffer, bufLength, NET_BUF_ALLOC_COPY);
    if(rc != 0)
        goto fail;

    Enable_Interrupts();

    rc = Eth_Transmit(device, nBuf, destAddress, bufLength);
    if(rc != 0)
        goto fail;

    Net_Buf_Destroy(nBuf);

    Disable_Interrupts();

  fail:
    Free(buffer);

    return rc;
}


/*
 * Receive an ethernet packet
 * Params:
 *   state->ebx - address of user receive buffer
 *   state->ecx - length of user buffer
 */
extern int Sys_EthPacketReceive(struct Interrupt_State *state) {
    struct Net_Device *device;
    struct Net_Buf *nBuf;
    void *buffer;
    int rc = 0;

    rc = Get_Net_Device_By_Name("eth0", &device);
    if(rc != 0)
        goto fail;

    Enable_Interrupts();

    rc = Eth_Receive(device, &nBuf);

    Disable_Interrupts();       /* interrupts must be disabled at the point where this function returns */

    if(rc != 0)
        goto fail;

    buffer = Malloc(NET_BUF_SIZE(nBuf));
    if(buffer == 0) {
        rc = ENOMEM;
        goto fail;
    }

    rc = Net_Buf_Extract_All(nBuf, buffer);
    if(rc != 0) {
        Free(buffer);
        goto fail;
    }

    Copy_To_User(state->ebx, buffer, state->ecx);

    Net_Buf_Destroy(nBuf);

    Free(buffer);

  fail:

    KASSERT(!Interrupts_Enabled());
    return rc;
}

/*
 * Send an ARP request to an IP address
 * Params:
 *   state->ebx - Address of the target IP address
 *   state->ecx - Address of the receive buffer for the found MAC address
 *
 */
extern int Sys_Arp(struct Interrupt_State *state) {
    IP_Address ipAddress;
    MAC_Address macAddress;
    struct Net_Device *device;
    int rc = 0;

    rc = Get_Net_Device_By_Name("eth0", &device);
    if(rc != 0)
        goto fail;

    /* Copy the address from user space */
    Copy_From_User(ipAddress.ptr, state->ebx, sizeof(IP_Address));

    Enable_Interrupts();

    /* Find the hardware address using the ARP protocol */
    rc = ARP_Resolve_Address(device, ARP_HTYPE_ETH, ARP_PTYPE_IPV4,
                             ipAddress.ptr, macAddress);

    Disable_Interrupts();

    if(rc != 0)
        goto fail;

    Copy_To_User(state->ecx, macAddress, sizeof(macAddress));

  fail:
    return rc;
}


/*
 * Add a route to the routing table
 * Params
 *   state->ebx - address of 4 byte ip address
 *   state->ecx - address of 4 byte netmask
 *   state->edx - address of 4 byte gateway address
 *   state->esi - address of interface name
 *   state->edi - length of interface name (excludes NULL character)
 */
extern int Sys_RouteAdd(struct Interrupt_State *state) {
    IP_Address ipAddress;
    Netmask netmask;
    IP_Address gateway;
    char *interface;
    int rc = 0;

    rc = Copy_User_String(state->esi, state->edi, 1023, &interface);
    if(rc != 0)
        return rc;

    if(!Copy_From_User(ipAddress.ptr, state->ebx, 4)) {
        rc = EINVALID;
        goto fail;
    }

    if(!Copy_From_User(netmask.ptr, state->ecx, 4)) {
        rc = EINVALID;
        goto fail;
    }

    if(state->edx != 0) {
        if(!Copy_From_User(gateway.ptr, state->edx, 4)) {
            rc = EINVALID;
            goto fail;
        }

        Enable_Interrupts();
        rc = Net_Add_Route(&ipAddress, &netmask, &gateway, 0, interface);
    }

    else {
        Enable_Interrupts();
        rc = Net_Add_Route(&ipAddress, &netmask, NULL, 0, interface);
    }

    Disable_Interrupts();

    if(rc != 0) {
        goto fail;
    }

  fail:
    Free(interface);
    return rc;
}

/*
 * Delete a route from the routing table
 * Params:
 *   state->ebx - address of 4 byte ip address
 *   state->ecx - address of 4 byte netmask
 */
extern int Sys_RouteDel(struct Interrupt_State *state) {
    IP_Address ipAddress;
    Netmask netmask;
    int rc;

    if(!Copy_From_User(ipAddress.ptr, state->ebx, 4)) {
        rc = EINVALID;
        goto fail;
    }

    if(!Copy_From_User(netmask.ptr, state->ecx, 4)) {
        rc = EINVALID;
        goto fail;
    }

    Enable_Interrupts();
    rc = Net_Delete_Route(&ipAddress, &netmask);
    Disable_Interrupts();

    if(rc != 0)
        return rc;

  fail:
    return rc;
}

/*
 * Get the routing table
 * Params:
 *   state->ebx - address of structure buffer
 *   state->ecx - number of structures in buffer
 */
extern int Sys_RouteGet(struct Interrupt_State *state) {
    struct IP_Route *routes;
    int rc = 0;
    routes = Malloc(sizeof(struct IP_Route) * state->ecx);
    if(routes == NULL)
        return ENOMEM;

    Enable_Interrupts();
    TODO_P(PROJECT_ROUTING, "collect route table into routes");
    Disable_Interrupts();

    if(rc < 0)
        goto fail;

    if(!Copy_To_User(state->ebx, routes, sizeof(struct IP_Route) * rc)) {
        rc = EINVALID;
    }

  fail:

    Free(routes);
    return rc;
}

/*
 * Configure the IP address mapping to devices
 * Params:
 *   state->ebx - address of name of device
 *   state->ecx - length of the name excluding NULL character
 *   state->edx - address of 4 byte IP address
 *   state->esi - address of 4 byte subnet
 */
extern int Sys_IPConfigure(struct Interrupt_State *state) {
    char *interface;
    IP_Address ipAddress;
    Netmask netmask;
    bool fIpAddress = false;
    bool fNetmask = false;

    int rc = 0;

    rc = Copy_User_String(state->ebx, state->ecx, 1023, &interface);
    if(rc != 0)
        return rc;

    if(state->edx != 0) {
        if(!Copy_From_User(ipAddress.ptr, state->edx, 4)) {
            rc = EINVALID;
            goto fail;
        }
        fIpAddress = true;
    }

    if(state->esi != 0) {
        if(!Copy_From_User(netmask.ptr, state->esi, 4)) {
            rc = EINVALID;
            goto fail;
        }
        fNetmask = true;
    }

    rc = IP_Device_Configure(interface,
                             fIpAddress ? &ipAddress : NULL,
                             fNetmask ? &netmask : NULL);

  fail:
    Free(interface);
    return rc;
}

/*
 * Get information associated with each networking device
 * Params:
 *   state->ebx - address of structure buffer
 *   state->ecx - number of structures in buffer
 *   state->edx - address of interface name
 *   state->esi - length of interface name
 */
extern int Sys_IPGet(struct Interrupt_State *state) {

    char *interface = NULL;
    int rc = 0;
    struct IP_Device_Info *devInfo;

    if(state->edx != 0) {
        rc = Copy_User_String(state->edx, state->esi, 1023, &interface);
        if(rc != 0) {
            return rc;
        }
    }

    devInfo = Malloc(sizeof(struct IP_Device_Info) * state->ecx);
    if(devInfo == NULL)
        return ENOMEM;

    rc = IP_Device_Stat(devInfo, state->ecx, interface);
    if(rc < 0)
        goto fail;

    if(!Copy_To_User
       (state->ebx, devInfo, sizeof(struct IP_Device_Info) * rc)) {
        rc = EINVALID;
    }

  fail:
    Free(devInfo);

    if(interface != 0)
        Free(interface);

    return rc;
}

/*
 * Add a route to the routing table
 * Params
 *   state->ebx - address of 4 byte ip address
 *   state->ecx - string to send
 *   state->edx - length of string
 */
extern int Sys_IPSend(struct Interrupt_State *state) {
    IP_Address ipAddress;
    int rc = 0;
    char *string = NULL;
    struct Net_Buf *nBuf;

    if(!Copy_From_User(ipAddress.ptr, state->ebx, 4)) {
        rc = EINVALID;
        goto fail;
    }

    rc = Copy_User_String(state->ecx, state->edx, 1500, &string);
    if(rc != 0) {
        goto fail;
    }

    Enable_Interrupts();

    TODO_P(PROJECT_IP,
           "construct a buffer with the ip frame and transmit");

  fail:
    Free(string);

    Disable_Interrupts();

    return rc;
}

/*
 * Create a socket
 * Params
 *   state->ebx - type
 *   state->ecx - flags
 */
extern int Sys_Socket(struct Interrupt_State *state) {
    int rc;
    Enable_Interrupts();
    rc = Socket_Create((uchar_t) state->ebx, (int)state->ecx);
    Disable_Interrupts();
    return rc;
}

/*
 * Bind a socket
 * Params
 *   state->ebx - fd
 *   state->ecx - port
 *   state->edx - ip address
 */
extern int Sys_Bind(struct Interrupt_State *state) {
    IP_Address address;
    int rc;

    Copy_From_User(address.ptr, state->edx, 4);

    Enable_Interrupts();
    rc = Socket_Bind(state->ebx, (ushort_t) state->ecx, &address);
    Disable_Interrupts();

    return rc;
}

/*
 * Listen for an incoming connection
 * Params
 *   state->ebx - fd
 *   state->ecx - backlog
 */
extern int Sys_Listen(struct Interrupt_State *state) {
    int rc;

    Enable_Interrupts();
    rc = Socket_Listen(state->ebx, state->ecx);
    Disable_Interrupts();

    return rc;
}

/*
 * Accept an incoming connection
 * Params
 *   state->ebx - fd
 *   state->ecx - client port
 *   state->esi - client ip
 */
extern int Sys_Accept(struct Interrupt_State *state) {
    IP_Address ip;
    ushort_t port;
    int rc;

    Enable_Interrupts();
    rc = Socket_Accept(state->ebx, &ip, &port);
    Disable_Interrupts();

    if(rc >= 0) {
        Copy_To_User(state->esi, ip.ptr, 4);
        Copy_To_User(state->ecx, &port, sizeof(port));
    }

    return rc;
}

/*
 * Create a connection to a socket
 * Params
 *   state->ebx - fd
 *   state->ecx - port
 *   state->edx - ip address
 */
extern int Sys_Connect(struct Interrupt_State *state) {
    IP_Address address;
    int rc;

    Copy_From_User(address.ptr, state->edx, 4);

    Enable_Interrupts();
    rc = Socket_Connect(state->ebx, (ushort_t) state->ecx, &address);
    Disable_Interrupts();
    return rc;
}

/*
 * Send data to a remote socket
 * Params
 *   state->ebx - fd
 *   state->ecx - buffer
 *   state->edx - bufferSize
 *   state->esi -
 *   state->edi
 */
extern int Sys_Send(struct Interrupt_State *state) {
    int rc;

    uchar_t *buffer = Malloc(state->edx);
    if(buffer == 0)
        return ENOMEM;

    Copy_From_User(buffer, state->ecx, state->edx);

    Enable_Interrupts();
    rc = Socket_Send(state->ebx, buffer, state->edx);
    Disable_Interrupts();

    Free(buffer);

    return rc;
}

/*
 * Receive data from a socket
 * Params
 *   state->ebx - fd
 *   state->ecx - buffer
 *   state->edx - bufferSize
 *   state->esi -
 *   state->edi
 */
extern int Sys_Receive(struct Interrupt_State *state) {
    int rc;

    uchar_t *buffer = Malloc(state->edx);
    if(buffer == 0)
        return ENOMEM;

    Enable_Interrupts();
    rc = Socket_Receive(state->ebx, buffer, state->edx);
    Disable_Interrupts();

    if(rc > 0) {
        Copy_To_User(state->ecx, buffer, rc);
    }

    Free(buffer);

    return rc;
}

/*
 * Send data to a remote socket at the address
 * Params
 *   state->ebx - fd
 *   state->ecx - buffer
 *   state->edx - buffer size
 *   state->esi - port
 *   state->edi - ip address
 */
extern int Sys_SendTo(struct Interrupt_State *state) {
    TODO_P(PROJECT_SOCKETS, "sendto system call");
    return 0;
}

/*
 * Get the data's source
 * Params
 *   state->ebx - fd
 *   state->ecx - buffer
 *   state->edx - buffer size
 *   state->esi - port
 *   state->edi - ip address
 */
extern int Sys_ReceiveFrom(struct Interrupt_State *state) {
    TODO_P(PROJECT_SOCKETS, "recvfrom system call");
    return 0;
}

/*
 * Close a socket connection
 * Params
 *   state->ebx - socket fd
 *   state->ecx -
 *   state->edx -
 *   state->esi -
 *   state->edi
 */
extern int Sys_CloseSocket(struct Interrupt_State *state) {
    int rc;

    Enable_Interrupts();
    rc = Socket_Close(state->ebx);
    Disable_Interrupts();

    return rc;
}
