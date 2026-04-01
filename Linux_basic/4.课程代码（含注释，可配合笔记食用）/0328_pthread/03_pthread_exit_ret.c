#include <my_header.h>

/**
 * 知识点 1：通过 pthread_exit 传递返回值
 * 1. 参数类型：pthread_exit 接收的是一个 void * 类型的指针。
 * 2. 技巧：这里将 long 类型的 num 强制转换为 (void *)。这是一种常用技巧，
 * 直接把“数值”存在“指针变量本身”的位置，避免了申请内存的麻烦。(这里long和指针的大小都是8字节)
 * 3. 影响：调用后，线程立即终止，控制权直接交给主线程的 pthread_join。
 */
void func(){
    printf("func\n");
    long num = 10;
    // 立即退出整个线程，并将 10 这个值作为“遗言”留下
    pthread_exit((void *)num);//会让线程提前执行结束，那么后面的代码不会执行
    printf("+++++\n");
}

void *thread_func(void *arg){
    printf("I am son\n");
    func(); // 线程在这里就结束了，不会执行后续代码
    printf("func over int thread_func\n");
    return NULL;
}


int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id;
    //pthread_create的第四个参数可以将值传递给线程入口参数
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    printf("I am main\n");
    
    /**
     * 知识点 2：使用 pthread_join 回收返回值
     * 1. 变量准备：定义一个 void *retval，用来接收子线程传回来的指针。
     * 2. 传递地址：注意这里传的是 &retval（二级指针）。
     * join 函数内部会修改 retval 的值，使其指向子线程退出时提供的那个地址/数值。
     */
    void *retval;
    ret = pthread_join(thread_id, &retval);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");
    
    /**
     * 知识点 3：类型转换与数据还原
     * 1. 还原：因为子线程传过来时强转成了 void*，主线程用时需要强转回对应的类型（long）。
     * 2. 注意：这种直接传值的方法只适用于能塞进指针大小（64位系统通常是8字节）的数据。
     */
    long tmp = (long)retval;
    printf("tmp: %ld\n", tmp); // 输出结果为 10

    return 0;
}

