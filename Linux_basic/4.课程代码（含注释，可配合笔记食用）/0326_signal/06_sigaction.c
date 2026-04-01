#include <my_header.h>

void func(int num){
    printf("num: %d\n", num);
}

int main(int argc, char *argv[]){                                  
    
    // C语言中,数组名是数组的入口地址,函数名是函数的入口地址
    /* printf("&main: %p\n", &main); */
    /* printf("main: %p\n", main); */
    
    /* int a = 10; */
    /* int *p = &a; */

    struct sigaction act, oldact;
    // 清空act
    memset(&act, 0, sizeof(act));

    // 信号处理函数
    act.sa_handler = func;

    sigaction(2, &act, &oldact);

    printf("begin while\n");
    while(1);
    
    return 0;
}

