#include <my_header.h>

void *thread_func(void *arg){
    printf("I am son\n");
    
    int *pInt = (int *)malloc(4);
    *pInt = 10;

    return (void *)pInt;
}


int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id;
    //pthread_create的第四个参数可以将值传递给线程入口参数
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    printf("I am main\n");
    
    /* * 知识点 1：未初始化的二级指针 (野指针风险)
     * - retval 是一个二级指针变量，它在栈上占用 8 字节，但它的“值”是随机的。
     * - 它现在指向一个未知的内存地址。
     */
    /* void **retval =(void **)malloc(100); //正确写法 */
    void **retval; //错误写法
    /* * 知识点 2：错误的参数传递
     * - pthread_join 要求传入一个【有效的内存地址】，用来写入子线程的返回值。
     * - 这里你直接传了 retval（一个随机地址）。
     * - 结果：pthread_join 会尝试往那个“随机地址”写数据，导致 Segment Fault (段错误)。
     * * 正确思路：要么用 &retval(一级指针取地址)，要么像正确写法那样 malloc。
     */
    ret = pthread_join(thread_id, retval);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");

    int ptmp = *(int *)*retval;
    printf("ptmp: %d\n", ptmp);
    
    return 0;
}

