#include <my_header.h>

/* * 知识点 1：线程安全性 (Thread Safety)
 * - func() 是一个普通的全局函数。
 * - 在多线程环境下，多个线程可以【同时】进入并执行同一个函数。
 * - 如果 func 内部只有 printf 或局部变量，它是“线程安全”的；
 * - 如果 func 内部修改了全局变量且没加锁，它就是“非线程安全”的。
 */
void func(){
    printf("func\n");
    return; // 仅从当前函数 func 返回，回到 thread_func
}

void *thread_func(void *arg){
    printf("I am son\n");
    /* * 知识点 2：线程内的函数调用栈 (Thread Stack)
     * - 当子线程调用 func() 时，会在该子线程私有的【栈空间】中压入 func 的栈帧。
     * - 这证明了：子线程不仅是一个简单的执行入口，它拥有完整的函数调用能力。
     */
    func();
    /* * 知识点 3：执行流的回退
     * - func() 执行完 return 后，控制权回到 thread_func 继续向下执行。
     * - 注意：在 func 中执行 return 不会导致整个线程结束，只有在 thread_func 中
     * 执行 return 或调用 pthread_exit 才会结束线程。
     */
    printf("func over int thread_func\n");

    return NULL;
}


int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id;
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    printf("I am main\n");
    
    /* * 知识点 4：同步点的确立
     * - pthread_join 确保了主线程会一直等到子线程把 thread_func(包括里面的 func)
     * 全部跑完并返回后，才会继续执行打印 "over"。
     */
    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");

    return 0;
}

