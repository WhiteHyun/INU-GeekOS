/* Interface definitions for bget.c, the memory management package.
 *
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

#if defined (GEEKOS)

// Adapted for geekos: http://www.cs.umd.edu/~daveho/geekos/
// Original version of BGET downloaded from: http://www.fourmilab.ch/bget/
// $Revision: 1.1 $

// GeekOS changes are (mostly) confined to #if defined (GEEKOS)
// sections.

// Yes, we have prototypes :-)
#define PROTOTYPES

#endif // defined (GEEKOS)

#ifndef _
#ifdef PROTOTYPES
#define  _(x)  x                /* If compiler knows prototypes */
#else
#define  _(x)  ()               /* It it doesn't */
#endif /* PROTOTYPES */
#endif

typedef long bufsize;
void bpool _((void *buffer, bufsize len));
void *bget _((bufsize size));
void *bgetz _((bufsize size));
void *bgetr _((void *buffer, bufsize newsize));
void brel _((void *buf));
void bectl _((int (*compact) (bufsize sizereq, int sequence),
              void *(*acquire) (bufsize size),
              void (*release) (void *buf), bufsize pool_incr));
void bstats _((bufsize * curalloc, bufsize * totfree, bufsize * maxfree,
               long *nget, long *nrel));
void bstatse _((bufsize * pool_incr, long *npool, long *npget,
                long *nprel, long *ndget, long *ndrel));
void bufdump _((void *buf));
void bpoold _((void *pool, int dumpalloc, int dumpfree));
int bpoolv _((void *pool));
