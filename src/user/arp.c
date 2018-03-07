/*
 * Copyright (c) 2001,2003,2004 Neil Spring <nsping@cs.umd.edu>
 * Copyright (c) 2003,2013,2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 */

#include <net.h>
#include <conio.h>
#include <string.h>
#include <geekos/errno.h>
#include <ip.h>

int main(int argc, char **argv) {
    bool ipValid;
    uchar_t ipAddress[4];
    uchar_t macAddress[6];
    int rc = 0;
    int i;

    if(argc != 2) {
        Print("Usage:\n\t%s ip_address\n", argv[0]);
        return -1;
    }

    ipValid = Parse_IP(argv[1], ipAddress);
    if(!ipValid) {
        Print("IP Address %s not valid\n", argv[1]);
        return -2;
    }

    rc = Arp(ipAddress, macAddress);
    if(rc == ETIMEOUT) {
        Print("ARP timed out\n");
        return rc;
    } else if(rc != 0) {
        Print("ARP failed with error code %d\n", rc);
        return rc;
    }

    Print("Found MAC address: ");
    for(i = 0; i < 6; ++i) {
        Print("%x:", macAddress[i]);
    }

    Print("\n");


    return 0;
}
