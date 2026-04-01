#include <my_header.h>

// 信号处理函数（回调函数）
// 当 SIGALRM 信号触发时，内核会暂停主逻辑，来执行这个函数
void func(int num){
    // num 是捕获到的信号编号，对于 SIGALRM，通常是 14
    printf("捕捉到信号 num : %d\n", num);
    /* 
     * 【核心技巧：实现循环定时】
     * alarm 一次性触发后就失效了。
     * 如果想每隔 1 秒执行一次，必须在处理函数内部再次调用 alarm(1)。
     * 这就像是闹钟响了之后，你按了一下“贪睡键”，再定一个 1 秒后的闹钟。
     */
    alarm(1);
}

int main(int argc, char *argv[]){                                  
    
    // 1. 注册信号处理函数
    // 告诉内核：如果收到 SIGALRM 信号，不要杀掉进程，而是去执行 func 函数。
    signal(SIGALRM, func);
    
    // 2. 设置初始定时器
    // 程序启动 10 秒后，发送第一个 SIGALRM 信号
    alarm(10);
    
    printf("while begin\n");
    while(1);
    printf("while over\n");

    return 0;
}

/*
程序启动：
    注册 func 负责处理 SIGALRM。
    调用 alarm(10)，内核开始倒计时。
    主程序进入 while(1) 空转。
第 10 秒时：
    内核向进程发送 SIGALRM 信号。
    主程序被打断，跳转到 func 执行。
    func 打印 num : 14，并执行 alarm(1)（设置了一个 1 秒后的新闹钟）。
    func 执行完，回到主程序 while(1) 继续空转。
第 11 秒时（即 1 秒后）：
    刚才 func 里设定的 alarm(1) 爆炸。
    再次进入 func，打印并再次设定 alarm(1)。
后续：
    此后每隔 1 秒，程序都会打印一次信息。 
 */
