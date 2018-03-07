/*
 * echoclnt.c
 *
 * Copyright (c) 2009  <calvin@cs.umd.edu>
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

#include <socket.h>
#include <conio.h>
#include <string.h>
#include <ip.h>

int main(int argc, char **argv) {
    int fd;
    char buffer[256];
    char buffer2[256];
    int rc;
    uchar_t ipAddress[4];

    // Parse the IP Address

    fd = Socket(SOCK_STREAM, 0);
    if(fd < 0) {
        Print("Could not create socket\n");
        return fd;
    }

    if(argc < 2) {
        Print("Expected IP address as first argument\n");
        return -1;
    }

    Parse_IP(argv[1], ipAddress);

    // Connect to server
    rc = Connect(fd, 7, ipAddress);
    if(rc != 0) {
        Print("Could not connect to %s:7\n", argv[1]);
        return -1;
    }

    Print("CONNECTED TO THE SERVER\n");

    Print("Enter a word: ");

    Read_Line(buffer, 256);

    rc = Send(fd, (uchar_t *) buffer, 256);

    rc = Receive(fd, (uchar_t *) buffer2, 256);
    if(rc > 0) {
        Print("We received %d bytes!\n", rc);

        Print("%s\n", buffer2);
    }

    Close_Socket(fd);

    return 0;
}
