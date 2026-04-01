#include <my_header.h>

void *thread_func(void *arg){
    /* * 知识点 1：指针间接访问 (Dereferencing)
     * - 子线程通过强制类型转换后的 pint 指针，直接操作主线程栈空间里的变量 num。
     * - 这种方式不需要全局变量，也不需要 malloc，但必须保证主线程中 num 的生命周期。
     */
    printf("I am son\n");
    int *pint = (int *)arg;
    *pint = 30; // 通过地址修改主线程局部变量的值
    return NULL;
}

int main(int argc, char *argv[]){                                  
    /* * 知识点 2：主线程栈变量的生命周期
     * - num 是 main 函数里的局部变量，存储在主线程的“栈”区。
     * - 只要 main 函数没返回（return 0 之前），这个 num 的内存地址就是有效的。
     */
    int num = 10; //主线程中的变量10

    pthread_t thread_id;
    /* * 知识点 3：传递栈地址的风险
     * - 这里通过 (void *)&num 将局部变量的地址传给了子线程。
     * - 安全前提：主线程必须确保在子线程访问 num 期间，main 函数不会退出或 num 不会被销毁。
     * - 如果这是在一个普通子函数中定义的 num，而该函数在子线程运行前就结束了，
     * 子线程再去修改 *pint 就会引发“非法访问”或破坏其他函数的栈帧。
     */
    int ret = pthread_create(&thread_id, NULL, thread_func, (void *)&num); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    /* * 知识点 4：并发下的数据一致性观察
     * 在这里打印 num，结果可能是 10，也可能是 30。
     * 这取决于主线程打印得快，还是子线程赋值操作跑得快（线程竞争）。
     */
    printf("***num: %d\n", num);
    printf("I am main\n");
    // 知识点 5：sleep(5) 充当了临时的同步手段
    // 确保子线程有充足的时间完成 *pint = 30 的赋值操作。
    sleep(5);
    // 此时打印结果必然是 30
    printf("num: %d\n", num);
    
    return 0;
}

