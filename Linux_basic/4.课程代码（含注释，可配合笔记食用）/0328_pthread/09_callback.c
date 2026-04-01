#include <my_header.h>

/**
 * 知识点 1：什么是回调函数？
 * 回调函数（Callback）不是由实现方直接调用的，而是在特定的事件或条件下，
 * 由另外一方（通常是系统、库函数、或者像这里的函数指针）调用的函数。
 * * 这里的 func 和 func2 就是潜在的“回调函数”。
 */
int func(){
    printf("func\n");
    return 10;
}

int func2(){
    printf("func2\n");
    return 20;
}

int main(int argc, char *argv[]){                                  
    
    /* //主动调用 */
    /* func(); */
    /* func2(); */
    

    /**
     * 知识点 2：函数指针的声明 (The Pointer to Function)
     * 语法：返回值类型 (*指针变量名)(参数列表);
     * * int (*pFunc)();
     * 这行代码定义了一个指针 pFunc，它指向一个“返回值为 int 且没有参数”的函数。
     * 注意：(*pFunc) 的括号不能掉，否则就变成了声明一个返回 int* 的函数了。
     */
    int (*pFunc)();
    
    /**
     * 知识点 3：函数名本质上就是地址
     * 在 C 语言中，函数名（如 func）就代表该函数在内存代码段中的【起始地址】。
     * 所以可以将 func 直接赋值给函数指针 pFunc。
     */
    pFunc = func;
    printf("111\n");
    
    /**
     * 知识点 4：间接调用 (Indirect Call)
     * 通过指针来运行函数。这种方式比直接调用更灵活：
     * 你可以在程序运行时根据条件改变 pFunc 指向谁，而不需要在编译阶段死写代码。
     */
    pFunc(); //这里就间接调用了func函数
    printf("222\n");
    
    // 知识点 5：解耦与动态性
    // 同一个指针 pFunc，现在换成指向另一个逻辑（func2）
    pFunc = func2;
    printf("333\n");
    pFunc(); //这里就间接调用了func2函数
    printf("444\n");

    return 0;
}

