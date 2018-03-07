/*
 * Memmove implementation, taken from the web (had no license)
 * 
 * This is free software.  You are permitted to use,
 * redistribute, and modify it as specified in the file "COPYING".
 */

#include <string.h>

void *memmove(void *dest1, const void *source1, size_t length) {
    char *dest = dest1;
    const char *source = source1;
    char *d0 = dest;
    if(source < dest)
        /* Moving from low mem to hi mem; start at end.  */
        for(source += length, dest += length; length; --length)
            *--dest = *--source;
    else if(source != dest) {
        /* Moving from hi mem to low mem; start at beginning.  */
        for(; length; --length)
            *dest++ = *source++;
    }
    return (void *)d0;
}
