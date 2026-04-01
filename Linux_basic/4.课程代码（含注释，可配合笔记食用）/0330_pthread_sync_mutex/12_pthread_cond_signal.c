#include <my_header.h>
#include <pthread.h>

pthread_mutex_t lock;
pthread_cond_t cond;

void *thread_func(void *arg)
{
    // 确保主线程先运行并进入 wait 状态
    sleep(1);
    
    // 子线程上锁
    int ret = pthread_mutex_lock(&lock);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_lock");
    

    printf("I am son\n");

    /* * 知识点 1: pthread_cond_signal (唤醒线程)
     * 作用：至少唤醒一个在该条件变量 (cond) 上等待的线程。
     * 入参：pthread_cond_t *cond (目标条件变量地址)。
     * 返回值：成功返回 0。
     * 特点：
     * 1. 如果此时没有线程在 wait，信号会被丢弃（不记录，不像信号量）。
     * 2. 调用此函数并不需要持有互斥锁，但为了防止错过信号，通常在锁内发送。
     */
    ret = pthread_cond_signal(&cond);
    THREAD_ERROR_CHECK(ret, "pthread_cond_signal");

    /* * 知识点 2: 唤醒与“重新拿锁”的竞争 (核心重点)
     * 背景：子线程发送了信号，主线程会被从“等待队列”移到“就绪队列”。
     * 关键：主线程要想从 pthread_cond_wait 函数中返回，必须重新获得互斥锁。
     * 实验现象：
     * 这里的 sleep(100) 意味着子线程虽然发了信号，但依然死死攥着互斥锁不撒手。
     * 结果：主线程虽然被“唤醒”了，但会被卡在 pthread_cond_wait 内部试图 lock 的阶段，
     * 只有等 100 秒后子线程 unlock 了，主线程才能真正返回。
     */
    printf("son signaled, but sleeping with lock...\n");
    sleep(100);

    // 让子线程解锁
    ret = pthread_mutex_unlock(&lock);
    THREAD_ERROR_CHECK(ret, "son pthread_mutex_unlock1");

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
    pthread_create(&thread_id, NULL, thread_func, NULL);

    ret = pthread_mutex_lock(&lock);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_lock1");
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    //因为没有满足条件，主线程进入睡眠
    printf("begin wait\n");
    
    /* * 知识点 3: pthread_cond_wait 的返回行为
     * 刚才提到 wait 内部包含三个步骤：1.解锁 2.睡眠 3.重新加锁。
     * 只有当第 3 步“重新加锁”成功后，此函数才会返回。
     * 所以：即使子线程发了信号，主线程的这行代码也会一直阻塞到子线程释放锁。
     */
    ret = pthread_cond_wait(&cond, &lock);
    THREAD_ERROR_CHECK(ret, "main pthread_cond_wait");
    
    //共享资源
    printf("I am main condition ok\n");

    //解锁
    ret = pthread_mutex_unlock(&lock);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_unlock1");

    //主线程等子线程
    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");

    //最后一步，需要回收锁的资源
    ret = pthread_mutex_destroy(&lock);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_destroy1");

    //条件变量的销毁
    ret = pthread_cond_destroy(&cond);
    THREAD_ERROR_CHECK(ret, "pthread_cond_destroy");

    return 0;
}
