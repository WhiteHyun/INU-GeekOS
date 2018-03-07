/*
 * Network
 * Copyright (c) 2009, Calvin Grunewald
 * $Revision: 1.0 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

//#include <geekos/net/net.h>
#include <string.h>
#include <geekos/syscall.h>
#include <geekos/ktypes.h>
#include <geekos/net/ip.h>      /* struct IP_Route */

DEF_SYSCALL(EthPacketSend, SYS_ETHPACKETSEND, int,
            (const void *buffer, ulong_t length, const uchar_t dest[],
             const char *device_name), const void *arg0 = buffer;
            ulong_t arg1 = length;
            const uchar_t * arg2 = dest;
            const char *arg3 = device_name;
            ulong_t arg4 = strlen(device_name);
            , SYSCALL_REGS_5)

DEF_SYSCALL(EthPacketReceive, SYS_ETHPACKETRECEIVE, int,
                (void *buffer, ulong_t length), void *arg0 = buffer;
            ulong_t arg1 = length;
            , SYSCALL_REGS_2)

DEF_SYSCALL(Arp, SYS_ARP, int,
                (uchar_t * ipAddress, uchar_t * macAddress),
            uchar_t * arg0 = ipAddress;
            uchar_t * arg1 = macAddress;
            , SYSCALL_REGS_2)

DEF_SYSCALL(Route_Add, SYS_ROUTEADD, int,
                (uchar_t * ipAddress, uchar_t * netmask,
                 uchar_t * gateway, char *ifaceName, ulong_t len),
            uchar_t * arg0 = ipAddress;
            uchar_t * arg1 = netmask;
            uchar_t * arg2 = gateway;
            char *arg3 = ifaceName;
            ulong_t arg4 = len;
            , SYSCALL_REGS_5)

DEF_SYSCALL(Route_Delete, SYS_ROUTEDEL, int,
                (uchar_t * ipAddress, uchar_t * netmask),
            uchar_t * arg0 = ipAddress;
            uchar_t * arg1 = netmask;
            , SYSCALL_REGS_2)

DEF_SYSCALL(IP_Configure, SYS_IPCONFIGURE, int,
                (char *name, ulong_t len, uchar_t * ipAddress,
                 uchar_t * subnet), char *arg0 = name;
            ulong_t arg1 = len;
            uchar_t * arg2 = ipAddress;
            uchar_t * arg3 = subnet;
            , SYSCALL_REGS_4)

DEF_SYSCALL(Get_Routes, SYS_ROUTEGET, int,
                (struct IP_Route * buffer, ulong_t count),
            void *arg0 = buffer;
            ulong_t arg1 = count;
            , SYSCALL_REGS_2)

DEF_SYSCALL(Get_IP_Info, SYS_IPGET, int,
                (struct IP_Device_Info * buffer, ulong_t count,
                 char *interface, ulong_t ifaceLen), void *arg0 = buffer;
            ulong_t arg1 = count;
            char *arg2 = interface;
            ulong_t arg3 = ifaceLen;
            , SYSCALL_REGS_4)

DEF_SYSCALL(IP_Send, SYS_IPSEND, int,
                (uchar_t * ipAddress, char *message,
                 ulong_t messageLength), uchar_t * arg0 = ipAddress;
            char *arg1 = message;
            ulong_t arg2 = messageLength;
            , SYSCALL_REGS_3)

bool Parse_IP(const char *ip, uchar_t * ipBuffer) {
    char buffer[100];
    char *curr = buffer;
    char *numBegin = buffer;
    memcpy(buffer, ip, 100);
    int i = 0;
    bool done = false;

    for(i = 0; i < 4; ++i) {
        // find the first dot
        while (*curr != '\0' && *curr != '.') {
            ++curr;
        }

        if(*curr == '\0' && i != 3)
            return false;
        else if(*curr == '\0' && i == 3)
            done = true;

        *curr = '\0';

        // TODO - Need to disambiguate between atoi return of 0 for a valid number 0 versus
        //        a return of 0 in error
        ipBuffer[i] = atoi(numBegin);

        ++curr;
        numBegin = curr;
    }

    if(!done)
        return false;

    return true;
}
