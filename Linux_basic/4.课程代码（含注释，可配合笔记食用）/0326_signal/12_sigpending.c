#include <my_header.h>

// 信号处理函数：当捕获到 2 号信号 (SIGINT) 时执行
void func(int num){
    printf("\n收到信号 num: %d\n", num);
    printf("--- 执行信号处理函数，开始休眠 10 秒 ---\n");
    
    /* 
     * 在休眠期间，如果你按下 Ctrl + \ (发送 3 号信号 SIGQUIT)，
     * 因为我们在 main 中设置了 sa_mask 屏蔽了 3 号信号，
     * 所以 3 号信号会被阻塞，进入“未决状态”。
     */
    sleep(10); 

    // ---- 检测未决信号集 (sigpending 的核心用法) ----
    sigset_t set;
    sigemptyset(&set); // 初始化/清空信号集
    
    // sigpending 函数的作用：
    // 获取当前进程中由于被阻塞（Masked）而无法递送的信号集。
    // 如果某个信号产生了但被屏蔽了，它就会被放入 set 中。
    if (sigpending(&set) == -1) {
        perror("sigpending error");
        return;
    }

    // 检测 3 号信号 (SIGQUIT) 是否在当前的未决信号集中
    if(sigismember(&set, SIGQUIT)) // SIGQUIT 即 3 号信号
    {
        printf("检测结果：3号信号(SIGQUIT)目前正处于【未决信号集】中(已被屏蔽)\n");
    }
    else
    {
        printf("检测结果：3号信号不在未决信号集中\n");
    }

    printf("--- 信号处理函数结束 ---\n");
}

int main(int argc, char *argv[]){                                  
    
    struct sigaction act, oldact;

    // 初始化结构体
    memset(&act, 0, sizeof(act));

    // 1.设置信号处理函数
    act.sa_handler = func;

    // 2.设置信号屏蔽掩码(sa_mask)
    // 注意: sa_mask中的信号,只在信号处理函数(func)执行期间被屏蔽
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGQUIT); // 将 3 号信号加入屏蔽集
    act.sa_mask = mask;        // 当 2 号信号触发时，自动屏蔽 3 号信号

    // 3.注册 2 号信号 (SIGINT - 通常由 Ctrl+C 触发)
    // 当按下 Ctrl+C 时，执行 func，且执行期间 3 号信号被阻塞
    sigaction(SIGINT, &act, &oldact);

    printf("进程已启动（PID: %d）。\n", getpid());
    printf("测试步骤：\n1. 按 Ctrl+C 触发信号处理函数\n2. 在 10s 内按 Ctrl+\\ 发送 3 号信号\n");
    printf("开始循环，等待信号...\n");

    while(1) {
        pause(); // 让进程挂起，等待信号，比空 while(1) 更省 CPU
    }
    
    return 0;
}

