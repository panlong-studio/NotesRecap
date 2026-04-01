#include <my_header.h>

/* * 知识点 1: 清理函数的原型
 * 清理函数的类型必须是 void (*)(void *)。
 * 参数 p 是在 pthread_cleanup_push 时传入的资源指针。
 */
void free_func(void *p){
    printf("free_func\n");
    char *ptmp = (char *)p;
    free(ptmp); // 释放堆内存，防止内存泄露
    ptmp = NULL;
}

void close_func(void *p){
    printf("close_func\n");
    int *fd = (int *)p;
    close(*fd); // 关闭文件描述符，防止句柄泄露
}

void *thread_func(void *arg){
    
    char *pp = (char *)malloc(10);
    strcpy(pp, "hello");

    /* * 知识点 2: pthread_cleanup_push
     * 作用：将清理函数压入线程的“清理堆栈”。
     * 触发时机：
     * 1. 线程调用 pthread_exit(3) 退出时。
     * 2. 线程响应 pthread_cancel(3) 取消请求时。
     * 3. 线程调用 pthread_cleanup_pop(3) 且参数非 0 时。
     * 注意：直接调用 return 不会触发清理函数！
     * 这里的(3)表示该函数属于manual的第三节(库调用)
     */
    pthread_cleanup_push(free_func, pp);

    int fd = open("./Makefile", O_RDONLY);

    /* * 知识点 3: 这里的清理函数必须与 push 配对使用。
     * 遵循 LIFO（后进先出）原则，即后 push 的先执行。
     */
    pthread_cleanup_push(close_func, (void *)&fd);

    sleep(3); // 此时主线程会发起 pthread_cancel
    printf("son\n");

    /* * 知识点 4: pthread_cleanup_pop(int execute)
     * 作用：从清理堆栈中弹出一个处理程序。
     * 参数 execute:
     * - 0: 仅弹出，不执行清理函数。
     * - 非0: 弹出并立即执行清理函数。
     * 宏约束：push 和 pop 在代码逻辑上必须成对出现（通常底层是用 do{...}while(0) 宏实现的）。
     */
    pthread_cleanup_pop(1); // 弹出并执行 close_func
    pthread_cleanup_pop(1); // 弹出并执行 free_func

    printf("son over\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id;
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    sleep(1); // 确保子线程已经运行并压入了清理函数
    printf("I am main\n");

    /* * 知识点 5: pthread_cancel
     * 向目标线程发送取消请求。目标线程是否退出取决于其“取消状态”和“取消类型”。
     * 在 sleep(3) 等取消点（Cancellation Point）处，子线程会响应请求并退出。
     * 此时，清理堆栈中尚未被 pop 的函数会被自动调用。
     */
    ret = pthread_cancel(thread_id);
    printf("pthread_cancel ret: %d\n", ret);
    THREAD_ERROR_CHECK(ret, "pthread_cancel");

    /* * 知识点 6: pthread_join
     * 等待线程结束。如果线程是被取消的，其 retval 会被置为 PTHREAD_CANCELED。
     */
    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");

    return 0;
}

