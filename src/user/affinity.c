/*
 * A test program for GeekOS user mode
 * Copyright (c) 2016 Jeffrey K. Hollingsworth <hollings@cs.umd.edu>
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
#include <string.h>
#include <sched.h>

int main(int argc, char **argv) {
    int i, ret, start;

    if(argc == 1) {
        Print("Usage %s [all1|split]\n", argv[0]);
        Exit(-1);
    } else if(!strcmp(argv[1], "all0")) {
        ret = Spawn_Program("/c/affinity.exe", "affinity.exe core0", 1);
        ret = Spawn_Program("/c/affinity.exe", "affinity.exe core0", 1);
    } else if(!strcmp(argv[1], "all1")) {
        ret = Spawn_Program("/c/affinity.exe", "affinity.exe core1", 1);
        ret = Spawn_Program("/c/affinity.exe", "affinity.exe core1", 1);
    } else if(!strcmp(argv[1], "split")) {
        ret = Spawn_Program("/c/affinity.exe", "affinity.exe core1", 1);
        ret = Spawn_Program("/c/affinity.exe", "affinity.exe core0", 1);
    } else if(!strcmp(argv[1], "core0")) {
        Set_Affinity(Get_PID(), 0);
        start = Get_Time_Of_Day();
        for(i = 0; i < 10000000; i++) ;
        Print("End of core affinity 0 %d wall time passed\n",
              Get_Time_Of_Day() - start);
    } else if(!strcmp(argv[1], "core1")) {
        Set_Affinity(Get_PID(), 1);
        start = Get_Time_Of_Day();
        for(i = 0; i < 10000000; i++) ;
        Print("End of core affinity 1 %d wall time passed\n",
              Get_Time_Of_Day() - start);
    } else if(!strcmp(argv[1], "none")) {
        start = Get_Time_Of_Day();
        for(i = 0; i < 10000000; i++) ;
        Print("End of core affinity none at %d wall time passed\n",
              Get_Time_Of_Day() - start);
    } else {
        Print("invalid option %s\n", argv[1]);
    }

    return 0;
}
