#include"lib/stdio.h"
#include"lib/stdlib.h"


int main()
{
    char* p = (char*)malloc(12);
    p[0] = 'a';
    p[1] = 'b';
    p[2] = 0;
    printf("%d\n", p);
    for (;;);
}