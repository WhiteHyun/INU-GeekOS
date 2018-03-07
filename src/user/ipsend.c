#include <ip.h>
#include <net.h>
#include <conio.h>
#include <string.h>

int main(int argc, char **argv) {
    int rc;
    uchar_t ipAddress[4];

    if(argc != 3) {
        Print("Usage:\n%s <ip_address> <message>\n", argv[0]);
        return 1;
    }

    Parse_IP(argv[1], ipAddress);

    rc = IP_Send(ipAddress, argv[2], strlen(argv[2]));
    if(rc != 0) {
        Print("IP packet transmit failed\n");
    }

    return rc;

}
