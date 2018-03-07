/* todo: remove "send eof" stuff */

#include <socket.h>
#include <conio.h>
#include <string.h>
#include <net.h>
#include <ip.h>

#define SEND_BUFFER_SIZE 2000

static int sendBuffer[SEND_BUFFER_SIZE];

int main(int argc __attribute__ ((unused)), char **argv
         __attribute__ ((unused))) {
    uchar_t address[4];
    int rc, fd;
    int bytes;

    fd = Socket(SOCK_STREAM, 0);
    if(fd < 0) {
        Print("Could not create socket\n");
        return fd;
    }

    Parse_IP(argv[1], address);

    // Connect to server
    rc = Connect(fd, 4600, address);
    if(rc != 0) {
        Print("Could not connect to %s:4600\n", argv[1]);
        return -1;
    }

    int i;
    int curValue = 0;
    bytes = atoi(argv[2]);
    while (bytes != 0) {
        int bytesToSend = MIN(bytes, SEND_BUFFER_SIZE / 4);
        for(i = 0; i < bytesToSend; ++i)
            sendBuffer[i] = curValue++;

        rc = Send(fd, sendBuffer, bytesToSend * 4);
        if(rc < 0) {
            Print("Could not send bytes\n");
            return -1;
        }

        bytes -= bytesToSend;
    }

    // Send the EOF
    sendBuffer[0] = -1;

    rc = Send(fd, sendBuffer, 4);
    if(rc != 0) {
        Print("Could not send EOF\n");
    }

    Close_Socket(fd);

    return 0;
}
