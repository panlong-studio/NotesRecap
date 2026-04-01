#include <my_header.h>

/* * 知识点 1：线程函数指针
 * 线程执行的入口函数必须符合固定格式：void* 函数名(void* 参数)
 * - 返回值 void*: 允许线程返回任意类型的指针给主线程（通过 pthread_join 获取）。
 * - 参数 void* arg: 允许主线程向子线程传递任意类型的数据。
 */
void *thread_func(void *arg){
    printf("I am son\n");
    return NULL;
}

int main(int argc, char *argv[]){                                  
    
    //主线程: main线程
    //子线程: 通过pthread_create创建的线程

    /* * 知识点 2：pthread_t 线程标识符
     * 这是用来存储线程 ID 的类型。在 Linux 下，它通常是一个无符号长整型（unsigned long）。
     * 后续对该线程的控制（如等待、取消）都需要用到这个变量。
     */
    pthread_t thread_id;
    /* * 知识点 3：pthread_create 函数
     * 参数含义：
     * 1. &thread_id: 传出参数，用于保存新创建线程的 ID。
     * 2. NULL: 线程属性。传 NULL 表示使用默认属性（如默认栈大小、非分离状态等）。
     * 3. thread_func: 函数指针，指定子线程要执行的任务。
     * 4. NULL: 传递给 thread_func 的参数。如果不需要传参则设为 NULL。
     * * 返回值：成功返回 0，失败返回错误码（注意：它不设置 errno）。
     */
    pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程
    
    printf("I am main\n");
    while(1); //主线程会等待子线程
    /* * 知识点 4：主线程与进程生命周期
     * 在 Linux 中，进程是资源的分配单位。如果 main 函数（主线程）执行 return 0 退出，
     * 整个进程会被销毁，所有的子线程也会被强制终止，无论它们是否执行完毕。
     * * 这里使用 while(1) 是为了防止主线程退出，确保你能看到子线程的输出。
     * 进阶知识：在实际开发中，通常使用 pthread_join(thread_id, NULL) 来实现优雅等待。
     */
    return 0;
}

