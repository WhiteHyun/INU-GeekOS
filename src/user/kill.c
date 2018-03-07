/*
 * A test program for GeekOS user mode
 */

#include <conio.h>
#include <process.h>
#include <string.h>
#include <signal.h>

int main(int argc, char **argv) {
    int i, rc;
    for(i = 1; i < argc; ++i) {
        int victim_pid = atoi(argv[i]);
        rc = Kill(victim_pid, 1);
        if(rc < 0) {
            Print("Failed to kill, err %d\n", rc);
            Exit(rc);
        }
    }
    return 1;
}
