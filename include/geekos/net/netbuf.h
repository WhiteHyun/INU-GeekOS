/*
 * Network buffer
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#ifndef _NET_BUF_H_
#define _NET_BUF_H_

#include <geekos/string.h>
#include <geekos/list.h>
#include <geekos/defs.h>

#define NET_BUF_ALLOC_OWN 0x0
#define NET_BUF_ALLOC_LEND 0x1
#define NET_BUF_ALLOC_COPY 0x2

struct Message_Buffer;

DEFINE_LIST(Message_Buffer_List, Message_Buffer);

struct Message_Buffer {
    unsigned int length;
    void *buffer;
     DEFINE_LINK(Message_Buffer_List, Message_Buffer);
    uchar_t mustFree:1;
    uchar_t valid:1;
};

struct Net_Buf;

DEFINE_LIST(Packet_Queue, Net_Buf);

struct Net_Buf {
    ulong_t length;             /* sum of the length of the buffers */
    struct Message_Buffer_List buffers;
     DEFINE_LINK(Packet_Queue, Net_Buf);

#ifndef NDEBUG
    ulong_t mallocCount;
#endif
};

#ifdef GEEKOS
IMPLEMENT_LIST(Message_Buffer_List, Message_Buffer);
IMPLEMENT_LIST(Packet_Queue, Net_Buf);
#endif

/* Public functions */

#define NET_BUF_SIZE(net_buf) (net_buf->length)

/* Creation */
int Net_Buf_Create(struct Net_Buf **);

/* Deletion */
int Net_Buf_Destroy(struct Net_Buf *);

/* adding to the front or back of the buffer */
int Net_Buf_Prepend(struct Net_Buf *, void *, ulong_t, uchar_t);
int Net_Buf_Append(struct Net_Buf *, void *, ulong_t, uchar_t);

/* Extract data from the net buffer */
int Net_Buf_Extract(struct Net_Buf *, ulong_t start, void *dest, ulong_t);
int Net_Buf_Extract_All(struct Net_Buf *, void *dest);

/* Remove data from the net buffer */
int Net_Buf_Remove(struct Net_Buf *, ulong_t start, ulong_t length);
int Net_Buf_Remove_All(struct Net_Buf *);

#endif
