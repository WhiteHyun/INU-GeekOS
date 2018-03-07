/* A simple login program for GeekOS
 * Copyright (c) 2014 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
 *
 * All rights reserved.
 *
 * This code may not be resdistributed without the permission of the copyright holders.
 * Any student solutions using any of this code base constitute derviced work and may
 * not be redistributed in any form.  This includes (but is not limited to) posting on
 * public forums or web sites, providing copies to (past, present, or future) students
 * enrolled in similar operating systems courses the University of Maryland's CMSC412 course.
 */

#include <conio.h>
#include <process.h>
#include <fileio.h>

char login[20], password[20];

int main(int argc, char **argv) {

    while (1) {

        Print("login: ");
        Read_Line(login, 20);

        Print("password: ");
        Echo(false);
        Read_Line(password, 20);
        Echo(true);
        Print("\n");


    }

    return 0;
}
