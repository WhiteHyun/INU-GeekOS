/*
 * Create and extract the command line argument block for a process
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
 *
 * $Revision: 1.8 $
 * 
 */

#ifndef GEEKOS_ARGBLOCK_H
#define GEEKOS_ARGBLOCK_H

/**
 * Header struct for accessing argument block from user mode.
 * Just cast the address of the argument block passed by
 * the kernel to a pointer to this struct.
 */
struct Argument_Block {
    int argc;
    char **argv;
};

#ifdef GEEKOS

/*
 * Functions used by the kernel to create the argument block.
 */
void Get_Argument_Block_Size(const char *command, unsigned *numArgs,
                             ulong_t * argBlockSize);
void Format_Argument_Block(char *argBlock, unsigned numArgs,
                           ulong_t userAddress, const char *command);

#endif

#endif /* GEEKOS_ARGBLOCK_H */
