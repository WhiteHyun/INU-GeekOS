#include <net.h>
#include <conio.h>
#include <string.h>

int main(int argc, char **argv) {
    uchar_t buffer[ETH_MAX_DATA];
    int packetSize;
    int numPackets;
    int i, j;

    uchar_t destAddr[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    if(argc != 3) {
        Print("Usage:\n\t%s packet_size num_packets\n", argv[0]);
        return 1;
    }

    memset(buffer, '\0', ETH_MAX_DATA);

    packetSize = atoi(argv[1]);
    numPackets = atoi(argv[2]);

    if(packetSize < ETH_MIN_DATA)
        packetSize = ETH_MIN_DATA;

    else if(packetSize > ETH_MAX_DATA - 1)
        packetSize = ETH_MAX_DATA - 1;

    Print("Packet size: %d\n", packetSize);
    Print("Num packets: %d\n", numPackets);

    for(i = 0; i < numPackets; ++i) {
        for(j = 0; j < packetSize; ++j) {
            buffer[j] = 'a' + (i % 26);
        }
        EthPacketSend(buffer, packetSize + 1, destAddr, "eth0");
    }

    return 0;

}
