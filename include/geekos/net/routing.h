#ifndef ROUTING_H
#define ROUTING_H

#include <geekos/defs.h>
#include <geekos/ktypes.h>
#include <geekos/net/net.h>
#include <geekos/net/netbuf.h>
#include <geekos/net/ip.h>
#include <geekos/list.h>

#define NET_ROUTE_UP 1
#define NET_ROUTE_GATEWAY 2

struct Route;

DEFINE_LIST(Routing_Table, Route);

struct Route {
    IP_Address destination;
    Netmask netmask;
    IP_Address gateway;
    struct IP_Device *interface;
    uint_t prefixLength;
    uint_t metric;
    uint_t ticks;

     DEFINE_LINK(Routing_Table, Route);

    ushort_t flags;

};

IMPLEMENT_LIST(Routing_Table, Route);

extern void Init_Routing(void);
extern int Net_Add_Route(IP_Address * destination, Netmask * mask,
                         IP_Address * gateway, int metric,
                         char *interface);
extern int Net_Delete_Route(IP_Address * destination, Netmask * netmask);
extern int Net_Clean_Routes(ulong_t msecs);
extern int Net_Get_Route_Info(struct IP_Device **device,
                              IP_Address * ipAddress,
                              IP_Address * network, Netmask * subnet,
                              bool * fGateway, IP_Address * gateway);
extern int Net_Get_Route(struct IP_Device **device,
                         IP_Address * ipAddress, bool * fGateway,
                         IP_Address * gateway);
extern int Net_Get_Route_Table(struct IP_Route *table,
                               ulong_t maxEntries);
extern int Net_Route_Get_Metric(IP_Address * ipAddress, Netmask * subnet,
                                int *metric);
extern int Net_Get_Route_Table_Entry(const IP_Address * ipAddress,
                                     const Netmask * netmask,
                                     struct IP_Route *route);

#endif
