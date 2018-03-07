/*
 * Network buffer
 * Copyright (c) 2009, Calvin Grunewald <cgrunewa@umd.edu>
 * $Revision: 1.00 $
 *
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/net/netbuf.h>
#include <geekos/errno.h>
#include <geekos/malloc.h>
#include <geekos/ktypes.h>

/* Private Functions */

static struct Message_Buffer *Find_Buffer_At_Offset(struct Net_Buf *nBuf,
                                                    ulong_t offset,
                                                    ulong_t * bufOffset) {

    struct Message_Buffer *curr;
    ulong_t seenBytes = 0;

    if(NET_BUF_SIZE(nBuf) <= offset) {
        Print("buffer too small for offset\n");
        return NULL;
    }

    for(curr = Get_Front_Of_Message_Buffer_List(&nBuf->buffers);
        curr != 0; curr = Get_Next_In_Message_Buffer_List(curr)) {

        /* If a buffer isn't valid, pretend it doesn't exist */
        while (curr && !curr->valid) {
            curr = Get_Next_In_Message_Buffer_List(curr);
        }
        if(!curr)
            return NULL;        /* we ran out, so don't use an invalid pointer. */

        if(offset < seenBytes + curr->length) {
            if(bufOffset != NULL) {
                *bufOffset = offset - seenBytes;
            }
            return curr;
        }

        seenBytes += curr->length;
    }
    Print("buffer at offset not found?");
    return NULL;
}

/*
 * Split
 *
 * Returns the message buffer that starts at the address specified by
 *   the split parameter. For example, if we split at 0x200, the returned
 *   message buffer will contain the data starting at 0x200.
 */
static int Split_Buffer(struct Net_Buf *nBuf, ulong_t split) {

    struct Message_Buffer *curr, *buf1, *buf2;
    ulong_t offset = 0, buf1Size, buf2Size;

    if(split == NET_BUF_SIZE(nBuf))
        return 0;

    if(split > NET_BUF_SIZE(nBuf)) {
        Print("Split out of range");
        return -1;
    }

    curr = Find_Buffer_At_Offset(nBuf, split, &offset);
    if(curr == NULL) {
        Print("didn't find buffer at %lu\n", split);
        return -1;
    }                           /* not found */
    if(offset == 0)
        return 0;               /* already done */

    /*
     * Get the previous message buffer to use as
     * a reference. Be careful though, prev may be NULL
     */
    // unused prev = Get_Prev_In_Message_Buffer_List(curr);

    /* Allocate two new message buffers */
    buf1 = Malloc(sizeof(struct Message_Buffer));
    if(buf1 == 0)
        return ENOMEM;

#ifndef NDEBUG
    nBuf->mallocCount++;
#endif

    buf2 = Malloc(sizeof(struct Message_Buffer));
    if(buf2 == 0)
        return ENOMEM;

#ifndef NDEBUG
    nBuf->mallocCount++;
#endif


    /* Change pointers for first buffer */
    buf1Size = offset;
    buf1->buffer = curr->buffer;
    buf1->length = buf1Size;
    buf1->valid = 1;
    buf1->mustFree = curr->mustFree;    // only free if we needed to free the original buffer

    /* Change pointers for second buffer */
    buf2Size = (curr->length - offset);
    buf2->buffer = curr->buffer + buf1Size;
    buf2->length = buf2Size;
    buf2->valid = 1;
    buf2->mustFree = 0;

    curr->valid = 0;
    curr->mustFree = 0;

    /* Add the two new message buffers */
    Insert_Into_Message_Buffer_List(&nBuf->buffers, curr, buf1);
    Insert_Into_Message_Buffer_List(&nBuf->buffers, buf1, buf2);

    return 0;

}

/* Public Functions */

int Net_Buf_Create(struct Net_Buf **nBuf) {
    struct Net_Buf *buffer = Malloc(sizeof(struct Net_Buf));
    if(buffer == 0)
        return ENOMEM;

    memset(buffer, '\0', sizeof(struct Net_Buf));



    *nBuf = buffer;

    return 0;
}

int Net_Buf_Destroy(struct Net_Buf *nBuf) {

    Net_Buf_Remove_All(nBuf);
    Free(nBuf);

    return 0;
}

static int Create_Message_Buffer(struct Net_Buf *nBuf,
                                 struct Message_Buffer **mBufPtr,
                                 void *buffer, ulong_t size,
                                 uchar_t allocScheme) {

    struct Message_Buffer *mBuf;

    mBuf = Malloc(sizeof(struct Message_Buffer));
    if(mBuf == 0)
        return ENOMEM;

#ifndef NDEBUG
    nBuf->mallocCount++;
#endif

    memset(mBuf, '\0', sizeof(struct Message_Buffer));


    if(allocScheme == NET_BUF_ALLOC_COPY) {
        void *bufferCopy = Malloc(size);
        if(bufferCopy == 0)
            return ENOMEM;

#ifndef NDEBUG
        nBuf->mallocCount++;
#endif


        memcpy(bufferCopy, buffer, size);

        mBuf->length = size;
        mBuf->buffer = bufferCopy;
        mBuf->mustFree = 1;
        mBuf->valid = 1;
    } else if(allocScheme == NET_BUF_ALLOC_OWN) {
        mBuf->length = size;
        mBuf->buffer = buffer;
        mBuf->mustFree = 1;
        mBuf->valid = 1;

#ifndef NDEBUG
        nBuf->mallocCount++;
#endif
    } else if(allocScheme == NET_BUF_ALLOC_LEND) {
        mBuf->length = size;
        mBuf->buffer = buffer;
        mBuf->mustFree = 0;
        mBuf->valid = 1;
    } else {
        /* Should only be one of the three options */
        KASSERT(false);
    }

    *mBufPtr = mBuf;
    return 0;
}

int Net_Buf_Prepend(struct Net_Buf *nBuf, void *buffer, ulong_t size,
                    uchar_t allocScheme) {

    struct Message_Buffer *mBuf;
    int rc = 0;

    KASSERT0(size > 0, "size of prepended buffer should be nonzero");

    rc = Create_Message_Buffer(nBuf, &mBuf, buffer, size, allocScheme);
    if(rc != 0)
        return rc;

    Add_To_Front_Of_Message_Buffer_List(&nBuf->buffers, mBuf);

    nBuf->length += size;

    return rc;
}

int Net_Buf_Append(struct Net_Buf *nBuf, void *buffer, ulong_t size,
                   uchar_t allocScheme) {
    struct Message_Buffer *mBuf;
    int rc = 0;

    KASSERT0(size > 0, "size of appended  buffer should be nonzero");

    rc = Create_Message_Buffer(nBuf, &mBuf, buffer, size, allocScheme);
    if(rc != 0)
        return rc;

    Add_To_Back_Of_Message_Buffer_List(&nBuf->buffers, mBuf);

    nBuf->length += size;

    return 0;
}

int Net_Buf_Extract(struct Net_Buf *nBuf, ulong_t start, void *dest,
                    ulong_t size) {
    ulong_t bytesLeft = size;
    ulong_t bufOffset;
    ulong_t destOffset = 0;
    ulong_t numBytes;

    struct Message_Buffer *startBuf, *currBuf;

    if(start + size > NET_BUF_SIZE(nBuf))
        return -1;

    /* Find the message buffer containing the start address */
    startBuf = Find_Buffer_At_Offset(nBuf, start, &bufOffset);

    /* Copy data from start to end of the message buffer to the destination */
    numBytes = MIN(bytesLeft, startBuf->length - bufOffset);
    memcpy(dest, startBuf->buffer + bufOffset, numBytes);
    bytesLeft -= numBytes;
    destOffset += numBytes;

    if(bytesLeft == 0)
        return 0;

    /* For each buffer up until the one containing the end address */
    /* Copy the entire buffer into the destination */
    currBuf = Get_Next_In_Message_Buffer_List(startBuf);

    while (bytesLeft != 0) {
        if(!currBuf->valid) {
            currBuf = Get_Next_In_Message_Buffer_List(currBuf);
            continue;
        }

        KASSERT(currBuf != NULL);

        numBytes = MIN(bytesLeft, currBuf->length);
        memcpy(dest + destOffset, currBuf->buffer, numBytes);
        bytesLeft -= numBytes;
        destOffset += numBytes;

        currBuf = Get_Next_In_Message_Buffer_List(currBuf);
    }

    return 0;
}

int Net_Buf_Extract_All(struct Net_Buf *nBuf, void *dest) {
    return Net_Buf_Extract(nBuf, 0, dest, NET_BUF_SIZE(nBuf));
}

int Net_Buf_Remove(struct Net_Buf *nBuf, ulong_t start, ulong_t length) {

    struct Message_Buffer *splitBegin, *splitEnd, *next;

    int rc = 0;

    KASSERT0(nBuf, "asked to remove from null nBuf");

    if(length == 0)
        return -1;

    /* Split the buffer where we need to delete stuff */
    rc = Split_Buffer(nBuf, start);
    if(rc != 0) {
        Print("failed to split buffer at start of removal segment: %d\n",
              rc);
        return rc;
    }

    rc = Split_Buffer(nBuf, start + length);
    if(rc != 0) {
        Print("failed to split buffer at end of removal segment: %d\n",
              rc);
        return rc;
    }

    splitBegin = Find_Buffer_At_Offset(nBuf, start, NULL);
    splitEnd = Find_Buffer_At_Offset(nBuf, start + length, NULL);

    /* Iterate from split begin up until split end, deleting buffers */
    /* Important! SplitEnd may be NULL, so in that case, we remove from
     * split begin all the way to the end of the buffer */

    while (splitBegin != splitEnd) {
        next = Get_Next_In_Message_Buffer_List(splitBegin);
        splitBegin->valid = 0;
        splitBegin = next;
    }

    /* Decrement the length of the net buffer */
    nBuf->length -= length;


    return 0;
}

int Net_Buf_Remove_All(struct Net_Buf *nBuf) {
    struct Message_Buffer *curr, *next;

#ifndef NDEBUG
    ulong_t freeCount = 0;
#endif

    curr = Get_Front_Of_Message_Buffer_List(&nBuf->buffers);
    while (curr != 0) {
        next = Get_Next_In_Message_Buffer_List(curr);

        if(curr->mustFree) {
            Free(curr->buffer);
#ifndef NDEBUG
            freeCount++;
#endif
        }

        Free(curr);

#ifndef NDEBUG
        freeCount++;
#endif

        curr = next;
    }

    Clear_Message_Buffer_List(&nBuf->buffers);

#ifndef NDEBUG
    KASSERT(freeCount == nBuf->mallocCount);
    //Print("Deallocating nBuf -> no memory leads\n");
#endif

    return 0;
}
