#include <my_header.h>

void func(int num){
    printf("\n收到信号 num: %d\n", num);
}

int main(int argc, char *argv[]){                                  
    
    // 1. 注册 2 号信号 (SIGINT, 按 Ctrl+C 触发) 的处理动作
    // signal 是简化版的信号注册函数
    signal(2, func);
    
    // 2. 准备信号集
    sigset_t set, oldset;
    sigemptyset(&set);      // 清空信号集
    sigaddset(&set, 2);     // 将 2 号信号添加到信号集中
    
    // 全局屏蔽2号信号(sigprocmask函数本身不是阻塞函数)
    /* 
     * 3. 设置信号屏蔽 (sigprocmask)
     * SIG_BLOCK: 将 set 中的信号添加到进程当前的屏蔽掩码中。
     * 运行此行后，即便按下 Ctrl+C，信号也会被阻塞在“未决”状态，不会触发 func。
     * oldset: 用于保存修改之前的屏蔽状态（方便以后恢复）。
     */
    printf("--- 信号屏蔽已开启，现在按 Ctrl+C 将不会立即响应 ---\n");
    sigprocmask(SIG_BLOCK, &set, &oldset);
    
    // 模拟一段受保护的代码
    printf("程序开始休眠 10 秒...\n");
    sleep(10);
    printf("休眠结束 (sleep over).\n");
    printf("sleep over.\n");
    
    // 解除对2号信号的屏蔽
    /*
     * 4. 解除信号屏蔽
     * SIG_UNBLOCK: 从进程当前的屏蔽掩码中移除 set 中的信号。
     * 如果在休眠的 10 秒内你按下了 Ctrl+C，
     * 信号会被解除屏蔽并立即“递送”，导致 func 被调用。
     */
    printf("--- 信号屏蔽已解除 ---\n");
    sigprocmask(SIG_UNBLOCK, &set, &oldset);
    
    printf("主程序进入死循环，现在按 Ctrl+C 将直接触发信号处理函数...\n");
    printf("while begin\n");
    while(1){
        pause(); // 挂起等待信号
    }
    
    return 0;
}

