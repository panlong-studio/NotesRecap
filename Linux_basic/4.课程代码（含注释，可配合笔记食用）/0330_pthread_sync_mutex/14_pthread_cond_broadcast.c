#include <my_header.h>
#include <pthread.h>

pthread_mutex_t lock;
pthread_cond_t cond;

void *thread_funcA(void *arg)
{
    sleep(1);
    //子线程A上锁
    int ret = pthread_mutex_lock(&lock);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_lockA");

    printf("sonA wait begin\n");
    // 等待广播信号
    ret = pthread_cond_wait(&cond, &lock);
    THREAD_ERROR_CHECK(ret, "pthread_cond_waitA");
    
    //访问的共享资源
    printf("sonA wait end\n");

    //访问的共享资源
    printf("I am sonA\n");

    //子线程A解锁
    ret = pthread_mutex_unlock(&lock);
    THREAD_ERROR_CHECK(ret, "son pthread_mutex_unlockA");

    pthread_exit((void *)NULL);
}

void *thread_funcB(void *arg)
{
    sleep(1);
    //子线程B上锁
    int ret = pthread_mutex_lock(&lock);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_lockB");

    printf("sonB wait begin\n");
    // 等待广播信号
    ret = pthread_cond_wait(&cond, &lock);
    THREAD_ERROR_CHECK(ret, "pthread_cond_waitB");
    printf("sonB wait end\n");

    //访问的共享资源
    printf("I am sonB\n");

    //子线程B解锁
    ret = pthread_mutex_unlock(&lock);
    THREAD_ERROR_CHECK(ret, "son pthread_mutex_unlockB");

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
    pthread_t thread_idA, thread_idB;
    ret = pthread_create(&thread_idA, NULL, thread_funcA, NULL);//创建了子线程A
    THREAD_ERROR_CHECK(ret, "pthread_createA");

    ret = pthread_create(&thread_idB, NULL, thread_funcB, NULL);//创建了子线程B
    THREAD_ERROR_CHECK(ret, "pthread_createB");

    //共享资源
    printf("I am main condition ok\n");
    sleep(5); // 确保 A 和 B 都已经进入了 wait 队列

    /* * 知识点 1: pthread_cond_broadcast (广播唤醒)
     * 作用：唤醒所有等待在特定条件变量 (cond) 上的线程。
     * 入参：pthread_cond_t *cond (目标条件变量地址)。
     * 返回值：成功返回 0。
     * * * 知识点 2: 广播后的竞争机制 (重要)
     * 当执行 broadcast 后，sonA 和 sonB 会【同时】被唤醒。
     * 此时由于它们都需要重新加锁才能从 wait 函数返回，但互斥锁只有一把：
     * 1. 它们会从“条件等待队列”转移到“锁的竞争队列”。
     * 2. 只有一个子线程能先抢到锁并执行打印，执行完 unlock 后，另一个子线程才能抢到锁并继续。
     * * 结论：它们会【先后】完成打印，而不会像至使用一个 signal 那样导致其中一个永久卡死。
     */

    //将所有等待在条件变量上的线程都唤醒
    ret = pthread_cond_broadcast(&cond);
    THREAD_ERROR_CHECK(ret, "pthread_cond_broadcast");

    //主线程等子线程A
    ret = pthread_join(thread_idA, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_joinA");

    //主线程等子线程B
    ret = pthread_join(thread_idB, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_joinB");

    //最后一步，需要回收锁的资源
    ret = pthread_mutex_destroy(&lock);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_destroy1");

    //条件变量的销毁
    ret = pthread_cond_destroy(&cond);
    THREAD_ERROR_CHECK(ret, "pthread_cond_destroy");

    return 0;
}


