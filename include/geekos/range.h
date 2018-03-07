/*
 * Range checking
 * Copyright (c) 2003, David H. Hovemeyer <daveho@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.7 $
 * 
 */

#ifndef GEEKOS_RANGE_H
#define GEEKOS_RANGE_H

#include <geekos/ktypes.h>

/*
 * TODO: think about these, make sure they're correct
 */

/**
 * Check that given range is "proper".
 * @param start the start of the range
 * @param size the size of the range
 * @return true if range is properly within the space
 *   0(inclusive)..ULONG_MAX(inclusive)
 */
static __inline__ bool Check_Range_Proper(ulong_t start, ulong_t size) {
    /*
     * Wrap around is only permitted if the sum is zero.
     * E.g., start=ULONG_MAX, size==1 is a valid range.
     */
    ulong_t sum = start + size;
    return start <= sum || (sum == 0);
}

/**
 * Check that given range lies entirely under the
 * maximum value specified (exclusive).
 * @param start the start of the range
 * @param size the size of the range
 * @param max the lowest address NOT allowed to be in the range
 * @return true if range falls entirely within the range
 *   0(inclusive)..max(exclusive)
 */
static __inline__ bool
Check_Range_Under(ulong_t start, ulong_t size, ulong_t max) {
    if(!Check_Range_Proper(start, size))
        return false;

    return start < max && (start + size) <= max;
}

#endif /* GEEKOS_RANGE_H */
