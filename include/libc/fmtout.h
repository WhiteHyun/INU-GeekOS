/*
 * Generalized support for printf()-style formatted output
 * Copyright (c) 2004, David H. Hovemeyer <daveho@cs.umd.edu>
 * Copyright (c) 2013,2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 *
 * $Revision: 1.1 $
 *
 */

#ifndef OUTPUT_H
#define OUTPUT_H

#include <stdarg.h>

/*
 * An output sink for Format_Output().
 * Useful for emitting formatted output to a function
 * or a buffer.
 */
struct Output_Sink {
    /*
     * Emit a single character of output.
     * This is called for all output characters,
     * in order.
     */
    void (*Emit) (struct Output_Sink * o, int ch);

    /*
     * Finish the formatted output. Called after all characters
     * have been emitted.
     */
    void (*Finish) (struct Output_Sink * o);
};

int Format_Output(struct Output_Sink *q, const char *format, va_list ap);

#endif /* OUTPUT_H */
