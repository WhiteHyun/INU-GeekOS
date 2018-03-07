/*
 * GeekOS memory allocation API
 * Copyright (c) 2001, David H. Hovemeyer <daveho@cs.umd.edu>
 * $Revision: 1.13 $
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <geekos/screen.h>
#include <geekos/int.h>
#include <geekos/bget.h>
#include <geekos/kassert.h>
#include <geekos/malloc.h>
#include <geekos/lock.h>
#include <string.h>


/*
 * Initialize the heap starting at given address and occupying
 * specified number of bytes.
 */
void Init_Heap(ulong_t start, ulong_t size) {
    Print("Creating kernel heap: start=%lx, size=%ld\n", start, size);
    bpool((void *)start, size);
}

/*
 * Dynamically allocate a buffer of given size.
 * Returns null if there is not enough memory to satisfy the
 * allocation.
 */
void *Malloc(ulong_t size) {
    bool iflag;
    void *result;

    KASSERT(size > 0);

    iflag = Begin_Int_Atomic();
    result = bget(size);
    End_Int_Atomic(iflag);
    return result;
}

/*
 * Free a buffer allocated with Malloc().
 */
void Free(void *buf) {
    bool iflag;
    KASSERT0((((unsigned int)buf) & 0x3) == 0,
             "attempt to free a corrupted pointer (wasn't four-byte aligned).");

    iflag = Begin_Int_Atomic();
    brel(buf);
    End_Int_Atomic(iflag);
}
