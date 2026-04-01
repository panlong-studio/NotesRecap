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
    
    /* * 知识点 1：手动分配“指针容器”
     * - malloc(100) 在堆上申请了空间。
     * - retval 现在指向堆上一块合法的、大小为 100 字节的区域。
     * - 这块空间足以容纳子线程传回来的那个 8 字节的地址（void*）。
     */
    void **retval =(void **)malloc(100);
    /* * 知识点 2：合法的内存写入
     * - pthread_join 接收到 retval 这个合法的堆地址。
     * - 它将子线程 return 的 pInt 地址，写到了 retval 指向的那块堆内存的前 8 个字节里。
     */
    ret = pthread_join(thread_id, retval);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");
    
    /* * 知识点 3：三层解析逻辑
     * 1. *retval: 取出堆空间里存着的那个地址（即子线程里的 pInt）。
     * 2. (int *)*retval: 把这个地址转成 int 指针。
     * 3. *(int *)*retval: 获取该地址指向的具体数值（10）。
     */
    int ptmp = *(int *)*retval;
    printf("ptmp: %d\n", ptmp);

    // 资源释放
    free(*retval);
    free(retval);
    
    return 0;
}

