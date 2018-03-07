/*
 * Internet Protocol Definitions - Used with user level code
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.00 $
 * 
 */

#ifndef USER_IP_H
#define USER_IP_H

#include <geekos/net/ipdefs.h>

int Route_Add(uchar_t *, uchar_t *, uchar_t *, char *, ulong_t);
int Route_Delete(uchar_t *, uchar_t *);
int IP_Configure(char *, ulong_t, uchar_t *, uchar_t *);
int Get_Routes(struct IP_Route *buffer, ulong_t numRoutes);
int Get_IP_Info(struct IP_Device_Info *buffer, ulong_t count,
                char *interface, ulong_t ifaceNameLength);
bool Parse_IP(const char *ip, uchar_t * ipBuffer);
int IP_Send(uchar_t * ipAddress, char *message, ulong_t messageLength);

#endif
