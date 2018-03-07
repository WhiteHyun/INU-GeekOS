#include <socket.h>
#include <conio.h>
#include <string.h>

int main() {
    int rc;
    char buffer[256];

    memset(buffer, 0, 256);

    int fd = Socket(SOCK_STREAM, 0);
    if(fd < 0) {
        Print("Could not create socket\n");
        return fd;
    }
    // Bind to an address
    rc = Bind(fd, 7, INADDR_ANY);
    if(rc != 0) {
        Print("Could not do a multihomed bind to port 7\n");
        return -1;
    }
    // Listen for a connection
    rc = Listen(fd, 10);
    if(rc != 0) {
        Print("Could not listen for a connection\n");
        return 1;
    }

    do {

        int newSocket;
        uchar_t clientAddress[4];
        ushort_t clientPort;

        // Accept the connection
        newSocket = Accept(fd, &clientPort, clientAddress);
        if(newSocket < 0) {
            Print("Could not accept the connection\n");
            return 1;
        }

        rc = Receive(newSocket, (uchar_t *) buffer, 256);
        if(rc >= 0) {
            Print("Echoing data back\n");
            Send(newSocket, (uchar_t *) buffer, 256);
        }

        Close_Socket(newSocket);

    } while (strcmp(buffer, "exit") != 0);

    return 0;
}
