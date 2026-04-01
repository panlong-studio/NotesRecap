#include <my_header.h>

void *thread_func(void *arg){
    printf("I am son\n");
    /* * 知识点 1：子线程回传数据的存储选择 (重要！)
     * - 必须使用 malloc 在“堆”上申请内存。
     * - 绝对不能 return 局部变量的地址（栈地址），因为子线程一结束，其栈帧立即销毁。
     * - 返回的是一个地址，通过 return (void *) 指针名 传递出去。
     */
    int *pInt = (int *)malloc(4);
    *pInt = 10;
    // 将堆空间地址返回给“收件人”
    return (void *)pInt;
}


int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id;
    //pthread_create的第四个参数可以将值传递给线程入口参数
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    printf("I am main\n");
    
    /* * 知识点 2：接收返回值的变量 (void *retval)
     * - 定义一个 void 指针，用来存放子线程 return 的那个地址。
     * - 此时 retval 的值还是随机的。
     */
    void *retval;
    /* * 知识点 3：pthread_join 的二级指针参数 (核心难点)
     * - 功能：pthread_join 会把子线程返回的指针（void *pInt）写入到 &retval 指向的内存里。
     * - 为什么要传 &retval？
     * 因为 pthread_join 需要把“子线程返回的地址”写入主线程中的 retval 变量。
     * 想让函数内部修改调用者的指针变量本身，就必须传入这个指针变量的地址，因此参数类型是二级指针 void **。
     * - 成功后：retval 就不再是 NULL 或随机值，而是指向了子线程 malloc 的那块内存。
     */
    ret = pthread_join(thread_id, &retval);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");
    
    /* * 知识点 4：类型还原与使用
     * - 从 retval 拿到的依然是通用的 void* 类型，需要强制转换为原本的 int*。
     * - 此时主线程可以安全地读取子线程的返回值。
     */
    int *ptmp = (int *)retval;
    printf("ptmp: %d\n", *ptmp);

    /* * 知识点 5：内存回收的责任转移
     * - 虽然内存是子线程申请的，但子线程已经“去世”了。
     * - 主线程在用完数据后，必须负责 free(ptmp)，否则会造成内存泄漏。
     */
    free(ptmp);
    ptmp = NULL;
    
    return 0;
}

