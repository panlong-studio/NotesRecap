#include <my_header.h>

//线程的入口函数，必须返回 void* 且接收一个 void* 参数
void *thread_func(void *arg)
{   
    //pthread_self()获取当前线程的 ID
    printf("son thread id : %ld\n", pthread_self());
    //pthread_exit用于主动退出线程，NULL表示不返回任何值给回收者
    pthread_exit((void *)NULL);
}

int main(int argc, char *argv[])
{
    /**
     * 知识点 1：线程属性变量
     * pthread_attr_t 是一个结构体，用于存放线程的各项属性（如分离状态、栈大小、优先级等）。
     */
    pthread_attr_t attr;
    /**
     * 知识点 2：初始化属性
     * 必须在使用前调用 pthread_attr_init，它会将属性对象设置为系统默认值。
     */
    pthread_attr_init(&attr);
    /**
     * 知识点 3：设置分离属性 (PTHREAD_CREATE_DETACHED)
     * 默认情况下，线程是“可结合的”(PTHREAD_CREATE_JOINABLE)。
     * 设置为 DETACHED 后，线程运行结束后会自动由系统回收资源，不需要（也不允许）主线程调用 pthread_join。
     */
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    
    pthread_t thread_id;

    /**
     * 知识点 4：创建带属性的线程
     * 第二个参数传入 &attr。此时子线程一出生就是“分离状态”。
     */
    //此时创建子线程的时候，设置了attr属性，也就是分离属性
    int ret = pthread_create(&thread_id, &attr, thread_func, NULL);
    printf("ret : %d\n", ret);
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    // 知识点 5：属性对象的销毁
    // 一旦 pthread_create 调用完成，属性对象就不再需要了，应当销毁以释放资源。
    pthread_attr_destroy(&attr);

    /* sleep(1); */
    printf("main thread id : %ld\n", pthread_self());
    
    /**
     * 知识点 6：对分离线程调用 join 的结果
     * 由于我们在上面设置了 PTHREAD_CREATE_DETACHED，
     * 此处的 pthread_join 会立即失败并返回错误码（通常是 EINVAL，即 22）。
     * 因为分离线程是不允许被等待（join）的。
     */
    ret = pthread_join(thread_id, NULL);
    printf("ret : %d\n", ret); // 这里通常会打印出非 0 的错误码
    THREAD_ERROR_CHECK(ret, "pthread_join");

    return 0;
}

/**
 * 线程属性的生命周期:
 * 1.定义并初始化：pthread_attr_init(&attr);
 * 2.修改属性：如 pthread_attr_setdetachstate(...);
 * 3.创建线程：pthread_create(..., &attr, ...);
 * 4.销毁属性：pthread_attr_destroy(&attr);（注意：销毁属性对象不会影响已经创建好的线程）
 */

/**
 * 可结合状态(Joinable-默认)线程的特性
 * 1.资源回收: 必须由其他线程调用 pthread_join 才能回收
 * 2.数据获取: 可以通过 join 获取线程的返回值
 * 3.生存期: 如果不 join，会产生类似“僵尸进程”的资源残留
 * 4.调用 Join: 阻塞等待直到子线程结束
 *
 * 分离状态(Detached)线程的特性:
 * 1.资源回收: 线程结束时，系统自动回收所有资源
 * 2.数据获取: 无法获取返回值
 * 3.生存期: 适合“发后即忘”的任务
 * 4.调用 Join: 调用 join 会直接报错
 */
