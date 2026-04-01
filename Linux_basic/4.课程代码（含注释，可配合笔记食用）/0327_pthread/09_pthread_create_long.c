#include <my_header.h>

void *thread_func(void *arg){
    printf("I am son\n");
    /* * 知识点 1：将指针强制转换回数值 (Passing by Value)
     * - 这里的 arg 并不是一个有效的内存地址，而是主线程直接传过来的数字 10。
     * - 我们将 void * 强制转换为 long，从而直接拿到了这个“值”。
     * - 注意：这里 tmp 是子线程栈上的局部变量，对其修改不会影响主线程。
     */
    long tmp = (long)arg;
    tmp = 30; // 这里的修改仅在子线程内部有效
    printf("tmp: %ld\n", tmp);
    
    return NULL;
}


int main(int argc, char *argv[]){                                  
    
    //特殊情况: long型
    /* * 知识点 2：为什么是 long 型？
     * - 在 64 位 Linux 系统中，指针占 8 字节，long 也占 8 字节。
     * - 如果用 int（4字节），在强制转换时可能会触发编译器的警告或数据截断。
     * - 使用 long 可以完美“填满”指针参数的位宽，确保数据完整。
     */
    long num = 10;
    pthread_t thread_id;
    /* * 知识点 3：传值 vs 传址 (核心差异)
     * - 传址 : 传的是 &num，子线程通过地址修改主线程的变量。
     * - 传值 : 传的是 (void *)num，即将 10 这个数字强行解释为一个地址发过去。
     * - 优点：子线程拥有了该数值的一个独立副本，不需要担心主线程变量生命周期的问题
     * - 即使主线程销毁了 num，子线程手里的 10 依然存在。
     */
    int ret = pthread_create(&thread_id, NULL, thread_func, (void *)num); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    printf("I am main\n");
    printf("***num: %ld\n", num);

    sleep(5); //为了让主线程等待子线程执行完毕
    /* * 知识点 4：验证独立性
     * 由于这次是“传值”，子线程里对 tmp 的任何修改，都不会反馈到主线程的 num 上。
     */
    printf("num: %ld\n", num);
    
    return 0;
}

