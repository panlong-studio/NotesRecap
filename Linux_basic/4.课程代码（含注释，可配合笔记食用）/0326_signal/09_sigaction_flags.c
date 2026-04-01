#include <my_header.h>

// 在函数内部放了一个10秒的 sleep，用来观察信号嵌套的效果
void func(int num){
    printf("\n[进入函数] 正在处理信号 : %d\n", num);
    printf("--- sleep 开始 (10秒) ---\n");
    
    // sleep() 是一个会被信号中断的函数
    // 如果在 sleep 期间再次收到信号且设置了 SA_NODEFER，
    // 内核会立即暂停当前的 func，去开启一个新的 func (嵌套)
    sleep(10); 
    
    printf("--- sleep 结束 ---\n");
    printf("[退出函数] 信号 %d 处理完毕\n", num);
}

int main(int argc, char *argv[]){                                  
    
    struct sigaction act, oldact;
    // 1. 初始化结构体，清空内存块
    memset(&act, 0, sizeof(act));
    
    // 2. 指定处理函数(使用单参数版本)
    act.sa_handler = func;
    
    /* 
     * 3. 设置标志位为 SA_NODEFER
     * 
     * SA_NODEFER 的含义：
     * - "Defer" 是延迟/推迟的意思，"No Defer" 就是不推迟。
     * - 默认情况下：当 func 正在运行时，如果用户再次按下 Ctrl+C，
     *   内核会把这个信号“推迟”，等 func 运行完再处理。
     * - 设置 SA_NODEFER 后：当 func 正在运行时，如果用户再次按下 Ctrl+C，
     *   内核不再推迟，而是立即再次调用 func。
     * - 结果：会产生“函数嵌套调用”现象。
     */
    act.sa_flags = SA_NODEFER;
    
    // 4. 为信号2(SIGINT/Ctrl+C)注册配置
    sigaction(2, &act, &oldact);
    
    printf("进程已启动 (PID: %d)，请连续按下多次 Ctrl+C 观察现象...\n", getpid());

    printf("beign while\n");
    while(1){
        pause(); // 让进程挂起等待信号，比空 while(1)更节省CPU资源
    }
    
    return 0;
}

