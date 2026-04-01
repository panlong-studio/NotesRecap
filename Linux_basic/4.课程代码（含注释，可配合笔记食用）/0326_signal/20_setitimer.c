#include <my_header.h>
#include <sys/time.h>
#include <signal.h>

void func(int num){
    // 在 setitimer 模式下，不需要像 alarm 那样在函数内部再次手动调用 alarm(1)
    // 因为 it_interval 已经帮我们实现了自动重装载（Auto-reload）
    printf("捕捉到信号 num : %d (来自定时器)\n", num);
}

int main(int argc, char *argv[]){                                  
    
    // 1. 注册信号处理函数：捕获 SIGALRM
    signal(SIGALRM, func);
    // 设置了定时器,时长为10s
    
    // 2. 定义定时器结构体变量
    struct itimerval new_value;

    /*
     * 3. 配置【周期性间隔时间】 (it_interval)
     * 只要第一次定时闹钟响起后，之后每隔多长时间再次触发。
     */
    new_value.it_interval.tv_sec = 1;   // 每隔 1 秒
    new_value.it_interval.tv_usec = 0;  // 0 微秒
    
    /*
     * 4. 配置【第一次启动时间】 (it_value)
     * 程序运行后，经过多长时间开启第一次闹钟。
     */
    new_value.it_value.tv_sec = 10;  // 10 秒后第一次触发
    new_value.it_value.tv_usec = 0;  // 0 微秒
    
    /*
     * 5. 启动定时器
     * ITIMER_REAL: 以真实时间计时，触发 SIGALRM 信号
     * &new_value:  填入我们上面配置好的结构体
     * NULL:        不需要保存旧的配置
     */
    setitimer(ITIMER_REAL, &new_value, NULL);
    
    printf("定时器已设置：10秒后首次触发，之后每秒触发一次...\n");
    printf("while begin\n");
    while(1) {
        // 可以用 pause() 代替空循环以节省 CPU
        // pause(); 
    };
    printf("while over\n");

    return 0;
}

