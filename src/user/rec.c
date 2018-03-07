/*
 * A user mode program which uses deep recursion
 * to test stack growth.
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

/* this is a test version of the program, which checks to
   ensure that the user program itself is loaded properly,
   including all pages, including pages that should be
   zeroed before getting assigned to our process.  */

#include <conio.h>
#include <string.h>
#include <process.h>
#include <libuser.h>            /* for Malloc in the integrity check */

#define PARANOIA

unsigned int Quiet = 0;

extern void *_Entry(), *_end;
extern void *Malloc();
// extern void *snprintf();

int integrity_check_enabled;

int check_static() {
    static int f;
    if(f == 0) {
        return 1;
    } else {
        return 0;
    }
}


void Check_Integrity_Or_Die() {
    static unsigned int stored_code_checksum;
    static void *top;

    if(!integrity_check_enabled) {
        return;
    }

    if(top == 0x0) {
        top = Malloc;
        if(top < (void *)snprintf) {
            top = snprintf;
        }
    }

    int *p;
    unsigned int working_checksum = 0;
    if((void *)_Entry >= top) {
        Print("Code Integrity Check assumption failed.\n");
        if((void *)_Entry >= (void *)0x1000) {
            Print("Detail: _Entry %p.\n", &_Entry);
        }
    }
    if((void *)top <= (void *)0x2000) {
        Print("Detail: top %p.\n", top);
    }
    if((void *)top >= (void *)0x3000) {
        Print("Detail: top %p.\n", top);
    }
    /* the following can trigger if pages don't get bzeroed,
       at least based on student-reported symptoms.  I don't
       understand the mechanism by which Malloc / snprintf
       acquire an apparent value so large because of it.  */
    if(top > (void *)&top) {
        Print
            ("Code Integrity Check assumption failed: may fault accessing pages above the text (code) and globals.\n");
        Print("Detail: top %p exceeds &top %p.\n", top, &top);
    }
    if((char *)top < (char *)&top - 0x2000) {
        static int warned;
        if(!warned) {
            // Print("Code Integrity Check is Weak.\n");
            Print("Detail: top %p way lower than &top %p.\n", top, &top);
            warned = true;
        }
    }

    if((void *)_Entry < (void *)0x1000) {
        Print("Code Integrity Check Internal error _Entry too small: %p",
              _Entry);
        integrity_check_enabled = false;
        return;
    }

    for(p = (int *)_Entry; p < (int *)top; p++) {
        working_checksum += *p;
    }
    if(stored_code_checksum == 0) {
        stored_code_checksum = working_checksum;
    } else if(stored_code_checksum != working_checksum) {
        Print("Code Integrity Check Failed.\n");
        Print("Detail: Code Integrity Check Failed %x != %x.\n",
              stored_code_checksum, working_checksum);
        Exit(-1);
    }
}

void Recurse(int x) {
    volatile int stuff[512];

    if(x == 0)
        return;

#ifdef PARANOIA
    /* abuse the first int of the array. */
    for(stuff[1] = 1; stuff[1] < 510; stuff[1] += 1)
        stuff[stuff[1]] = stuff[1];
    stuff[1] = 1;               /* unneeded, but returns to the expected goal. */
#endif
    stuff[0] = stuff[511] = x;

    if(!Quiet || x % 100 == 0)
        Print("call %d, %x\n", x, (unsigned int)stuff);
    Check_Integrity_Or_Die();
    Recurse(x - 1);
    if(stuff[0] != x || stuff[511] != x) {
        Print
            ("rec failed: when returning, stack variables had changed.\n");
        Exit(-1);
    }
#ifdef PARANOIA
    /* abuse the first int of the array. */
    for(stuff[1] = 1; stuff[1] < 510; stuff[1] += 1) {
        if(stuff[stuff[1]] != stuff[1]) {
            Print
                ("rec failed: in paranoia mode, stack variables had changed.\n");
            Exit(-1);
        }
    }
#endif
    if(!Quiet || x % 100 == 0)
        Print("return %d\n", x);
}

int main(int argc, char **argv) {
    /* change recurse to 5-10 to see stack faults without page outs */
    int depth = 512;

    if(check_static()) {
        integrity_check_enabled = 1;
    } else {
        Print("Integrity check disabled, since statics are busted.\n");
        integrity_check_enabled = 0;
    }


    Check_Integrity_Or_Die();

    if(argc > 1) {
        depth = atoi(argv[1]);
        Print("Depth is %d\n", depth);
    }
    if(argc > 2) {
        Quiet = 1;
    }

    Recurse(depth);

    Print("Rec %d success\n", depth);

    return 0;
}
