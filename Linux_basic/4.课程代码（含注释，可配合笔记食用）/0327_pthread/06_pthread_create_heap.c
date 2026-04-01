#include <my_header.h>

void *thread_func(void *arg){
    /* * 知识点 1：通用的参数接收入口 (void *arg)
     * - 传入的 arg 是 pthread_create 的第四个参数。
     * - 因为是 void* 类型，所以使用前必须进行“强制类型转换”回原始类型（这里是 char*）。
     * - 注意：这里 pp 指向的内存地址和主线程中的 ptmp 是完全一样的。
     */
    char *pp = (char *)arg;
    printf("I am son\n");
    strcpy(pp, "hello from child thread");
    /* * 知识点 2：跨线程修改共享内存
     * - 子线程通过 strcpy 直接修改了主线程在堆上申请的内存。
     * - 由于子线程和主线程共享同一个“进程地址空间”，所以子线程对堆空间的修改
     * 在子线程返回后，主线程依然能够看到。
     */
    return NULL;
}

int main(int argc, char *argv[]){                                  
    /* * 知识点 3：堆空间作为通信媒介
     * - 为什么用 malloc 而不是局部变量？
     * - 局部变量在栈上，如果主线程函数执行完毕，栈帧销毁，子线程访问就会崩溃。
     * - 堆空间（Heap）在整个进程生命周期内有效，直到手动 free，是线程间传递大块数据的首选方案。
     */
    char *ptmp = (char *)malloc(50); //堆空间

    pthread_t thread_id;
    /* * 知识点 4：pthread_create 的第四个参数 (传参关键)
     * - 作用：这是向子线程传递数据的“唯一通道”。
     * - 传递的是“地址”：这里传递的是 ptmp 指针的值（即堆空间的起始地址）。
     * - 灵活性：如果需要传递多个参数，可以定义一个 struct 结构体，然后把结构体地址传进去。
     */
    int ret = pthread_create(&thread_id, NULL, thread_func, (void *)ptmp); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");

    printf("I am main\n");
    // 等待子线程写完数据
    sleep(5);
    /* * 知识点 5：读取“异步”修改后的结果
     * 主线程在 sleep 结束后，直接访问 ptmp，打印出的内容就是子线程刚才 strcpy 进去的字符串。
     */
    printf("ptmp: %s\n", ptmp);
    
    /* * 知识点 6：内存管理的原则
     * - 谁申请，谁释放：虽然子线程使用了这块内存，但习惯上仍由主线程（或管理线程）负责 free。
     * - 避免悬空指针：ptmp = NULL 是一流 C 程序员的自我修养，防止后续代码误用已释放的内存。
     */
    free(ptmp);
    ptmp = NULL; //保持好习惯

    return 0;
}

