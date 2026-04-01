#include <my_header.h>

void *thread_func(void *arg){
    printf("I am son\n");
    /* *
     * 子线程进入死循环，意味着它永远不会执行到 return。
     * 这会导致尝试“回收”它的主线程进入永久等待状态。
     */
    while(1);
    return NULL;
}


int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id;
    //pthread_create的第四个参数可以将值传递给线程入口参数
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    printf("I am main\n");
    /* * 知识点 1：pthread_join (线程等待/回收) —— 核心重点
     * - 功能：阻塞等待 thread_id 指定的线程结束。
     * - 行为：如果子线程还在运行，主线程会在此处“挂起（Block）”，立即让出 CPU 资源。
     * - 资源回收：子线程退出后，其残留的资源（如私有栈、线程描述符等）由 pthread_join 负责清理。
     * - 类似于进程中的 wait() 函数。
     */
    
    /* * 知识点 2：第二个参数 (void **retval)
     * - 作用：用来接收子线程 return 出来的那个指针。
     * - 现状：这里传 NULL，表示主线程虽然关心子线程什么时候死，但并不关心它死前留下了什么（返回值）。
     */
    pthread_join(thread_id, NULL);
    
    /* * 知识点 3：不可达的代码 (Unreachable Code)
     * 由于子线程里有个 while(1)，它永远不会结束。
     * 导致主线程会永久卡在 pthread_join，下面的 "over" 永远不会被打印。
     */
    printf("over\n");
    
    return 0;
}

