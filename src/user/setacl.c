// A acl test program for GeekOS user mode

#include <conio.h>
#include <fileio.h>
#include <process.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int ret;
    int perm = -1;

    if(argc != 4) {
        Print("Usage: setacl <file> <uid> [r | w | rw | owner | clear\n");
        Exit(-1);
    }

    if(!strcmp(argv[3], "r")) {
        perm = O_READ;
    } else if(!strcmp(argv[3], "rw")) {
        perm = O_READ | O_WRITE;
    } else if(!strcmp(argv[3], "w")) {
        perm = O_WRITE;
    } else if(!strcmp(argv[3], "owner")) {
        perm = O_OWNER;
    } else if(!strcmp(argv[3], "clear")) {
        perm = 0;
    } else {
        Print("Usage: setacl <file> <uid> [r | w | rw | clear\n");
        Exit(-1);
    }

    ret = SetAcl(argv[1], atoi(argv[2]), perm);
    if(ret) {
        Print("SetAcl returned %d\n", ret);
    }
    return (0);
}
