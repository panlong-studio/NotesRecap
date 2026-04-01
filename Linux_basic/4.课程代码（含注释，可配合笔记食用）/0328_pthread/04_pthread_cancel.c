#include <my_header.h>

/**
 * 知识点 1：取消点 (Cancellation Point)
 * 子线程在 while(1) 死循环中，为什么能被取消？
 * 因为 sleep(1) 是一个“取消点”。当主线程发送 cancel 信号后，
 * 子线程运行到这些特定的系统调用函数时，会检查自己是否被取消，若是则立即退出。
 * 如果循环里没有任何取消点（比如纯粹的 while(1) i++;），线程可能无法被杀掉。
 */
void *thread_func(void *arg){
    while(1){
        sleep(1);
    }
    // 这里的 pthread_exit 永远不会被执行，因为线程在循环中就被 cancel 了
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id;
    //pthread_create的第四个参数可以将值传递给线程入口参数
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    // 给子线程一点运行时间
    sleep(1);
    printf("I am main\n");
    
    /**
     * 知识点 2：pthread_cancel 异步取消
     * 1. 作用：向目标线程发送一个取消请求。
     * 2. 非阻塞：该函数调用后立即返回，并不等待目标线程真正结束。
     * 3. 返回值：成功返回 0。注意：ret 为 0 仅代表“请求发送成功”，不代表“线程已死”。
     */
    ret = pthread_cancel(thread_id);
    printf("ret: %d\n",ret);
    THREAD_ERROR_CHECK(ret, "pthread_cancel");
    
    /**
     * 知识点 3：取消后的回收
     * 即使线程是被取消的，主线程也必须调用 pthread_join 来回收它的 PCB 等资源。
     * 特别注意：如果线程是被 cancel 的，其退出状态码（retval）会被系统设置为 -1，
     * 即宏定义 PTHREAD_CANCELED。
     */
    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");
    
    return 0;
}

