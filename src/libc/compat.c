#include <conio.h>
#include <stddef.h>
#include <malloc.h>
#include <geekos/syscall.h>
#include <geekos/projects.h>

DEF_SYSCALL(Sbrk, SYS_SBRK, void *, (int arg0),, SYSCALL_REGS_1)
void *Malloc(unsigned long n __attribute__ ((unused))) {
    Print("Malloc not implemented in user mode\n");
    return 0;
    TODO_P(PROJECT_MALLOC, "Malloc");
}

void Free(void *n) {
    TODO_P(PROJECT_MALLOC, "free!");
}
