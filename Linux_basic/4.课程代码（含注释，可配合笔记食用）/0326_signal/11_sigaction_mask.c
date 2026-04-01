#include <my_header.h>

// 信号处理函数
void func(int num)
{
    printf("\n[进入函数] 正在处理信号 : %d\n", num);
    printf("--- sleep 开始 (10秒)，此时信号 3 会被屏蔽 ---\n");
    
    // 模拟一段耗时操作
    // 在这 10 秒内，即使你按了 Ctrl+\ (信号3)，程序也不会立刻处理
    sleep(10); 
    
    printf("--- sleep 结束 ---\n");
    printf("[退出函数] 信号 %d 处理完毕\n", num);
}

int main(int argc, char *argv[])
{
    // 在触发2号信号的过程中，如果来了3号信号
    // 3号信号会被屏蔽

    struct sigaction act, oldact;
    // 初始化结构体
    memset(&act, 0, sizeof(act));
    
    // 指定信号处理函数
    act.sa_handler = func;

    /* 
     * 3. 设置 sa_mask（信号屏蔽掩码）
     * 
     * 原理：
     * 当信号 2 (SIGINT) 被触发并进入 func 函数时，
     * 内核会自动将 act.sa_mask 中包含的信号加入到系统的“信号屏蔽字”中。
     * 
     * 效果：
     * 在 func 运行期间，如果你发送了信号 3 (SIGQUIT)，
     * 该信号会被阻塞（Pending 状态），不会打断 func 的执行。
     * 当 func 执行结束返回后，内核会自动解除屏蔽，刚才“攒着”的信号 3 会被立刻递送。
     */
    sigset_t mask;
    sigemptyset(&mask);   // 第一步：清空信号集，防止有随机值
    sigaddset(&mask, 3);  // 第二步：将 3 号信号 (SIGQUIT, 快捷键 Ctrl+\) 加入该集合
    act.sa_mask = mask;   // 第三步：将配置好的掩码赋值给结构体
    
    // 4. 注册信号 2 (SIGINT / Ctrl+C)
    // 注意：sa_mask 只在处理信号 2 的过程中生效！
    sigaction(2, &act, &oldact);
    
    printf("进程已启动 (PID: %d)。\n", getpid());
    printf("实验步骤：\n");
    printf("1. 按下 Ctrl+C (信号2)，进入函数。\n");
    printf("2. 在 10秒内，按下 Ctrl+\\ (信号3)。\n");
    printf("3. 观察现象：信号 3 不会立刻爆发，而是等 func 结束后才打印 '退出确认'。\n");
    printf("beign while.\n");
    while(1);
    return 0;
}


