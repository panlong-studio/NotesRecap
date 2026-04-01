#include <my_header.h>

void free_func(void *p)
{
    printf("free_func\n");
    char *ptmp = (char *)p;
    free(ptmp);
    ptmp = NULL;
}

void close_func(void *p)
{
    printf("close_func\n");
    int *fd = (int *)p;
    close(*fd);
}

void *thread_func(void *arg){
    // 堆空间资源
    char *pp = (char *)malloc(10);
    strcpy(pp, "hello");
    pthread_cleanup_push(free_func, pp);

    // 文件描述符资源
    int fd = open("./Makefile", O_RDONLY);
    pthread_cleanup_push(close_func, (void *)&fd);

    sleep(3);
    printf("son\n");

    /* * 知识点 1: pthread_exit 与 清理程序的自动触发
     * 当线程调用 pthread_exit() 时，系统会自动弹出（pop）并执行当前栈中
     * 所有的清理处理程序。
     * 顺序依然遵循 LIFO：先执行 close_func，再执行 free_func。
     */
    pthread_exit(NULL);

    /* * 知识点 2: return 与 pthread_exit 的关键区别 (重点！)
     * 如果你在这里使用 `return NULL;` 而不是 `pthread_exit(NULL);`：
     * 清理程序 **不会** 被触发。这会导致内存泄漏和文件描述符未关闭。
     * 记住：在 push/pop 块中，如果要正常退出线程并确保清理，必须用 pthread_exit。
     */
    /* return NULL; */

    /* * 知识点 3: 死代码（Dead Code）
     * 由于上面调用了 pthread_exit，下面的代码永远不会被执行。
     * 但 pthread_cleanup_pop 必须写在这里，因为 push/pop 在语法上必须成对。
     */
    pthread_cleanup_pop(1);
    pthread_cleanup_pop(1);

    printf("son over\n");
    pthread_exit(NULL);
}

int main(int argc, char *argv[]){                                  
   
    pthread_t thread_id;
    int ret = pthread_create(&thread_id, NULL, thread_func, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_create");

    sleep(1);
    printf("I am main\n");

    /* * 知识点 4: pthread_join 的同步作用
     * 此时主线程没有调用 pthread_cancel。
     * 主线程会阻塞在 join 处，直到子线程自己运行到 pthread_exit。
     * 子线程退出时自动执行清理函数，随后主线程被唤醒，打印 "over"。
     */
    ret = pthread_join(thread_id, NULL);
    THREAD_ERROR_CHECK(ret, "pthread_join");
    printf("over\n");
    
    return 0;
}

