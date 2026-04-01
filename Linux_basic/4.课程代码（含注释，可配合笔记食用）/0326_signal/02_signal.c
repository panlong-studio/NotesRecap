#include <my_header.h>

void func(int num)
{
    printf("num: %d\n", num);
}

void func2(int num)
{
    printf("func2 num: %d\n", num);
}
int main(int argc, char *argv[])
{
    signal(2, func);
    signal(3, func);

    sleep(10);
    printf("=====\n");

    while(1);
    return 0;
}


