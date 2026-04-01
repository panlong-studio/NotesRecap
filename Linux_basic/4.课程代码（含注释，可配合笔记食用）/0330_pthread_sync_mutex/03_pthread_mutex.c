#include <my_header.h>

/* * 知识点 1: 互斥锁变量定义
 * 类型：pthread_mutex_t
 * 作用：用于保护“临界资源”，确保同一时刻只有一个线程能访问受保护的代码段。
 */
pthread_mutex_t lock; 
int gCnt = 0; // 临界资源：被多个线程并发访问的全局变量

void *thread_func(void *arg){
    for(int idx = 0; idx < 1000000; ++idx){
        /* * 知识点 2: pthread_mutex_lock (加锁/锁定)
         * 入参：pthread_mutex_t *mutex (锁变量的地址)
         * 返回值：成功返回 0；失败返回错误码。
         * 运行机制：
         * 1. 如果锁当前处于解开状态，调用线程将锁定它并立即返回。
         * 2. 如果锁已被其他线程占用，调用线程将“阻塞（挂起）”，直到锁被释放。
         */
        int ret = pthread_mutex_lock(&lock);
        THREAD_ERROR_CHECK(ret, "pthread_mutex_lock");
        
        // --- 临界区 (Critical Section) 开始 ---
        ++gCnt;
        // --- 临界区 结束 ---
        
        /* * 知识点 3: pthread_mutex_unlock (解锁/释放)
         * 入参：pthread_mutex_t *mutex
         * 返回值：成功返回 0。
         * 作用：释放对锁的占用，唤醒正在等待该锁的其他线程。
         */
        ret = pthread_mutex_unlock(&lock);
        THREAD_ERROR_CHECK(ret, "pthread_mutex_unlock");
    }
    printf("I am son\n");

    pthread_exit((void *)NULL);
}

int main(int argc, char *argv[]){                                  
    
    /* * 知识点 4: pthread_mutex_init (初始化锁)
     * 入参：
     * 1. pthread_mutex_t *mutex: 锁变量地址。
     * 2. const pthread_mutexattr_t *attr: 锁的属性。传入 NULL 表示使用默认属性。
     * 返回值：成功返回 0。
     * 注意：使用互斥锁前必须先初始化，否则行为未定义。
     */
    int ret = pthread_mutex_init(&lock, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_init");

    pthread_t thread_id;
    ret = pthread_create(&thread_id, NULL, thread_func, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    // 共享资源gCnt进行改写操作
    for(int idx = 0; idx < 1000000; ++idx){
        ret = pthread_mutex_lock(&lock);
        THREAD_ERROR_CHECK(ret, "pthread_mutex_lock");

        /* * 知识点 5: 为什么需要锁？（原子性问题）
         * ++gCnt 在汇编层面分为三步：Load(读)、Modify(改)、Store(写)。
         * 如果没有锁，两个线程可能同时读取了旧值(如 10)，各自加1得到11，
         * 然后先后写回 11，导致本该变为 12 的结果变成了 11。这叫“丢失更新”。
         */
        ++gCnt;

        ret = pthread_mutex_unlock(&lock);
        THREAD_ERROR_CHECK(ret, "pthread_mutex_unlock");
    }

    printf("I am main\n");

    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("gCnt: %d\n", gCnt); // 结果必然是 2000000，体现了锁的保护作用
    
    /* * 知识点 6: pthread_mutex_destroy (销毁锁)
     * 入参：pthread_mutex_t *mutex
     * 作用：释放互斥锁占用的系统资源。
     * 约束：
     * 1. 只有处于解锁状态的锁才能被销毁。
     * 2. 销毁后不能再对该锁进行 lock/unlock 操作。
     */
    ret = pthread_mutex_destroy(&lock);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_destroy");
    
    return 0;
}

