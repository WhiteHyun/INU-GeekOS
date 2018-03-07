#include <net.h>
#include <conio.h>
#include <string.h>

int main(int argc, char **argv) {

    const unsigned char dest[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    if(argc == 1) {
        Print("\tUsage: %s string\n", argv[0]);
        return 1;
    }

    int rc = EthPacketSend(argv[1], strlen(argv[1]), dest, "eth0");
    return rc;
}
