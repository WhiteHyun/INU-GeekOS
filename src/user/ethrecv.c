#include <net.h>
#include <conio.h>
#include <string.h>

int main() {
    uchar_t buffer[ETH_MAX_DATA];
    int rc = 0;

    memset(buffer, '\0', ETH_MAX_DATA);

    rc = EthPacketReceive(buffer, ETH_MAX_DATA);

    if(rc == 0)
        Print("Received: %s\n", buffer);
    else
        Print("Receive failed\n");

    return rc;

}
