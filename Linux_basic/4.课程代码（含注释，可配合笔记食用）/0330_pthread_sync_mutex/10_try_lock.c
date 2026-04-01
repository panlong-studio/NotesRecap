#include <my_header.h>

pthread_mutex_t lock1;

void *thread_func(void *arg)
{
    int ret = 0;
    /* * 知识点 1: 忙等待 (Busy Waiting / Polling)
     * 逻辑：子线程进入一个无限循环，不断地调用 pthread_mutex_trylock。
     * 行为特点：
     * 1. 非阻塞：trylock 发现锁被占用会立即返回 EBUSY (16)，而不会让线程进入睡眠。
     * 2. 高 CPU 占用：与 sleep 不同，这个循环会疯狂占用 CPU 资源进行“询问”。
     * 3. 实时性：一旦主线程解锁，子线程几乎可以在微秒级时间内感知并夺取锁。
     */
    while(1)
    {
        //子线程再上1锁
        ret = pthread_mutex_trylock(&lock1);
        /* * 知识点 2: 轮询判断
         * ret == 0 表示成功抢到钥匙，此时跳出循环进入临界区。
         * 如果 ret != 0 (通常是 EBUSY)，循环继续，瞬间发起下一次请求。
         */
        if(0 == ret)
        {
            break;
        }

    }
    //访问的共享资源
    printf("I am son\n");
    //子线程释放1锁
    ret = pthread_mutex_unlock(&lock1);
    THREAD_ERROR_CHECK(ret, "son pthread_mutex_unlock1");

    pthread_exit((void *)NULL);
}

int main(int argc, char *argv[])
{
    //初始化互斥锁
    int ret = pthread_mutex_init(&lock1, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_init");

    pthread_t thread_id;
    ret = pthread_create(&thread_id, NULL, thread_func, NULL);//创建了子线程
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    //主线程首先抢占锁1
    ret = pthread_mutex_lock(&lock1);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_lock1");
    
    /* * 知识点 3: 模拟长耗时操作
     * sleep(100) 会让主线程睡眠 100 秒。
     * 重要背景：在这 100 秒内，子线程会执行 while(1) 里的 trylock 亿万次！
     * 可以在终端使用 'top' 或 'htop' 命令观察，会发现一个核心的 CPU 占用率接近 100%。
     */
    sleep(100);

    //模拟的共享资源
    printf("I am main\n");
    
    // 主线程释放锁，子线程的 while 循环终于可以结束
    ret = pthread_mutex_unlock(&lock1);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_unlock1");

    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");

    //最后一步，需要回收锁的资源
    ret = pthread_mutex_destroy(&lock1);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_destroy1");

    return 0;
}


