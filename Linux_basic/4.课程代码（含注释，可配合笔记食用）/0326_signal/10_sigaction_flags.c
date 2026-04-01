#include <my_header.h>

void func(int num)
{
    printf("\n[信号捕获] 捕捉到信号编号 : %d\n", num);
}

int main(int argc, char *argv[])
{
    struct sigaction act, oldact;
    // 初始化结构体，防止随机值干扰
    memset(&act, 0, sizeof(act));
    
    // 指定处理函数
    act.sa_handler = func;
    
    /*
     * 3. 设置标志位为 SA_RESTART
     *
     * 核心作用：自动重启被打断的系统调用。
     *
     * 场景描述：
     * 程序运行到 read() 时会阻塞（停在那等用户输入）。
     * 此时如果你按下 Ctrl+C：
     *
     * (A) 如果不设置 SA_RESTART：
     *     read() 函数会立即报错返回 -1，并设置错误码 errno 为 EINTR（Interrupted system call）。
     *     你会看到程序打印 "read over" 然后直接退出，而你根本没机会输入内容。
     *
     * (B) 如果设置了 SA_RESTART (当前代码)：
     *     当信号处理函数 func 执行完后，内核会自动让 read() 重新开始工作。
     *     你会发现程序打印完信号编号后，依然停在 read 那里等你输入。
     */
    act.sa_flags = SA_RESTART;
    
    // 4. 注册信号2(SIGINT / Ctrl+C)
    sigaction(2, &act, &oldact);

    char buf[100] = {0};
    printf("进程已启动 (PID: %d)。\n", getpid());
    printf("请输入一些文字 (此时按下 Ctrl+C 观察 read 是否会被强制结束): \n");
    
    // 5. 这是一个阻塞式的系统调用
    // STDIN_FILENO 是标准输入（键盘）的文件描述符
    ssize_t bytes_read = read(STDIN_FILENO, buf, sizeof(buf));
    
    if (bytes_read == -1) {
        // 如果没有 SA_RESTART，Ctrl+C 会导致代码运行到这里
        perror("read 失败");
    } else {
        printf("read 成功结束，读取到了: %s", buf);
    }

    printf("read over\n");

    return 0;
}


