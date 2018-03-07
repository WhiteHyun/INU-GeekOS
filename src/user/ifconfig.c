#include <net.h>
#include <string.h>
#include <conio.h>
#include <ip.h>

#define NUMBER_OF_DEVICES 10

static char addressCommand[] = "address";
static char netmaskCommand[] = "netmask";

int main(int argc, char **argv) {
    if(argc == 1 || argc == 2) {
        struct IP_Device_Info deviceInfo[NUMBER_OF_DEVICES];
        int i;
        char *interface = (argc == 1) ? NULL : argv[1];
        int rc =
            Get_IP_Info(deviceInfo, NUMBER_OF_DEVICES, interface,
                        strlen(interface));
        if(rc < 0)
            return rc;

        for(i = 0; i < rc; ++i) {
            struct IP_Device_Info *device = deviceInfo + i;
            Print("%-10s", device->name);
            Print("HWaddr %02x:%02x:%02x:%02x:%02x:%02x\n",
                  device->mac[0],
                  device->mac[1],
                  device->mac[2],
                  device->mac[3], device->mac[4], device->mac[5]);
            Print("          inet addr:%d.%d.%d.%d  Mask:%d.%d.%d.%d\n",
                  device->ipAddress.ptr[0],
                  device->ipAddress.ptr[1],
                  device->ipAddress.ptr[2],
                  device->ipAddress.ptr[3],
                  device->netmask.ptr[0],
                  device->netmask.ptr[1],
                  device->netmask.ptr[2], device->netmask.ptr[3]);
            Print("          RX packets:%ld  errors:%ld\n",
                  device->rxPackets, device->rxPacketErrors);
            Print("          TX packets:%ld  errors:%ld\n",
                  device->txPackets, device->txPacketErrors);
            Print("          RX bytes:%ld  TX bytes:%ld\n",
                  device->rxBytes, device->txBytes);
            Print("          Interrupt: %d  I/O Port: %lx\n",
                  device->interrupt, device->ioport);
            Print("\n\n");
        }

        return 0;
    } else if(argc == 4) {
        int rc;

        uchar_t buffer[4];
        bool result = Parse_IP(argv[3], buffer);
        if(!result) {
            Print("Invalid address entered\n");
            return 1;
        }


        if(strcmp(argv[2], addressCommand) == 0) {
            rc = IP_Configure(argv[1], strlen(argv[1]), buffer, NULL);
        } else if(strcmp(argv[2], netmaskCommand) == 0) {
            rc = IP_Configure(argv[1], strlen(argv[1]), NULL, buffer);
        } else {
            Print("Invalid command specified\n");
            return 1;
        }

        return rc;
    }
    return 1;
}
