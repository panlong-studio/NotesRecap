#include <my_header.h>

void *thread_func(void *arg){
    /* * 知识点 1：pthread_self() 函数
     * - 功能：获取当前调用线程自身的线程 ID。
     * - 场景：子线程不知道主线程给它分配的 thread_id 变量值时，可以通过此函数“自报家门”。
     * - 类型：%ld：在 Linux (x86_64) 下，pthread_t 通常是 long unsigned int。
     */
    printf("***son thread id: %ld\n", pthread_self());
    printf("I am son\n");
    return NULL;
}

int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id;
    /* * 知识点 2：pthread 系列函数的返回值
     * - 区别：大多数 Linux 系统调用（如 open, read）成功返回 0，失败返回 -1 并设置 errno。
     * - 特点：pthread 库函数成功返回 0，失败直接返回“错误码”（正整数）。
     */
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程
    THREAD_ERROR_CHECK(ret, "pthread_create");
    /* * 知识点 3：主线程视角下的子线程 ID
     * 这里打印的是 pthread_create 第一个参数传出的值。
     * 注意：主线程打印的 thread_id 和子线程内部 pthread_self() 的值应当是完全一致的。
     */
    printf("son thread id: %ld\n", thread_id);
    /* * 知识点 4：主线程也是“线程”
     * 主线程（main 所在的执行流）同样拥有线程 ID。
     * 在 Linux 内核看来，主线程只是进程中的第一个线程（主执行流）。
     */
    printf("main thread id: %ld\n", pthread_self());

    printf("I am main\n");
    while(1); //主线程会等待子线程
    return 0;
}

