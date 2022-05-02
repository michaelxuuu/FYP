#include"lib/stdio.h"
#include"lib/unistd.h"

int main(int argc, char** argv)
{
    printf("%s!\n", argv[0]);
    exit();
}