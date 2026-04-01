#include <my_header.h>

pthread_mutex_t lock1;
pthread_mutex_t lock2;

void *thread_func(void *arg)
{
    //子线程先上2锁
    int ret = pthread_mutex_lock(&lock2);
    THREAD_ERROR_CHECK(ret, "son pthread_mutex_lock2");

    printf("----\n");
    sleep(1);
    printf("--- over \n");
    
    /* * 知识点 1: pthread_mutex_trylock (尝试加锁)
     * 作用：尝试锁定互斥锁。与 pthread_mutex_lock 不同，它绝不会阻塞调用线程。
     * 入参：pthread_mutex_t *mutex (目标锁地址)
     * 返回值：
     * - 0: 成功获取锁。
     * - EBUSY: 锁已被其他线程占用（注意：这在 pthread 中不属于严重 Error，而是正常状态）。
     * - 其他错误码。
     */
    //子线程再上1锁
    ret = pthread_mutex_trylock(&lock1);
    printf("ret : %d\n", ret);
    /* * 知识点 2: 逻辑陷阱
     * 此时主线程持有 lock1。trylock 会立即返回 EBUSY (16)。
     * 但下方的 THREAD_ERROR_CHECK 如果识别到非 0 就会报错。
     * 这里报错但不会退出，代码继续往下走访问共享资源，会破坏互斥性（因为没拿到锁）。
     */
    THREAD_ERROR_CHECK(ret, "son pthread_mutex_trylock1");

    //访问的共享资源
    printf("I am son\n");

    //先让子线程解锁1锁
    //这里子线程尝试去解锁主线程拥有的lock1,会报错
    ret = pthread_mutex_unlock(&lock1);
    THREAD_ERROR_CHECK(ret, "son pthread_mutex_unlock1");

    //先让子线程解锁2锁
    ret = pthread_mutex_unlock(&lock2);
    THREAD_ERROR_CHECK(ret, "son pthread_mutex_unlock2");

    pthread_exit((void *)NULL);
}

int main(int argc, char *argv[])
{
    //初始化互斥锁
    int ret = pthread_mutex_init(&lock1, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_init");

    ret = pthread_mutex_init(&lock2, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_init");

    pthread_t thread_id;
    ret = pthread_create(&thread_id, NULL, thread_func, NULL);//创建了子线程
    THREAD_ERROR_CHECK(ret, "pthread_create");

    //主线程先上1锁
    ret = pthread_mutex_lock(&lock1);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_lock1");

    printf("+++++\n");
    sleep(1);
    printf("+++++ over\n");

    //主线程尝试上2锁(会被子线程阻塞)
    //主线程在这里阻塞等待，直到子线程调用了 unlock(&lock2)
    //一旦子线程放手，主线程成功获取了 lock2 的所有权
    ret = pthread_mutex_lock(&lock2);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_lock2");

    //模拟的共享资源
    printf("I am main\n");

    //主线程解2锁
    ret = pthread_mutex_unlock(&lock2);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_unlock2");
    
    //主线程解1锁
    ret = pthread_mutex_unlock(&lock1);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_unlock1");

    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");

    //最后一步，需要回收锁的资源
    //这里主线程明明已经解开了lock1的锁,为什么这里还是会报错:Device or resource busy?
    //因为子线程在报错 trylock 失败后，做了一个动作：pthread_mutex_unlock(&lock1);
    //在很多 Linux 线程库实现中，如果一个线程尝试去 unlock 一个它并不拥有的锁，会造成内部状态机的紊乱（Undefined Behavior）。
    //虽然主线程后面执行了它自己的 unlock(&lock1)，但由于之前的“非法干预”，这个锁的引用计数或 Owner 标记已经坏掉了.
    //内核认为它不再是一个“干净的、可销毁的”锁, 所以这里才会报错。
    ret = pthread_mutex_destroy(&lock1);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_destroy1");
    
    // 子线程、主线程对 lock2 的操作是完全合法且符合“所有权”规则的, 所以这里没有报错
    ret = pthread_mutex_destroy(&lock2);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_destroy2");
    return 0;
}


