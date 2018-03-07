#include <geekos/net/routing.h>
#include <geekos/kassert.h>
#include <geekos/screen.h>
#include <geekos/malloc.h>
#include <geekos/errno.h>
#include <geekos/net/ipdefs.h>
#include <geekos/kthread.h>
#include <geekos/net/ip.h>
#include <geekos/int.h>
#include <geekos/synch.h>
#include <geekos/timer.h>

#include <geekos/projects.h>

static struct Routing_Table s_routingTable;
static struct Mutex s_routingTableMutex;


void Init_Routing(void) {
    // Initialize the routing table with all local routes
    struct IP_Device *ipDevice;
    int rc;

    Mutex_Init(&s_routingTableMutex);

    TODO_P(PROJECT_ROUTING,
           "generate routes for directly attached IP devices");
}

int Net_Add_Route(IP_Address * destination, Netmask * mask,
                  IP_Address * gateway, int metric, char *interface) {


    Mutex_Lock(&s_routingTableMutex);

    TODO_P(PROJECT_ROUTING, "add route to the table");

    Mutex_Unlock(&s_routingTableMutex);

    return 0;
}

int Net_Delete_Route(IP_Address * destination, Netmask * netmask) {

    Mutex_Lock(&s_routingTableMutex);
    TODO_P(PROJECT_ROUTING, "delete this route");

    Mutex_Unlock(&s_routingTableMutex);

    return -1;
}

/*
 * Clean gatewayed routes
 */
int Net_Clean_Routes(ulong_t msecs) {
    int cleanedRoutes = 0;
    Mutex_Lock(&s_routingTableMutex);

    TODO_P(PROJECT_ROUTING, "remove stale routes");
    Mutex_Unlock(&s_routingTableMutex);

    return cleanedRoutes;
}
