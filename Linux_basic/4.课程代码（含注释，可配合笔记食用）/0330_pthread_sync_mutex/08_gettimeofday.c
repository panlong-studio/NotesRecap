#include <my_header.h>
#include <sys/time.h>

//定义全局的锁变量
pthread_mutex_t lock;
int gCnt = 0;

void *thread_func(void *arg)
{
    for(int idx = 0; idx < 1000000; ++idx)
    {
        int ret = pthread_mutex_lock(&lock);
        THREAD_ERROR_CHECK(ret, "pthread_mutex_lock");
        ++gCnt;
        ret = pthread_mutex_unlock(&lock);
        THREAD_ERROR_CHECK(ret, "pthread_mutex_unlock");
    }
    printf("I am son\n");

    pthread_exit((void *)NULL);
}

int main(int argc, char *argv[])
{
    /* * 知识点 1: struct timeval 结构体
     * 作用：保存秒和微秒级别的时间。
     * 成员：
     * - tv_sec: 秒 (seconds)
     * - tv_usec: 微秒 (microseconds)，1秒 = 1,000,000微秒
     */
    struct timeval beg, end;
    /* * 知识点 2: gettimeofday 函数
     * 作用：获取当前系统时间（相对于 1970-01-01 00:00:00 UTC 的偏移量）。
     * 入参：
     * 1. struct timeval *tv: 指向存放时间结果的结构体。
     * 2. struct timezone *tz: 通常传入 NULL（已废弃）。
     * 返回值：成功返回 0，失败返回 -1。
     * 特点：精度非常高（微秒级），常用于性能测试。
     */
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
        //上锁
        ret = pthread_mutex_lock(&lock);
        THREAD_ERROR_CHECK(ret, "pthread_mutex_lock");
        ++gCnt;//(1)将gCnt的值从内存读到寄存器中
               //(2)CPU会修改gCnt的值
               //(3)将修改之后的值传到内存中
        ret = pthread_mutex_unlock(&lock);
        THREAD_ERROR_CHECK(ret, "pthread_mutex_unlock");
    }

    printf("I am main\n");

    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("gCnt: %d\n", gCnt);


    //最后一步，需要回收锁的资源
    ret = pthread_mutex_destroy(&lock);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_destroy");
    
    //获取结束时间
    gettimeofday(&end, NULL);

    /* * 知识点 3: 时间差值计算
     * 计算公式：(秒差 * 1,000,000) + 微秒差
     * 这样可以得到总的运行时间（单位：微秒）。
     * 注意：加锁/解锁是极其频繁的系统调用或库调用，涉及内核态/用户态切换，
     * 会显著增加程序的执行时长。
     */
    printf("time : %ld\n", (end.tv_sec - beg.tv_sec) * 1000000 
                           + (end.tv_usec - beg.tv_usec));

    //y/d + 目标行号 + G 多行的拷贝/多行的删除
    return 0;
}


