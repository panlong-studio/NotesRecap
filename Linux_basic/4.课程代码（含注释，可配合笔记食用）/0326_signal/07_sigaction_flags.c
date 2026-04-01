#include <my_header.h>

// num: 信号编号
// siginfo: 指向 siginfo_t 结构体，包含发送者 PID 等丰富信息
// p: 上下文指针，通常强制类型转换为 ucontext_t*，此处不常用
void func(int num, siginfo_t *siginfo, void *p){
    printf("捕捉到信号编号: %d\n", num);
    printf("当前进程 (My PID): %d\n", getpid());
    // 从 siginfo 结构体中获取发送者的 PID
    printf("信号发送者 (Sender PID): %d\n", siginfo->si_pid);
}

int main(int argc, char *argv[]){                                  
    // 定义两个 sigaction 结构体变量
    // act: 用于设置新的处理行为
    // oldact: 用于保存旧的处理行为（如果不想保存可以传 NULL）
    struct sigaction act, oldact;
    
    // 1. 初始化结构体，先清零，防止垃圾数据导致不可预知的行为
    memset(&act, 0, sizeof(act));

    // 2. 指定处理函数
    // 注意：sigaction 结构体有两个处理函数成员：sa_handler 和 sa_sigaction
    // 由于我们要用三个参数的函数，所以给 sa_sigaction 赋值 
    act.sa_sigaction = func;

    // 3. 设置标志位
    // SA_SIGINFO 告诉内核：请使用 sa_sigaction 指向的三个参数的函数
    // 而不是 sa_handler 指向的一个参数的函数
    act.sa_flags = SA_SIGINFO;
    
    // 4. 注册信号处理行为
    // 参数 2: SIGINT (Ctrl+C 的信号编号)
    // 参数 &act: 指向我们定义的行为配置
    // 参数 &oldact: 备份旧的行为，方便以后恢复
    sigaction(2, &act, &oldact);

    printf("进程已启动 (PID: %d)，尝试按下 Ctrl+C 触发信号...\n", getpid());

    printf("begin while\n");
    // 让程序一直运行，等待信号
    while(1);
    
    return 0;
}

