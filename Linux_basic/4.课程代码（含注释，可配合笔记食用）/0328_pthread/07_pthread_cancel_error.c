#include <my_header.h>

/**
 * 知识点 1：线程被动退出导致的“遗留问题”
 * 1. 资源持有：子线程申请了堆空间（malloc）和文件描述符（open）。
 * 2. 致命打击：主线程在 sleep(1) 后发起 cancel。
 * 3. 响应点：子线程在执行 sleep(3) 时响应了取消请求，直接退出。
 * 4. 后果：第15行的 free(pp) 和第16行的 close(fd) 根本没有被执行的机会！
 * 这将直接导致：内存泄漏 (Memory Leak) 和 文件描述符泄漏。
 */
void *thread_func(void *arg){
        
    //堆空间资源
    char *pp = (char *)malloc(10);
    strcpy(pp, "hello");

    //文件符描述资源
    int fd = open("./Makefile", O_RDONLY);
    
    // 取消点：线程通常在这里被主线程杀死
    sleep(3);

    // --- 以下代码在被 cancel 的情况下变成了“死代码”，不会执行 --
    printf("son\n");

    free(pp);
    close(fd);
    printf("son over\n");

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
     * 知识点 2：pthread_cancel 的局限性
     * 当你调用 cancel 时，你只管“杀”，不管“埋”。
     * 被取消的线程会立即停止执行后续逻辑，导致它手里拿着的资源（锁、内存、文件）瞬间变成孤儿。
     */
    // 如果不写pthread_cancel函数,让子线程自己正常退出,则子线程的资源会自己清理
    // 但如果主线程让子线程被动退出,则有资源可能没有来得及清理,这就会造成资源浪费
    ret = pthread_cancel(thread_id);
    printf("ret: %d\n",ret);
    THREAD_ERROR_CHECK(ret, "pthread_cancel");
    
    /**
     * 知识点 3：回收线程并不等于回收资源
     * pthread_join 只能回收线程的 PCB（进程控制块）等内核资源，
     * 它无法帮子线程去 free 内存或 close 文件。
     */
    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");
    

    return 0;
}

