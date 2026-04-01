#include <my_header.h>

/* 普通函数中的 return
 * 在普通函数中调用 return，仅仅是结束当前函数的执行，
 * 将控制权返回给调用者（caller），不会导致整个线程结束。
 */
void func(){
    printf("func\n");
    return;
}

/*
    线程入口函数 thread_func
    ----------------------------
    知识点1：为什么线程函数写成 void *thread_func(void *arg) ?
    - pthread_create 要求线程入口函数的类型必须是：
          void *(*start_routine)(void *)
    - start_routine 是一个函数指针，它指向的函数类型是：参数为 void *，返回值为 void *
      即：
          参数：void *
          返回值：void *
    - 参数 void *arg 用来接收主线程传递给子线程的数据
    - 返回值 void * 可以作为“线程退出时的结果”返回给 pthread_join

    知识点2：线程“主动退出”的几种方式
    - 在线程函数中执行 return 某个值;
    - 在线程函数中调用 pthread_exit(某个值);
    - 被其他线程取消（pthread_cancel）【这是另一类机制】
*/
void *thread_func(void *arg){
    printf("I am son\n");
    func();
    printf("func over int thread_func\n");
    return NULL;
    /* 线程入口函数中的 return
     * 1. 行为：在线程主函数中执行 return，等同于调用了 pthread_exit()。
     * 2. 结果：这会导致该子线程“主动退出”，释放其私有的栈空间。
     * 3. 返回值：return 后面的值（如 NULL）会被传递给 pthread_join 的第二个参数。
     */
}


int main(int argc, char *argv[]){                                  
    
    pthread_t thread_id; // 定义线程标识符（本质是无符号长整型）
    //pthread_create的第四个参数可以将值传递给线程入口参数
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL); //创建了子线程
    /* pthread_create 创建线程
     * 参数1: 线程ID的地址
     * 参数2: 线程属性（NULL表示默认）
     * 参数3: 线程入口函数地址（从该函数开始运行）
     * 参数4: 传递给入口函数的参数（NULL表示不传参）
     */
    THREAD_ERROR_CHECK(ret, "pthread_create");
    
    printf("I am main\n");
    
    /* pthread_join 线程等待
     * 作用：这是一个阻塞调用。主线程会停在这里，直到 ID 为 thread_id 的线程结束。
     * 意义：
     * 1. 回收资源：类似进程的 wait，防止出现类似“僵尸进程”的残留资源。
     * 2. 获取返回值：第二个参数可以用来接收 thread_func 中 return 的那个指针。
     */
    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");

    return 0;
}

