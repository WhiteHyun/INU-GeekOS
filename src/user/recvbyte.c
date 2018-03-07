/*
 * recvbyte.c
 *
 *  Created on: May 5, 2009
 *      Author: calvin
 */

/* todo: this is broken in deciding when to be done. bufferIndex, for example */

#include <socket.h>
#include <conio.h>
#include <string.h>
#include <net.h>
#include <ip.h>

#define MAX_BUFFER_SIZE 8192

static int receiveBuffer[MAX_BUFFER_SIZE];

int main(int argc __attribute__ ((unused)), char **argv
         __attribute__ ((unused))) {
    int fd = Socket(SOCK_STREAM, 0);
    if(fd < 0) {
        return -1;
    }

    int rc = Bind(fd, 4600, INADDR_ANY);
    if(rc != 0) {
        return -1;
    }

    rc = Listen(fd, 1);
    if(rc != 0) {
        return -1;
    }

    ushort_t port;
    uchar_t address[4];

    int ns = Accept(fd, &port, address);
    if(ns < 0) {
        return -1;
    }

    Print("Socket is connected\n");

    int bytesRead;
    int bufferPtr = 0;
    int bufferIndex = 0;        /* possibly sketchy */

    do {
        bytesRead =
            Receive(ns, ((uchar_t *) receiveBuffer) + bufferPtr, 256);
        if(bytesRead < 0) {
            Print("Receive failed\n");
            Close_Socket(ns);
            return -1;
        }

        bufferPtr += bytesRead;
        bufferIndex += bytesRead / 4;
        //Print("Buffer Index: %d, Buffer Value: %d\n", bufferIndex, receiveBuffer[bufferIndex - 1]);

    }
    while (receiveBuffer[bufferIndex - 1] != -1);

    Close_Socket(ns);

    Close_Socket(fd);

    return 0;
}
