#include <my_header.h>

void *thread_func(void *arg){
    while(1){
        // sleep 是一个天然的“取消点” (Cancellation Point)
        // 线程执行到这里会检查是否有 cancel 信号
        sleep(1);
    }
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
     * 知识点 1：发起取消请求
     * pthread_cancel 只是发送一个信号，并不会导致子线程瞬间消失。
     * 子线程会在下一次运行到 sleep(1) 时响应。
     */
    ret = pthread_cancel(thread_id);
    printf("ret: %d\n",ret);
    THREAD_ERROR_CHECK(ret, "pthread_cancel");
    
    /**
     * 知识点 2：回收特殊的返回值
     * 1. 变量准备：void *retval 依然用来接收子线程的“遗言”。
     * 2. 机制：如果线程是被 pthread_cancel 终止的，内核会自动
     * 将该线程的退出码设置为 PTHREAD_CANCELED。
     */
    void *retval;
    ret = pthread_join(thread_id, &retval);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");
    
    /**
     * 知识点 3：识别 PTHREAD_CANCELED (核心重点)
     * 1. 宏定义：在 pthread.h 中，PTHREAD_CANCELED 被定义为 ((void *)-1)。
     * 2. 错误警示：千万不要对 retval 进行解引用操作（如 *(int*)retval）。
     * 因为 -1 作为一个地址是非法的（属于内核空间），强行读取会导致“段错误”(Segmentation fault)。
     * 3. 正确做法：直接将这个“指针值”转换为数值进行比较或打印。
     */
    // #define PTHREAD_CANCELED ((void *)-1)
    if(retval == PTHREAD_CANCELED){
        /* printf("%d\n", *(int*)retval); //error */
        // 正确做法：强转为 long 并打印其代表的数值（通常会打印出 -1）
        printf("retval: %ld\n", (long)retval); //ok
    }

    return 0;
}

