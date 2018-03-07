/*
 * Bit set data structure
 * Copyright (c) 2001,2003,2004 David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2003,2013,2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 */

#include <geekos/kassert.h>
#include <geekos/malloc.h>
#include <geekos/bitset.h>
#include <geekos/string.h>
#include <libc/conio.h>

#define FIND_OFFSET_AND_BIT(bitPos,offset,bit)	\
do {						\
    offset = bitPos / 8;			\
    bit = bitPos % 8;				\
} while (0)

#define FIND_NUM_BYTES(totalBits) \
    ((totalBits / 8) + ((totalBits % 8 != 0) ? 1 : 0))

void *Create_Bit_Set(uint_t totalBits) {
    ulong_t numBytes;
    void *bitSet;

    numBytes = FIND_NUM_BYTES(totalBits);

    bitSet = Malloc(numBytes);
    if(bitSet != 0)
        memset(bitSet, '\0', numBytes);
    else
        Print("malloc failed\n");

    return bitSet;
}

void Set_Bit(void *bitSet, uint_t bitPos) {
    ulong_t offset, bit;

    FIND_OFFSET_AND_BIT(bitPos, offset, bit);
    ((uchar_t *) bitSet)[offset] |= (1 << bit);
}

void Clear_Bit(void *bitSet, uint_t bitPos) {
    ulong_t offset, bit;

    FIND_OFFSET_AND_BIT(bitPos, offset, bit);
    ((uchar_t *) bitSet)[offset] &= ~(1 << bit);
}

bool Is_Bit_Set(void *bitSet, uint_t bitPos) {
    ulong_t offset, bit;

    FIND_OFFSET_AND_BIT(bitPos, offset, bit);
    return (((uchar_t *) bitSet)[offset] & (1 << bit)) != 0;
}

int Find_First_Free_Bit(void *bitSet, ulong_t totalBits) {
    uint_t numBytes = FIND_NUM_BYTES(totalBits);
    ulong_t offset;
    uchar_t *bits = (uchar_t *) bitSet;

    for(offset = 0; offset < numBytes; ++offset) {
        if(bits[offset] != 0xff) {
            uint_t bit;
            for(bit = 0; bit < 8; ++bit) {
                if((bits[offset] & (1 << bit)) == 0)
                    return (offset * 8) + bit;
            }
#ifdef GEEKOS
            /* kassert only defined properly when running in geekos. */
            KASSERT(false);
#endif
        }
    }

    return -1;
}

/*
 * This is slow!!
 */
int Find_First_N_Free(void *bitSet, uint_t runLength, ulong_t totalBits) {
    uint_t i, j;

    for(i = 0; i < totalBits - runLength; i++) {
        if(!Is_Bit_Set(bitSet, i)) {
            for(j = 1; j < runLength; j++) {
                if(Is_Bit_Set(bitSet, i + j)) {
                    break;
                }
            }
            if(j == runLength) {
                return i;
            }
        }
    }
    return -1;
}

void Destroy_Bit_Set(void *bitSet) {
    Free(bitSet);
}
