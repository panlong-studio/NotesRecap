#include <my_header.h>

void func(){
    printf("func\n");
    /* * 知识点 1：pthread_exit 函数 (线程的“紧急出口”)
     * - 功能：终止调用它的线程。
     * - 特点：无论在哪个函数（哪怕是深层嵌套的子函数）中调用，都会立即结束整个线程。
     * - 参数 (void *retval)：这就是线程的退出状态码。主线程 join 时拿到的就是这个值。
     * - 后果：该行代码之后的所有代码（包括本函数和调用者函数中的）统统不会执行。
     */
    pthread_exit(NULL);
    // 由于 pthread_exit 已经终结了线程，这一行 printf 永远不会被执行
    printf("+++++\n");
}

void *thread_func(void *arg){
    printf("I am son\n");
    func();
    /* * 知识点 3：级联终止
     * 虽然 func() 只是 thread_func 调用的一个子函数，但因为 func 内部执行了 pthread_exit，
     * 整个子线程在 func 内部就断掉了，程序流永远不会回到这里。
     * 所以下面这一行printf也不会被执行。
     */
    printf("func over int thread_func\n");
    return NULL;
}


int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id;
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    printf("I am main\n");
    
    /* * 知识点 4：join 依然有效
     * 即使子线程是通过 pthread_exit 提前退出的，主线程的 pthread_join 依然能正常
     * 捕获到它退出的信号，并回收其资源。
     */
    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");

    return 0;
}

