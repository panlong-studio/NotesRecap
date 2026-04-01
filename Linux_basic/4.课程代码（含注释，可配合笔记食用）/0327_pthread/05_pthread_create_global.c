#include <my_header.h>

/* * 知识点 1：全局变量与线程共享
 * - 全局变量存储在进程的数据段（Data Segment）。
 * - 进程内的所有线程共享同一个全局变量 `gCnt`。
 * - 这种“共享”是多线程通信的基础，也是产生 Bug 的根源。
 */
int gCnt = 0;

void *thread_func(void *arg){
    /* * 知识点 2：非原子操作(难点)
     * - `++gCnt` 在 CPU 层面通常分为三步：
     * 1. Load：将 gCnt 从内存读到寄存器。
     * 2. Add：在寄存器中加 1。
     * 3. Store：将新值写回内存。
     * - 如果线程 A 执行到第 2 步时，时间片用完被切换，线程 B 读到了旧值，
     * 最终两次加法可能只产生了一次加 1 的效果，这就是“竞态条件”。
     */
    for(int idx=0; idx<1000000; ++idx){
        ++gCnt;
    }
    printf("I am son\n");
    return NULL;
}

int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id;
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    // 主线程也在对同一个全局变量进行 1,000,000 次自增
    for(int idx=0; idx<1000000; ++idx){
        ++gCnt;
    }

    printf("son thread id: %ld\n", thread_id);
    printf("main thread id: %ld\n", pthread_self());

    printf("I am main\n");
    sleep(5);
    
    /* * 知识点 3：验证并发安全性
     * 运行结果通常会小于 2000000。
     * 这种结果的不确定性证明了：在没有保护（如互斥锁 Mutex）的情况下，
     * 多个线程同时操作共享资源是非线程安全的（Thread-Unsafe）。
     */
    printf("gCnt: %d", gCnt);
    return 0;
}

