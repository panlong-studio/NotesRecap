#include <my_header.h>

void *thread_func(void *arg){
    printf("I am son\n");
    /* * 知识点 1：局部变量的本质 (栈空间)
     * - num 是 thread_func 内部定义的局部变量，它分配在子线程的【私有栈】上。
     * - 只要 thread_func 执行完毕（遇到 return 或 pthread_exit），
     * 该线程的栈帧就会被标记为释放（无效）。
     */
    int num = 100;
    /* * 知识点 2：致命错误 —— 返回栈地址
     * - 这里将局部变量 num 的内存地址传回给了主线程。
     * - 现状：函数结束的一瞬间，这块内存就不再属于 num 了，它随时可能被系统分配给其他数据。
     */
    return (void *)&num; //局部变量的地址
                         //函数结束之后局部变量已经被销毁
}


int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id;
    //pthread_create的第四个参数可以将值传递给线程入口参数
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    printf("I am main\n");
    
    void *retval;
    /* * 知识点 3：接收到的是“幽灵地址”
     * - pthread_join 成功执行后，retval 确实拿到了 num 曾经待过的那个地址。
     * - 但此时子线程已经销毁，那个地址指向的内存已经变成了“非法区域”或“脏数据”。
     */
    ret = pthread_join(thread_id, &retval);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");
    /* * 知识点 4：非法解引用 (Segmentation Fault 风险)
     * - ptmp 指向了一个已经失效的栈地址。
     * - 执行 *ptmp 时，如果这块内存还没被系统收回，你可能运气好能读到 100；
     * - 但绝大多数情况下，你会读到一个随机数，或者直接触发【段错误】导致程序崩溃。
     */
    int *ptmp = (int *)retval;
    printf("ptmp: %d\n", *ptmp);
    
    return 0;
}

