#include <my_header.h>

void func(int num)
{
    // 当收到信号时，会打断当前的 sleep 或正常执行流程，进入此函数
    printf("num: %d\n", num);
}

int main(int argc, char *argv[])
{                                  
    // 创建子进程
    // pid > 0 代表父进程（返回值为子进程的 PID）
    // pid == 0 代表子进程
    pid_t pid = fork();

    if(pid == 0)
    {
        // 子进程
        // 注册信号处理函数
        // 告诉内核：如果收到 2 号信号 (SIGINT)，请执行 func 函数，而不是默认的终止进程
        signal(2, func);
        printf("[子进程] 我的 PID 是 %d，我正在循环中等待信号...\n", getpid());
        while(1)
        {
            sleep(1);
            printf("[子进程] 正在运行 while(1)...\n");
        }
    }
    else
    {
        // 父进程
        printf("[父进程] 我的 PID 是 %d，我创建的子进程 PID 是 %d\n", getpid(), pid);
        printf("[父进程] 我将休眠 10 秒，然后给子进程发送 2 号信号...\n");

        // 父进程先休眠 10 秒，确保子进程已经运行并注册了信号处理函数
        sleep(10);
        /*
         * kill 函数
         * 功能：给指定的进程或进程组发送信号。
         * 参数 1 (pid)：目标进程的 PID。
         * 参数 2 (sig)：要发送的信号编号（如 2 代表 SIGINT）。
         */
        // kill(pid, 2);
        if (kill(pid, 2) == 0) {
            printf("[父进程] 信号发送成功。\n");
        } else {
            perror("kill error");
        }

        // 父进程发送完信号后退出
        printf("[父进程] 任务完成，退出。\n");
    }
    
    return 0;
}

// 前 10 秒：
// 父进程静静等待。
// 子进程每隔一秒打印一次 [子进程] 正在运行 while(1)...。
// 第 10 秒时：
// 父进程调用 kill(pid, 2)。
// 子进程的 sleep(1) 被信号打断。
// 子进程立即跳转执行 func，打印出 num : 2。
// 信号处理完后：
// 由于子进程的信号处理函数 func 结束后没有调用 exit()。
// 子进程会回到 while(1) 继续循环，继续打印 正在运行...。
// 父进程结束，子进程变成“孤儿进程”，被系统（init 或 systemd）领养。

