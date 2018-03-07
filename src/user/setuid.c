// A acl test program for GeekOS user mode

#include <conio.h>
#include <fileio.h>
#include <process.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int ret;
    int perm;

    if(argc != 3) {
        Print("Usage: setuid <file> set | clear\n");
        Exit(-1);
    }

    if(!strcmp(argv[2], "set")) {
        perm = 1;
    } else if(!strcmp(argv[2], "clear")) {
        perm = 0;
    } else {
        Print("Usage: setuid <file> set | clear\n");
        Exit(-1);
    }

    ret = SetSetUid(argv[1], perm);
    if(ret) {
        Print("SetSetuid returned %d\n", ret);
    }
    return (0);
}
