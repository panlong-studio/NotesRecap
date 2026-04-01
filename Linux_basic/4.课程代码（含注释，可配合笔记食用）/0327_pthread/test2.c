#include <my_header.h>

//交换两个数的值
//int a = 3, int b = 4;
//swap(a, b);
//a = 4, b = 3;

/* void swap(int x, int y) */
/* { */
/*     int tmp = x; */
/*     x = y; */
/*     y = tmp; */
/* } */

void swap(int *x, int *y)
{
    int tmp = *x;
    *x = *y;
    *y = tmp;
}

int main(int argc, char *argv[])
{
    int a = 3;
    int b = 4;
    printf("执行swap之前a = %d, b = %d\n", a, b);
    swap(&a, &b);
    printf("执行swap之后a = %d, b = %d\n", a, b);
    return 0;
}


