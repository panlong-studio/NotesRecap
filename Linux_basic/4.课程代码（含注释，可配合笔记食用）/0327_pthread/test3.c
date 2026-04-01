#include <my_header.h>

int *func()
{
    static int num = 100;
    return &num;
}

int main(int argc, char *argv[])
{
    return 0;
}
