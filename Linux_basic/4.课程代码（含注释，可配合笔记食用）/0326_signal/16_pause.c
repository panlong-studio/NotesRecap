#include <my_header.h>

void func(int num){
    printf("num: %d\n", num);
}

int main(int argc, char *argv[]){                                  
    
    signal(2, func);

    printf("pause begin\n");
    /* 
     * pause() 函数
     * 进程执行到这里会“暂停”，不再消耗 CPU 资源。
     * 它会一直等待，直到有一个信号传给该进程。
     * 只有当信号处理函数 (func) 执行完并返回后，pause 才会返回。
     * pause 的返回值永远是 -1
     */
    pause();
    printf("pause over\n");
    
    return 0;
}

