#include <conio.h>

extern void ELF_Print(char* msg);

int main(int argc, char** argv)
{
    char s1[40] = "Hi! This is the first string\n";
    char s2[40] = "Hi! This is the second string\n";
    ELF_Print(s1);
    ELF_Print(s2);
    return 0;
}
