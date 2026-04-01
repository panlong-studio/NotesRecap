#include <my_header.h>

void *thread_func(void *arg){
    sleep(1); //这里的写法是为了控制时序,让func函数中的局部变量num提前被销毁
              //那么在thread_func函数中访问num,就是访问了已经销毁的局部变量
              //这样就会出现问题
    printf("I am son\n");
    int *pint = (int *)arg;
    *pint = 30;
    
    printf("*pint: %d\n", *pint);

    return NULL;
}

void func(){
    int num = 10; //主线程中的10
    pthread_t thread_id;
    //pthread_create的第四个参数可以将值传递给线程入口参数
    int ret = pthread_create(&thread_id, NULL, thread_func, (void *)&num); //创建了子线程                                                                       
    THREAD_ERROR_CHECK(ret, "pthread_create");
    /* sleep(3); //防止func函数直接结束,导致num被清除 */
}

int main(int argc, char *argv[]){                                  
    
    func();
    printf("I am main\n");
    sleep(5); //为了让主线程等待子线程执行完毕
    
    return 0;
}

