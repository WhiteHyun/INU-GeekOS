#include <net.h>
#include <string.h>
#include <conio.h>
#include <ip.h>

#define NUMBER_OF_ROUTES 25

static char addCommand[] = "add";
static char delCommand[] = "del";

static char destinationOption[] = "dest";
static char gatewayOption[] = "gw";
static char netmaskOption[] = "netmask";
static char deviceOption[] = "dev";

int main(int argc, char **argv) {
    /* Print out the routing table */
    if(argc == 1) {
        int i, rc;
        struct IP_Route routes[NUMBER_OF_ROUTES];
        char buffer[16];
        rc = Get_Routes(routes, NUMBER_OF_ROUTES);
        if(rc < 0) {
            Print("Cannot display routing table\n");
            return rc;
        }

        Print("%-16s%-16s%-16s%-6s%-8s%-10s\n",
              "Destination", "Gateway", "Netmask", "Flags", "Metric",
              "Iface");

        for(i = 0; i < rc; ++i) {
            struct IP_Route *route = routes + i;


            /* Format the destination */
            snprintf(buffer, 16, "%d.%d.%d.%d",
                     route->destination.ptr[0],
                     route->destination.ptr[1],
                     route->destination.ptr[2],
                     route->destination.ptr[3]);
            Print("%-16s", buffer);


            /* Format the Gateway */
            if(route->fGateway == 1) {
                snprintf(buffer, 16, "%d.%d.%d.%d",
                         route->gateway.ptr[0],
                         route->gateway.ptr[1],
                         route->gateway.ptr[2], route->gateway.ptr[3]);
                Print("%-16s", buffer);
            } else {
                Print("%-16s", "*");
            }

            /* Format the netmask */
            snprintf(buffer, 16, "%d.%d.%d.%d",
                     route->netmask.ptr[0],
                     route->netmask.ptr[1],
                     route->netmask.ptr[2], route->netmask.ptr[3]);
            Print("%-16s", buffer);

            /* Format the flags */
            *buffer = '\0';
            if(route->fUp == 1) {
                strcat(buffer, "U");
            }
            if(route->fGateway == 1) {
                strcat(buffer, "G");
            }

            Print("%-6s", buffer);

            /* Format the metric */
            Print("%-8d", route->metric);

            /* Format the interface */
            Print("%s\n", route->interface);

        }

        Print("\n");
        return 0;
    }


    else if(argc > 2) {
        /* Add a route to the table */
        if(strcmp(argv[1], addCommand) == 0 && !(argc & 1)) {
            uchar_t destination[4];
            uchar_t netmask[4];
            uchar_t gateway[4];
            char *device = NULL;
            bool fDestination = false;
            bool fNetmask = false;
            bool fGateway = false;
            bool fDevice = false;
            int rc;

            int index = 2;
            while (index < argc) {
                if(strcmp(argv[index], destinationOption) == 0) {
                    fDestination = true;
                    rc = Parse_IP(argv[index + 1], destination);
                    if(!rc) {
                        Print("IP destination address invalid\n");
                        return rc;
                    }

                    index += 2;
                } else if(strcmp(argv[index], gatewayOption) == 0) {
                    fGateway = true;
                    rc = Parse_IP(argv[index + 1], gateway);
                    if(!rc) {
                        Print("IP gateway address invalid\n");
                        return rc;
                    }

                    index += 2;
                } else if(strcmp(argv[index], netmaskOption) == 0) {
                    fNetmask = true;
                    rc = Parse_IP(argv[index + 1], netmask);
                    if(!rc) {
                        Print("Netmask address invalid\n");
                        return rc;
                    }

                    index += 2;
                } else if(strcmp(argv[index], deviceOption) == 0) {
                    fDevice = true;
                    device = argv[index + 1];
                    index += 2;
                } else {
                    Print("Unreconzied route option");
                    return 1;
                }




            }

            if(!fDestination || !fNetmask || !fDevice) {
                Print("Not enough information provided to the command\n");
                return 0;
            }


            rc = Route_Add(destination, netmask,
                           (fGateway ? gateway : NULL), device,
                           strlen(device));
            if(rc != 0) {
                Print("Route add failed\n");
                return rc;
            }

            return 0;

        }

        /* Remove a route from the table */
        else if(strcmp(argv[1], delCommand) == 0) {
            uchar_t destination[4];
            uchar_t netmask[4];
            bool fDestination = false;
            bool fNetmask = false;
            int rc, index = 2;

            if(argc != 6) {
                Print
                    ("Invalid number of parameters to delete route command\n");
                return 1;
            }

            while (index != argc) {
                if(strcmp(argv[index], destinationOption) == 0) {
                    fDestination = true;
                    rc = Parse_IP(argv[index + 1], destination);
                    if(!rc) {
                        Print("IP destination address invalid\n");
                        return rc;
                    }

                    index += 2;
                } else if(strcmp(argv[index], netmaskOption) == 0) {
                    fNetmask = true;
                    rc = Parse_IP(argv[index + 1], netmask);
                    if(!rc) {
                        Print("Netmask address invalid\n");
                        return rc;
                    }

                    index += 2;
                }
            }

            if(!fDestination || !fNetmask) {
                Print("Not enough information provided to command\n");
                return 1;
            }

            rc = Route_Delete(destination, netmask);
            if(rc != 0) {
                Print("Route delete command failed\n");
                return rc;
            }

            return 0;

        }

        else {
            Print("Unreconized command options\n");
            return 1;
        }
    }

    Print("Invalid command\n");
    return 1;
}
