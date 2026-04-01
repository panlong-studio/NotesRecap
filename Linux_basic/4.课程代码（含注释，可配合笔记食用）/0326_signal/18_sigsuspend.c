#include <my_header.h>

void func(int num){
    printf("\n[Handler] 捕获信号: %d\n", num);
}

int main(int argc, char *argv[]){                                  
    
    // 1. 注册 2 号信号处理函数
    signal(2, func);
    
    sigset_t set, oldset;
    sigemptyset(&set);
    sigaddset(&set, 2);

    // 2. 全局屏蔽 2 号信号
    // 保护“重要代码”不被 2 号信号打断
    sigprocmask(SIG_BLOCK, &set, &oldset);
    // ----------
    // 这里有一段重要的代码,不想让这段代码被2号信号打断
    // 这段代码需要对2号信号屏蔽
    printf("--- [关键代码段开始] 屏蔽 2 号信号，此时按 Ctrl+C 无效 ---\n");
    sleep(10);
    printf("--- 关键代码段结束 (sleep over) ---\n");
    // 这段代码执行结束后再解除2号信号的屏蔽
    // ----------
    /* sigprocmask(SIG_UNBLOCK, &set, &oldset); */

    /* printf("pause begin\n"); */
    /* pause(); */
    /* printf("pause over\n"); */
    
    /* 
     * 在之前的逻辑中，我们通常会：
     * 1. sigprocmask(SIG_UNBLOCK, &set, NULL); // 解除屏蔽
     * 2. pause();                             // 挂起等待
     * 
     * 缺陷：如果在执行完第 1 步解除屏蔽后，还没来得及执行第 2 步 pause 时，
     * 信号突然来了，处理函数执行完后回到主逻辑执行 pause()，
     * 此时 pause 就会永久挂起，错过刚才那个信号（这就是竞态条件）。
     */

    // 3. 准备 sigsuspend 的临时掩码
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, 3);  // 将 3 号信号加入临时屏蔽集
    printf("--- sigsuspend 开始 (暂时挂起并更换屏蔽集) ---\n");
    /* 
     * sigsuspend(&mask) 函数的作用（原子操作）：
     * a) 临时将进程的信号屏蔽字设置为 mask (此时 2 号信号被解除了屏蔽，3 号被屏蔽了)。
     * b) 立即挂起进程，等待信号。
     * c) 当信号触发并从处理函数返回后，sigsuspend 返回，并将信号屏蔽字【恢复】为调用前的值（即回到 SIGINT 被屏蔽的状态）。
     */
    //sigsuspend(&mask);
    if (sigsuspend(&mask) == -1) {
        // sigsuspend 总是返回 -1，并设置 errno 为 EINTR
        printf("--- sigsuspend 返回 (信号处理完毕) ---\n");
    }
    // 此时，由于 sigsuspend 恢复了原有的 mask，2 号信号又是被屏蔽的状态了
    printf("当前状态：2号信号已恢复为屏蔽状态\n");

    printf("while begin\n");
    while(1) {
        // 如果想在这里再次响应 2 号信号，需要手动 sigprocmask 解除屏蔽
        pause(); 
    }
    
    return 0;
}

