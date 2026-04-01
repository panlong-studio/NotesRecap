#include <my_header.h>

/* * 知识点 1: 条件变量定义
 * 类型：pthread_cond_t
 * 作用：条件变量本身不是锁，它是一种“发信号”的机制。
 * 它允许线程在某个特定条件（如队列为空、任务未到）下进入睡眠，
 * 直到另一个线程发出“信号”唤醒它。
 */
pthread_mutex_t lock;
pthread_cond_t cond;

void *thread_func(void *arg)
{
    /* * 知识点 2: 为什么要 sleep(1)？
     * sleep 是为了确保主线程先运行并进入 pthread_cond_wait 状态。
     */
    sleep(1);
    
    int ret = pthread_mutex_lock(&lock);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_lock");

    printf("I am son\n");

    /* * 思考点：
     * 注意：由于主线程阻塞在 pthread_cond_wait 处且没有其他线程发信号（signal/broadcast），
     * 子线程执行完并退出后，主线程将陷入死锁（永久睡眠）。
     */
    ret = pthread_mutex_unlock(&lock);
    THREAD_ERROR_CHECK(ret, "son pthread_mutex_unlock1");

    pthread_exit((void *)NULL);
}

int main(int argc, char *argv[])
{
    // 初始化互斥锁
    int ret = pthread_mutex_init(&lock, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_init");

    /* * 知识点 3: pthread_cond_init (初始化条件变量)
     * 入参：
     * 1. pthread_cond_t *cond: 条件变量地址。
     * 2. const pthread_condattr_t *attr: 属性，传 NULL 表示默认。
     * 返回值：成功返回 0。
     */
    ret = pthread_cond_init(&cond, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_cond_init");

    pthread_t thread_id;
    ret = pthread_create(&thread_id, NULL, thread_func, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_create");

    // 主线程加锁
    ret = pthread_mutex_lock(&lock);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_lock1");

    printf("begin wait\n");

    /* * 知识点 4: pthread_cond_wait
     * 作用：让当前线程在条件变量 cond 上等待。
     * 入参：
     * 1. pthread_cond_t *cond: 等待的条件变量。
     * 2. pthread_mutex_t *mutex: 已经持有的互斥锁。
     * * 内部极其关键的三个原子操作（由内核保证顺序）：
     * 1. 解锁 (unlock)：释放传入的互斥锁（以便其他线程能拿到锁去修改条件）。
     * 2. 睡眠 (sleep)：线程进入等待队列，交出 CPU。
     * --- (当被 signal 唤醒后) ---
     * 3. 重新加锁 (relock)：在函数返回前，必须重新获得互斥锁。
     */
    ret = pthread_cond_wait(&cond, &lock);
    THREAD_ERROR_CHECK(ret, "main pthread_cond_wait");

    printf("I am main condition ok\n");

    ret = pthread_mutex_unlock(&lock);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_unlock1");

    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");

    // 销毁锁
    ret = pthread_mutex_destroy(&lock);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_destroy1");

    /* * 知识点 5: pthread_cond_destroy (销毁条件变量)
     * 作用：释放条件变量占用的资源。
     * 约束：只有当没有线程在等待该条件变量时，才能销毁它。
     */
    ret = pthread_cond_destroy(&cond);
    THREAD_ERROR_CHECK(ret, "pthread_cond_destroy");

    return 0;
}
