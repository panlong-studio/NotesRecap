#include <my_header.h>

void func(int num){
    printf("\n[Handler] 捕获到信号: %d，正在执行处理逻辑...\n", num);
}

int main(int argc, char *argv[]){                                  
    
    // 1. 注册 2 号信号 (SIGINT) 的处理动作
    signal(2, func);
    // 2. 准备信号集，用于屏蔽 2 号信号
    sigset_t set, oldset;
    sigemptyset(&set);
    sigaddset(&set, 2); // 将 SIGINT (2) 加入集合
    /* 
     * 3. 开启信号屏蔽 (保护关键代码段)
     * 从这一行开始，即便用户按下 Ctrl+C，信号也会被阻塞在“未决状态”，不会打断程序。
     */
    printf("--- [关键代码段开始] 屏蔽 2 号信号，此时按 Ctrl+C 无效 ---\n");
    sigprocmask(SIG_BLOCK, &set, &oldset);
    // ----------
    // 这里有一段重要的代码,不想让这段代码被2号信号打断
    // 这段代码需要对2号信号屏蔽
    printf("正在处理重要任务，请等待 10 秒...\n");
    sleep(10);
    printf("任务处理完成 (sleep over).\n");
    // 这段代码执行结束后再解除2号信号的屏蔽
    // ----------

    /*
     * 4. 解除屏蔽
     * 重要：如果在上面的 10 秒内你按下了 Ctrl+C，
     * 那么在执行 sigprocmask(SIG_UNBLOCK...) 的瞬间，
     * 内核会立即发现有一个未决的 2 号信号，并迅速调用 func 函数。
     */
    sigprocmask(SIG_UNBLOCK, &set, &oldset);
    
    /*
     * 5. 等待信号 (pause)
     * 如果刚才在屏蔽期间按过 Ctrl+C，那么信号在上面那一对 sigprocmask 处已经处理完了。
     * 此时 pause() 会再次挂起，等待下一个（新的）信号。
     */
    printf("pause begin(正在等待新的信号唤醒)\n");
    pause();
    printf("pause over\n");
    
    printf("while begin\n");
    while(1);
    printf("while over\n");
    return 0;
}

