#include <my_header.h>

int main(int argc, char *argv[]){                                  
    
    // 定义局部锁变量（注意：虽然在 main 中定义，但若要多线程共享，需传递地址或定义为全局）
    pthread_mutex_t lock;

    /* * 知识点 1: pthread_mutex_init 的第二个参数
     * 入参：const pthread_mutexattr_t *attr
     * 作用：设置锁的属性（如：普通锁、检错锁、嵌套锁/递归锁）。
     * 这里的 NULL 指的是 PTHREAD_MUTEX_DEFAULT（默认属性）。
     * 在 Linux 中，默认属性通常是“非递归锁”，即不可重复加锁。
     */
    int ret = pthread_mutex_init(&lock, NULL);
    printf("ret : %d\n", ret);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_init");

    // 第一次上锁
    ret = pthread_mutex_lock(&lock);
    printf("ret : %d\n", ret);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_lock");
    
    /* * 知识点 2: 自死锁 (Self-Deadlock)
     * 现象：如果取消下面代码的注释，程序运行到这里会“卡死”。
     * 原因：默认锁（Normal Mutex）不维护“拥有者”计数。
     * 当同一个线程试图对已经由自己持有的锁再次加锁时，它会进入阻塞状态，等待锁被释放。
     * 但释放锁的动作又被阻塞在后面，形成了“我等我自己”的死循环。
     * 解决：若业务需要重复加锁，需在 init 时设置属性为 PTHREAD_MUTEX_RECURSIVE（递归锁）。
     */
    /* ret = pthread_mutex_lock(&lock); */
    /* printf("ret : %d\n", ret); */
    /* THREAD_ERROR_CHECK(ret, "pthread_mutex_lock"); */
    
    // 第一次解锁
    ret = pthread_mutex_unlock(&lock);
    printf("ret : %d\n", ret);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_unlock");

    /* * 知识点 3: 异常解锁行为
     * 场景：对一个已经解锁的锁再次调用 unlock，或者解锁一个由其他线程持有的锁。
     * 后果：在 POSIX 标准中，这通常会导致“未定义行为（Undefined Behavior）”。
     * 有些系统可能会报错（EPERM），有些则可能导致程序崩溃或锁状态混乱。
     * 原则：加锁次数与解锁次数必须严格 1:1 对应。
     */
    /* ret = pthread_mutex_unlock(&lock); */
    /* printf("ret : %d\n", ret); */
    /* THREAD_ERROR_CHECK(ret, "pthread_mutex_unlock"); */
    
    /* * 知识点 4: pthread_mutex_destroy 的前提
     * 约束：被销毁的锁必须处于“解锁状态（Unlocked）”。
     * 报错：如果销毁一个正被某个线程持有的锁，函数会返回 EBUSY 错误。
     * 返回值：0 表示成功，非 0 表示失败。
     */
    ret = pthread_mutex_destroy(&lock);
    printf("ret : %d\n", ret);
    THREAD_ERROR_CHECK(ret, "pthread_mutex_destroy");

    
    return 0;
}

