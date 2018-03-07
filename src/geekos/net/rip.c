/*
 * RIP Protocol
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/net/rip.h>
#include <geekos/alarm.h>
#include <geekos/net/routing.h>
#include <geekos/kthread.h>
#include <geekos/synch.h>
#include <geekos/screen.h>
#include <geekos/net/ipdefs.h>

#include <geekos/projects.h>

#define DEBUG_RIP(x...) Print("RIP: " x)


void Init_RIP(void) {
    TODO_P(PROJECT_RIP,
           "start threads, setup state, set alarm to timeout routes");
}
