#include <my_header.h>
#include <pthread.h>

pthread_mutex_t lock;
pthread_cond_t cond;

/* * 知识点 1: 多线程等待同一条件变量
 * 此时 sonA 和 sonB 都会进入 pthread_cond_wait，并排队挂在 cond 的等待队列中。
 */
void *thread_funcA(void *arg)
{
    sleep(1);
    //子线程B上锁
    int ret = pthread_mutex_lock(&lock);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_lockA");

    printf("sonA wait begin\n");
    // 等待信号，同时释放锁
    ret = pthread_cond_wait(&cond, &lock);
    THREAD_ERROR_CHECK(ret, "pthread_cond_waitA");
    
    //访问的共享资源
    printf("sonA wait end\n");

    //访问的共享资源
    printf("I am sonA\n");

    //让子线程A解锁
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
    ret = pthread_cond_wait(&cond, &lock);
    THREAD_ERROR_CHECK(ret, "pthread_cond_waitB");
    printf("sonB wait end\n");

    //访问的共享资源
    printf("I am sonB\n");

    //让子线程B解锁
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

    /* //主线程拿到锁 */
    /* ret = pthread_mutex_lock(&lock); */
    /* THREAD_ERROR_CHECK(ret, "main pthread_mutex_lock1"); */

    //共享资源
    printf("I am main condition ok\n");
    /* * 知识点 2: 信号的“遗失”风险
     * 如果主线程在子线程还没进入 wait 状态之前就发出了 signal，这个信号会永久丢失。
     * 所以这里 sleep(5) 是为了确保两个子线程都已经安全地挂在了等待队列上。
     */
    sleep(5);
    
    /* * 知识点 3: pthread_cond_signal 的局限性
     * 作用：唤醒等待队列中的【至少一个】线程（通常是队首的那一个）。
     * 运行结果预测：
     * 执行这行代码后，sonA 和 sonB 中只有一个会醒来并打印 "wait end"。
     * 另一个线程将继续阻塞在 wait 处，导致 main 函数在 join 时永久卡死。
     */
    ret = pthread_cond_signal(&cond);
    THREAD_ERROR_CHECK(ret, "pthread_cond_signal");

    /* //解锁 */
    /* ret = pthread_mutex_unlock(&lock); */
    /* THREAD_ERROR_CHECK(ret, "main pthread_mutex_unlock1"); */
    
    /* * 知识点 4: 解决方案预览 —— pthread_cond_broadcast
     * 如果想让 sonA 和 sonB 同时醒来，应该使用：
     * pthread_cond_broadcast(&cond);
     * 它会唤醒所有在该条件变量上等待的线程。
     */

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


