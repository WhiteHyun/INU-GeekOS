/*
 * Internet Protocol Definitions - Used with user level code
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#ifndef IPDEFS_H
#define IPDEFS_H

#include <geekos/ktypes.h>
#include <geekos/defs.h>

#define NET_NAME_SIZE 32

typedef union IP_Address {
    uchar_t ptr[4];
    uint_t address;
} IP_Address;

typedef union Netmask {
    uchar_t ptr[4];
    uint_t mask;
} Netmask;

struct IP_Route {
    IP_Address destination;
    Netmask netmask;
    IP_Address gateway;
    int metric;
    uint_t ticks;
    char interface[NET_NAME_SIZE];


    uchar_t fGateway:1;
    uchar_t fUp:1;
};

struct IP_Device_Info {
    IP_Address ipAddress;
    Netmask netmask;
    IP_Address gateway;
    char name[NET_NAME_SIZE];

    /* Error tracking */
    ulong_t rxPackets;
    ulong_t txPackets;
    ulong_t rxPacketErrors;
    ulong_t txPacketErrors;
    ulong_t rxBytes;
    ulong_t txBytes;


    ulong_t ioport;
    uchar_t interrupt;

    uchar_t mac[6];



};

#endif
