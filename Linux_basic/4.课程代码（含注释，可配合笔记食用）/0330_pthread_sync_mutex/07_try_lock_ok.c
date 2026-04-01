#include <my_header.h>

pthread_mutex_t lock1;
pthread_mutex_t lock2;

void *thread_func(void *arg)
{
    /* * 知识点 3: 轮询与回退策略 (Polling and Backoff)
     * 重要背景：为了破坏死锁的“请求与保持”条件，当发现无法获取后续资源时，
     * 必须主动释放已经持有的资源，稍后再重试。
     */
    while(1)
    {
        //子线程先上2锁
        int ret = pthread_mutex_lock(&lock2);
        THREAD_ERROR_CHECK(ret, "son pthread_mutex_lock2");

        printf("----\n");
        sleep(1);
        printf("--- over \n");
        //子线程再上1锁
        ret = pthread_mutex_trylock(&lock1);
        printf("ret : %d\n", ret);
        THREAD_ERROR_CHECK(ret, "son pthread_mutex_trylock1");
        
        /* * 知识点 4: 冲突处理
         * 如果 ret == EBUSY (代表主线程占着 lock1)，子线程不能死等。
         */
        if(ret != 0)
        {
            /* * 知识点 5: 释放已持有的锁 (关键步骤)
             * 既然拿不到 lock1，就先把已经占用的 lock2 让出来，
             * 这样主线程在请求 lock2 时就能成功，从而打破僵局。
             */
            pthread_mutex_unlock(&lock2);
            // 让子线程稍作休息，避免产生“活锁”（两个线程不停地拿锁释放、拿锁释放）
            sleep(1);
            continue;//重新开始 while 循环，再次尝试
        }
        
        //同时拿到两把锁后，安全访问资源
        //访问的共享资源
        printf("I am son\n");

        //子线程解锁1锁
        ret = pthread_mutex_unlock(&lock1);
        THREAD_ERROR_CHECK(ret, "son pthread_mutex_unlock1");

        //子线程解锁2锁
        ret = pthread_mutex_unlock(&lock2);
        THREAD_ERROR_CHECK(ret, "son pthread_mutex_unlock2");
        break; //成功完成任务，退出循环
    }

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
    
    //主线程再上2锁
    //此时子线程因为 trylock 失败释放了 lock2，主线程得以成功获取 lock2
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
    ret = pthread_mutex_destroy(&lock1);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_destroy1");

    ret = pthread_mutex_destroy(&lock2);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_destroy2");
    return 0;
}


