#include <my_header.h>

/**
 * 知识点 1：pthread_exit 在普通函数中的威力
 * 1. 立即性：一旦执行 pthread_exit，当前【整个线程】直接退出。
 * 2. 穿透性：它不仅退出 func()，还会导致调用它的 thread_func() 也随之结束。
 * 3. 拦截：第12行的 printf 和第18行的 printf 都将被“跳过”，永远不会执行。
 */
void func(){
    printf("func\n");
    // 关键点：让线程提前结束。参数 NULL 会传递给 pthread_join 的第二个参数
    pthread_exit(NULL);
    // 以下代码处于“不可达”状态
    printf("+++++\n");
}

/**
 * 知识点 2：线程执行流的中断
 */
void *thread_func(void *arg){
    printf("I am son\n");
    func(); // 进入 func 后，由于内部调用了 pthread_exit，执行流在此处戛然而止
    // 以下代码也不会执行，因为子线程已经不存在了
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
     * 知识点 3：主线程的视角
     * 即使子线程是在 func() 里通过 pthread_exit 异常“早退”的，
     * 主线程的 pthread_join 依然能正常回收它，并感知到它结束了。
     */
    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");

    /**
     * 知识点 4：在main函数中使用pthread_exit() 会发生什么?
     * 现象：主线程会立即退出，但进程不会立刻销毁。
     * 结果： 进程会一直等到所有子线程都运行结束后才真正结束。
     * 这和 return 0 或 exit(0) 完全不同（后者都会瞬间杀掉所有子线程）。
     */

    return 0;
}

