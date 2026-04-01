#include <my_header.h>

void func(int num){
    printf("num: %d\n", num);
}

void func2(int num){
    printf("num2: %d\n", num);
}

int main(int argc, char *argv[]){                                  
    
    /* signal(2, SIG_IGN); */
    // 忽略2号信号
    /* signal(SIGINT, SIG_IGN); */
    
    // 对2号信号执行默认行为
    /* signal(SIGINT, SIG_DFL); */

    signal(2, func);
    sleep(10);

    printf("=======\n");
    /* signal(2, SIG_DFL); */
    signal(2, func2);

    while(1);

    return 0;
}

