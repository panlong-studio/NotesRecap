#include <my_header.h>

void *thread_func(void *arg){
    printf("I am son\n");
    /* * 知识点 1：将数值伪装成指针返回 (Return by Value)
     * - 虽然 return 要求一个 void* 类型的地址，但我们直接把 long 型变量 num 传给它。
     * - 本质：这并不是在返回一个地址，而是直接把 100 这个二进制数值填入了
     * 专门存放返回值的寄存器（或内存位置）中。
     * - 优势：不需要 malloc，不涉及堆空间，运行速度极快。
     */
    long num = 100;

    return (void *)num;
}


int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id;
    //pthread_create的第四个参数可以将值传递给线程入口参数
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    printf("I am main\n");
    
    /* * 知识点 2：接收载体 void *retval
     * - 此时 retval 只是主线程栈上的一个 8 字节空间。
     */
    void *retval;

    /* * 知识点 3：pthread_join 的数据搬运
     * - 当子线程执行 return (void *)100 时，系统会把 100 这个值
     * “拷贝”到 &retval 指向的这 8 字节空间里。
     * - 执行完此行后，retval 的值就变成了 100（即 0x0000000000000064）。
     */
    ret = pthread_join(thread_id, &retval);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");
    
    /* * 知识点 4：将指针强制转回数值
     * - 重点：千万不要对此时的 retval 进行解引用（即不要写 *retval）。
     * - 因为 retval 存的是数值 100，如果你把它当地址去访问（*retval），
     * 程序会尝试访问内存地址为 100 的地方，直接导致 Segfault 崩溃。
     * - 正确做法：直接强转回 long 型，拿回我们的数值。
     */
    long ptmp = (long)retval;
    printf("ptmp: %ld\n", ptmp);
    
    return 0;
}

