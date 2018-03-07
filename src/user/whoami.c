// A user id test program for GeekOS user mode

#include <conio.h>
#include <fileio.h>
#include <process.h>
#include <string.h>

int main(int argc, char *argv[]) {
    Print("my uid = %d\n", GetUid());
    if(argc > 1) {
        int expectedUid = atoi(argv[1]);
        if(expectedUid == GetUid()) {
            Exit(0);
        } else {
            Exit(-1);
        }
    }
    return 0;
}
