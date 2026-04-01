#include <my_header.h>
#include <sys/time.h>

//定义全局的锁变量
pthread_mutex_t lock;
int gCnt = 0;

void *thread_func(void *arg)
{
    for(int idx = 0; idx < 1000000; ++idx)
    {
        /* * 知识点 1: 移除锁的影响 (性能视角)
         * 移除 pthread_mutex_lock 和 unlock 后，程序不再进行用户态到内核态的切换，
         * 也不再进行线程间的同步等待。你会发现 time 输出值会大幅缩减。
         */

        /* * 知识点 2: 竞态条件 (Race Condition) —— 核心重要知识点
         * 虽然代码中 ++gCnt 看起来只有一行，但在汇编层面是非原子的。
         * 多个线程同时执行这行代码时，会发生执行流的交织（Interleaving）。
         */

        /* int ret = pthread_mutex_lock(&lock); */
        /* THREAD_ERROR_CHECK(ret, "pthread_mutex_lock"); */
        ++gCnt;
        /* ret = pthread_mutex_unlock(&lock); */
        /* THREAD_ERROR_CHECK(ret, "pthread_mutex_unlock"); */
    }
    printf("I am son\n");

    pthread_exit((void *)NULL);
}

int main(int argc, char *argv[])
{
    struct timeval beg, end;
    gettimeofday(&beg, NULL);

    //初始化互斥锁
    int ret = pthread_mutex_init(&lock, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_init");

    pthread_t thread_id;
    ret = pthread_create(&thread_id, NULL, thread_func, NULL);//创建了子线程
    THREAD_ERROR_CHECK(ret, "pthread_create");

    //共享资源gCnt进行改写操作
    for(int idx = 0; idx < 1000000; ++idx)
    {
        /* * 知识点 3: 内存可见性与原子性缺失
         * 由于没有锁，主线程和子线程可能同时读取到 gCnt 的旧值。
         * 例如：gCnt 当前为 100。
         * 1. 主线程读取 100 到寄存器，准备加 1。
         * 2. 子线程恰好也读取 100 到寄存器。
         * 3. 主线程写回 101。
         * 4. 子线程也写回 101。
         * 结果：两次加法操作，gCnt 只增加了 1。这就是“数据损坏”。
         */

        //上锁
        /* ret = pthread_mutex_lock(&lock); */
        /* THREAD_ERROR_CHECK(ret, "pthread_mutex_lock"); */
        ++gCnt;//(1)将gCnt的值从内存读到寄存器中
               //(2)CPU会修改gCnt的值
               //(3)将修改之后的值传到内存中
        /* ret = pthread_mutex_unlock(&lock); */
        /* THREAD_ERROR_CHECK(ret, "pthread_mutex_unlock"); */
    }

    printf("I am main\n");

    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("gCnt: %d\n", gCnt);
    /* * 知识点 4: 结果的不可预测性
     * 在上份加锁的代码中，gCnt 必然是 2,000,000。
     * 在本份代码中，gCnt 的值通常会小于 2,000,000，且每次运行的结果都不一样。
     * 这体现了多线程程序的“不确定性（Non-deterministic）”。
     */

    //最后一步，需要回收锁的资源
    ret = pthread_mutex_destroy(&lock);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_destroy");

    gettimeofday(&end, NULL);

    /* * 知识点 5: 吞吐量 (Throughput) 与 同步开销 (Overhead)
     * 通过 time 的对比，你会发现不加锁的程序比加锁的快得多。
     * 结论：锁保证了安全，但牺牲了效率。在实际开发中需要权衡。
     */
    printf("time : %ld\n", (end.tv_sec - beg.tv_sec) * 1000000 
                           + (end.tv_usec - beg.tv_usec));

    //y/d + 目标行号 + G 多行的拷贝/多行的删除
    return 0;
}


