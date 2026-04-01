#include <my_header.h>

/**
 * 知识点 1：清理回调函数 (Routine)
 * 1. 签名要求：必须是 void func(void *arg) 格式。
 * 2. 作用：当线程被取消或主动调用 pthread_exit 时，系统会自动调用这些函数。
 */
void free_func(void *p){
    printf("free_func\n");
    char *ptmp = (char *)p;
    free(ptmp); // 确保堆内存被释放
    ptmp = NULL; 
}

void close_func(void *p){
    printf("close_func\n");
    int *fd = (int *)p; // 这里的 p 是 fd 的地址
    close(*fd); // 确保文件描述符被关闭
}

void *thread_func(void *arg){
        
    //堆空间资源
    char *pp = (char *)malloc(10);
    strcpy(pp, "hello");
    /**
     * 知识点 2：pthread_cleanup_push (压栈)
     * 1. 作用：将清理函数及其参数压入该线程私有的“清理栈”。
     * 2. 时机：紧跟在资源申请（malloc/open/lock）之后。
     * 3. 匹配：push 和 pop 必须在同一个作用域内【成对出现】，否则编译报错。
     */
    pthread_cleanup_push(free_func, pp);

    //文件符描述资源
    int fd = open("./Makefile", O_RDONLY);
    pthread_cleanup_push(close_func, (void *)&fd);
    
    /**
     * 知识点 3：响应取消
     * 子线程执行 sleep(3) 时，如果主线程发起了 pthread_cancel：
     * 系统会检测到取消信号，并自动【逆序】弹出清理栈里的函数。
     * 先调用 close_func，再调用 free_func。这就保证了即便 printf("son") 没运行，资源也释放了。
     */
    sleep(3);
    printf("son\n");
    /**
     * 知识点 4：pthread_cleanup_pop (弹栈)
     * 参数 1 (execute):
     * - 若为 1：弹出函数并【立即执行】它。
     * - 若为 0：只弹出函数，【不执行】它。
     * 在这里写 1，相当于手动触发了清理，同时也把保险撤销了（因为资源已经手动释放了）。
     */
    pthread_cleanup_pop(1); // 弹出并执行 close_func
    pthread_cleanup_pop(1); // 弹出并执行 free_func
    
    printf("son over\n");

    pthread_exit(NULL);

    /* 如果这里不撤销（不 pop）会怎样？ */
    /* 由于 push 和 pop 在底层是用宏实现的，带有大括号 { 和 }。 */
    /* 如果你只 push 不 pop，你的代码作用域就没闭合，编译器会直接报错：error: expected ‘}’ before ...。 */

    /* 总结: */
    /* push： 挂上保险（预防意外退出时的资源泄露）。 */
    /* pop(1)： 任务完成，取下保险（撤销）并执行清理。 */
    /* pop(0)： 任务完成，取下保险（撤销）但不执行清理（如果已经手动完成清理）。 */
}


int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id;
    //pthread_create的第四个参数可以将值传递给线程入口参数
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    sleep(1);
    printf("I am main\n");
    
    // 发起取消请求。由于子线程设置了清理栈，这里不再担心资源泄漏
    //通过加入资源清理,我们解决了pthread_cancel让子线程被动退出时,资源没来得及被清理的问题
    ret = pthread_cancel(thread_id);
    printf("ret: %d\n",ret);
    THREAD_ERROR_CHECK(ret, "pthread_cancel");
    
    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");
    

    return 0;
}

