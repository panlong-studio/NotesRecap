#include <my_header.h>

// 定义一个符合 sa_handler 原型的函数，只有一个参数
void func(int num){
    printf("num: %d\n", num);
}

int main(int argc, char *argv[]){                                  
    
    struct sigaction act, oldact;
    // 清空act
    memset(&act, 0, sizeof(act));

    // 使用sa_handler 
    act.sa_handler = func;
    
    // 设置标志位
    // SA_RESETHAND: 信号处理函数执行一次后，自动恢复为默认行为 (SIG_DFL)
    // 此时没有设置 SA_SIGINFO，所以内核会按照 1 个参数的方式调用 func
    // 信号处理函数只会生效一次
    act.sa_flags = SA_RESETHAND;
    
    // 注册信号 2 (SIGINT)
    sigaction(2, &act, &oldact);
    
    printf("程序启动，等待信号 (PID: %d)...\n", getpid());
    printf("提示：第一次 Ctrl+C 会触发函数，第二次会直接退出程序。\n");
    printf("begin while\n");
    while(1);
    
    return 0;
}

