#include <my_header.h>

/**
 * 知识点 1：子线程入口函数的工作模式
 * - 一旦 pthread_create 调用成功，该函数将异步执行。
 * - 这里的 while(1) 使子线程进入死循环，它会一直占用 CPU 时间片（除非被挂起）。
 */
void *thread_func(void *arg){
    printf("I am son\n");
    // 知识点 2：线程的独立执行流
    // 子线程拥有自己的栈空间，这个死循环只影响子线程自己的执行流。
    while(1);
    return NULL;
}

int main(int argc, char *argv[]){                                  
    
    // 知识点 3：pthread_t 的本质
    // 它是线程库维护的一个 ID，用于在进程内部唯一标识一个线程。
    pthread_t thread_id;
    /**
     * 知识点 4：pthread_create 的瞬间行为
     * 这是一个“非阻塞”调用。主线程调用它后会立即返回，继续执行下面的 printf。
     * 此时，系统中同时存在两个执行流：主线程和子线程。
     */
    pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程
    /**
     * 知识点 5：并发 (Concurrency) 与竞争
     * 屏幕上先打印 "I am main" 还是 "I am son" 是不确定的！
     * 这取决于 Linux 内核调度器（Scheduler）先给哪个线程分配 CPU 时间片。
     */
    printf("I am main\n");
    /**
     * 知识点 6：进程的“寿终正寝”
     * - main 函数中的 while(1) 极其重要。
     * - 如果主线程结束（执行到 return 0），Linux 会调用 exit() 退出整个进程。
     * - 进程一死，所有子线程（即使在 while(1)）会被立刻强制杀死。
     * - 这就是为什么初学者发现子线程没输出就结束了，通常是因为主线程跑太快退出了。
     */
    while(1); //主线程会等待子线程
    return 0;
}

// 1. 为什么两个 while(1) 不会互相卡死？
// 在你的CPU上，拥有非常多的物理核心。
// 多核并行：系统可以将主线程分配给核心 0，子线程分配给核心 1，它们物理上是同时运行的。
// 时间片轮转：即便是在单核环境下，Linux 内核也会每秒切换数千次执行对象，让你感觉两个死循环在“同时”运行。

// 2. 内存共享 vs 独立
// 这是 C 语言多线程最危险也最迷人的地方：
// 共享：如果此时你在全局定义一个 int count = 0;，主线程和子线程都可以修改它（这会引出后续的“锁”知识点）。
// 独立：每个线程有自己的局部变量。thread_func 里的变量和 main 里的变量互不干扰。

// 3. 如何在 Linux 终端观察它们？
// 当你运行这个程序时，打开另一个终端输入：
// ps -aL | grep your_program_name
// 你会在结果中看到两个具有相同进程 ID（PID）但不同线程 ID（LWP）的条目，这能直观地证明你的进程里有两个执行流在跑。

