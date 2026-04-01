#include <my_header.h>

/**
 * 知识点 1：手动设置取消点 pthread_testcancel()
 * 1. 背景：在 Linux 中，只有部分系统调用（如 sleep, read, write）是“天然取消点”。
 * 2. 作用：如果你的线程执行的是纯数学计算或死循环，里面没有任何天然取消点，
 * 那么即使主线程发出了 cancel 请求，子线程也会视而不见，继续存活。
 * 3. 机制：pthread_testcancel() 专门用来手动设置取消点。
 * 一旦执行到这一行，线程会检查自己是否收到了 cancel 信号，如果有，立刻原地退出。
 */
void *thread_func(void *arg){
    while(1){
        // 在死循环中手动埋设取消点
        // 保证了即使循环体内没有任何 I/O 操作，也能被主线程终止
        pthread_testcancel();
    }
    // 由于线程在循环中被取消，这一行代码永远不会被执行
    pthread_exit(NULL);
}


int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id;
    //pthread_create的第四个参数可以将值传递给线程入口参数
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    sleep(1);
    printf("I am main\n");
    
    /**
     * 知识点 2：发送取消请求
     * 此时子线程正在 while(1) 循环中。
     * 因为有了 pthread_testcancel()，这个 cancel 请求才能生效。
     */
    ret = pthread_cancel(thread_id);
    printf("ret: %d\n",ret);
    THREAD_ERROR_CHECK(ret, "pthread_cancel");
    
    // pthread_join 会阻塞在这里，直到子线程退出
    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");
    return 0;
}

