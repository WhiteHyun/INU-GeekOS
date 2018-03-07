/*
 * Bit set data structure
 * Copyright (c) 2003,2013,2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 * Copyright (c) 2003 David H. Hovemeyer <daveho@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.14 $
 * 
 */

#ifndef GEEKOS_BITSET_H
#define GEEKOS_BITSET_H

#include <geekos/ktypes.h>

void *Create_Bit_Set(uint_t totalBits);
void Set_Bit(void *bitSet, uint_t bitPos);
void Clear_Bit(void *bitSet, uint_t bitPos);
bool Is_Bit_Set(void *bitSet, uint_t bitPos);
int Find_First_Free_Bit(void *bitSet, ulong_t totalBits);       /* -1 if not found */
int Find_First_N_Free(void *bitSet, uint_t runLength, ulong_t totalBits);       /* -1 if not found */
void Destroy_Bit_Set(void *bitSet);

#if 0
struct Bit_Set {
    int size;
    uchar_t bits[0];            /* Note: unwarranted chumminess with compiler */
};

struct Bit_Set *Create_Bit_Set(uchar_t * bits, int totalBits);
int Set_Bit(struct Bit_Set *set, int bitPos);
int Clear_Bit(struct Bit_Set *set, int bitPos);
int Is_Bit_Set(struct Bit_Set *set, int bitPos);
int Find_First_Free_Bit(struct Bit_Set *set);
int Find_First_N_Free(struct Bit_Set *set, int runLength);
uchar_t *Get_Bits(struct Bit_Set *set);
#endif

#endif
