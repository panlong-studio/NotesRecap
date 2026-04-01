#include <my_header.h>

/* * 知识点 1: 多锁竞争环境
 * 当一个逻辑流程需要同时占有多个资源（锁）时，风险指数会大幅增加。
 */
pthread_mutex_t lock1;
pthread_mutex_t lock2;

void *thread_func(void *arg){
    // 子线程加锁顺序：lock2 -> lock1
    int ret = pthread_mutex_lock(&lock2);
    THREAD_ERROR_CHECK(ret, "son pthread_mutex_lock2");

    printf("-----\n");
    /* * 知识点 2: 保持锁并睡眠 (Hold and Wait)
     * sleep(1) 模拟了线程在持有锁的状态下处理业务逻辑。
     * 此时子线程拿着 lock2，主线程拿着 lock1，双方都在等待对方手里的资源。
     */
    sleep(1); 
    printf("----- over \n");

    /* * 知识点 3: 死锁的发生点
     * 线程执行到此处会阻塞。因为 lock1 被主线程占用，而主线程正在等待子线程释放 lock2。
     * 结果：两个线程都进入睡眠/挂起状态，永远无法被唤醒。
     */
    ret = pthread_mutex_lock(&lock1); 
    THREAD_ERROR_CHECK(ret, "son pthread_mutex_lock1");

    printf("I am son\n");

    ret = pthread_mutex_unlock(&lock1);
    THREAD_ERROR_CHECK(ret, "son pthread_mutex_unlock1");

    ret = pthread_mutex_unlock(&lock2);
    THREAD_ERROR_CHECK(ret, "son pthread_mutex_unlock2");

    pthread_exit((void *)NULL);
}

int main(int argc, char *argv[]){                                  
    
    //初始化互斥锁
    int ret = pthread_mutex_init(&lock1, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_init");

    ret = pthread_mutex_init(&lock2, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_init");

    pthread_t thread_id;
    ret = pthread_create(&thread_id, NULL, thread_func, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_create");

    // 主线程加锁顺序：lock1 -> lock2
    ret = pthread_mutex_lock(&lock1);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_lock1");

    printf("+++++\n");
    sleep(1); // 故意制造时间窗口，让子线程有足够时间锁住 lock2
    printf("+++++ over\n");
    
    /* * 知识点 4: 资源请求的环路等待
     * 主线程尝试获取 lock2，但 lock2 被子线程死死攥在手里。
     * 由于主线程不释放 lock1，子线程也无法推进。
     * 此时程序在终端上会表现为“卡死”，不再打印后续内容。
     */
    ret = pthread_mutex_lock(&lock2);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_lock2");

    printf("I am main\n");

    ret = pthread_mutex_unlock(&lock2);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_unlock2");

    ret = pthread_mutex_unlock(&lock1);
    THREAD_ERROR_CHECK(ret, "main pthread_mutex_unlock1");

    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");

    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);
    
    return 0;
}

