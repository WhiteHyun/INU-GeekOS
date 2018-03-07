#include <conio.h>
#include <process.h>
#include <sched.h>
#include <sema.h>
#include <string.h>
#include <fileio.h>

int main(int argc, char *argv[]) {
    char command_line[256];
    int recursion_depth;
    int r;

    assert(argc > 0);
    if(argc == 1) {
        Print("usage: execr2 <recursion depth>\n");
        Exit(-2);
    }
    assert(argv[1][0] >= '0' && argv[1][0] <= '9');
    recursion_depth = atoi(argv[1]);
    if(recursion_depth == 0) {
        Print("done!\n");
        Exit(0);
    } else {
        Print(".%d...\n", recursion_depth);
        snprintf(command_line, 256, "execr3 %d", recursion_depth - 1);
        r = Execl("/c/execr3.exe", command_line);
        Print("returned from Execl.  not good.\n");
        assert(0);
    }
    return 0;                   /* compiler appeasement */
}
