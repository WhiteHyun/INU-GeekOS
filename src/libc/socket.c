/*
 * Socket Interface
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <socket.h>
#include <string.h>
#include <geekos/syscall.h>

uchar_t INADDR_ANY[4] = { 0, 0, 0, 0 };
uchar_t INADDR_BROADCAST[4] = { 255, 255, 255, 255 };

DEF_SYSCALL(Socket, SYS_SOCKET, int, (uchar_t type, int flags),
            ulong_t arg0 = type;
            ulong_t arg1 = flags;
            , SYSCALL_REGS_2)
DEF_SYSCALL(Connect, SYS_CONNECT, int,
                (ulong_t id, ushort_t port, uchar_t ipAddress[4]),
            ulong_t arg0 = id;
            ulong_t arg1 = port;
            void *arg2 = ipAddress;
            , SYSCALL_REGS_3)
DEF_SYSCALL(Accept, SYS_ACCEPT, int,
                (ulong_t id, ushort_t * port, uchar_t ipAddress[4]),
            ulong_t arg0 = id;
            void *arg1 = port;
            void *arg2 = ipAddress;
            , SYSCALL_REGS_3)
DEF_SYSCALL(Listen, SYS_LISTEN, int, (ulong_t id, ulong_t backlog),
            ulong_t arg0 = id;
            ulong_t arg1 = backlog;
            , SYSCALL_REGS_2)
DEF_SYSCALL(Bind, SYS_BIND, int,
                (ulong_t id, ushort_t port, uchar_t ipAddress[4]),
            ulong_t arg0 = id;
            ulong_t arg1 = port;
            void *arg2 = ipAddress;
            , SYSCALL_REGS_3)
DEF_SYSCALL(Receive, SYS_RECEIVE, int,
                (ulong_t id, void *buffer, ulong_t bufferSize),
            ulong_t arg0 = id;
            void *arg1 = buffer;
            ulong_t arg2 = bufferSize;
            , SYSCALL_REGS_3)
DEF_SYSCALL(Send, SYS_SEND, int,
                (ulong_t id, void *buffer, ulong_t bufferSize),
            ulong_t arg0 = id;
            void *arg1 = buffer;
            ulong_t arg2 = bufferSize;
            , SYSCALL_REGS_3)
DEF_SYSCALL(Send_To, SYS_SENDTO, int,
                (ulong_t id, uchar_t * buffer, ulong_t bufferSize,
                 ushort_t port, uchar_t ipAddress[4]), ulong_t arg0 = id;
            void *arg1 = buffer;
            ulong_t arg2 = bufferSize;
            ulong_t arg3 = port;
            void *arg4 = ipAddress;
            , SYSCALL_REGS_5)
DEF_SYSCALL(Receive_From, SYS_RECEIVEFROM, int,
                (ulong_t id, uchar_t * buffer, ulong_t bufferSize,
                 ushort_t * port, uchar_t ipAddress[4]), ulong_t arg0 =
            id;
            void *arg1 = buffer;
            ulong_t arg2 = bufferSize;
            void *arg3 = port;
            void *arg4 = ipAddress;
            , SYSCALL_REGS_5)
DEF_SYSCALL(Close_Socket, SYS_CLOSESOCKET, int, (ulong_t id),
            ulong_t arg0 = id;
            , SYSCALL_REGS_1)
