#include <my_header.h>
#include <pthread.h>
#include <sys/time.h>

pthread_mutex_t lock;
pthread_cond_t cond;

void *thread_func(void *arg)
{
    sleep(1);
    //子线程上锁
    int ret = pthread_mutex_lock(&lock);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_lock");

    //访问的共享资源
    printf("I am son\n");

    pthread_cond_signal(&cond);

    //子线程解锁
    ret = pthread_mutex_unlock(&lock);
    THREAD_ERROR_CHECK(ret, "son pthread_mutex_unlock");

    pthread_exit((void *)NULL);
}

int main(int argc, char *argv[])
{
    //初始化互斥锁
    int ret = pthread_mutex_init(&lock, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_init");

    //初始化条件变量
    ret = pthread_cond_init(&cond, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_cond_init");

    //创建子线程
    pthread_t thread_id;
    ret = pthread_create(&thread_id, NULL, thread_func, NULL);//创建了子线程
    THREAD_ERROR_CHECK(ret, "pthread_create");

    ret = pthread_mutex_lock(&lock);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_lock");

    printf("begin wait\n");
    //因为没有满足条件，而睡眠
    
    /* * 知识点 1: struct timespec 结构体
     * 作用：用于 pthread 高精度限时函数的参数。
     * 成员：
     * - tv_sec: 秒 (seconds)
     * - tv_nsec: 纳秒 (nanoseconds)，1秒 = 1,000,000,000 纳秒。
     * 对比：它比 gettimeofday 使用的 struct timeval (微秒级) 精度更高。
     */
    struct timeval now;
    struct timespec tm;
    // 获取当前微秒级时间
    gettimeofday(&now, NULL);
    
    /* * 知识点 2: 绝对时间 (Absolute Time) 概念
     * 重要：pthread_cond_timedwait 使用的是“绝对时间”，即“未来的某个时间点”。
     * 计算方式：当前系统时间 + 你想等待的时长。
     * 示例：now.tv_sec + 100 表示主线程最长等待 100 秒。
     */
    tm.tv_sec = now.tv_sec + 100;
    tm.tv_nsec = 0; // 如果需要更精细，可以将 now.tv_usec * 1000 转化过来

    /* * 知识点 3: pthread_cond_timedwait (限时等待)
     * 作用：在条件变量上等待，但增加超时机制。
     * 入参：
     * 1. pthread_cond_t *cond: 条件变量。
     * 2. pthread_mutex_t *mutex: 互斥锁。
     * 3. const struct timespec *abstime: 等待截止的绝对时间点。
     * 返回值：
     * - 0: 成功在限时内收到信号并重新拿到锁。
     * - ETIMEDOUT (110): 时间到了信号还没来。
     * - 其他: 错误码。
     * 注意：即使超时返回，它依然会在返回前尝试【重新加锁】，这是为了保证临界区逻辑完整。
     */
    ret = pthread_cond_timedwait(&cond, &lock, &tm);
    
    /* * 知识点 4: 超时处理逻辑
     * 在生产环境中，通常需要判断 ret == ETIMEDOUT 来决定是继续等还是报错。
     */
    THREAD_ERROR_CHECK(ret, "main pthread_cond_wait");

    printf("I am main condition ok\n");

    //解锁
    ret = pthread_mutex_unlock(&lock);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_unlock");

    //主线程等子线程
    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");

    //最后一步，需要回收锁的资源
    ret = pthread_mutex_destroy(&lock);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_destroy");

    //条件变量的销毁
    ret = pthread_cond_destroy(&cond);
    THREAD_ERROR_CHECK(ret, "pthread_cond_destroy");

    return 0;
}


