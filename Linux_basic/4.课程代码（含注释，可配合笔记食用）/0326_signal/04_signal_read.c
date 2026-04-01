#include <my_header.h>

void func(int num){
    printf("num: %d\n", num);
}

int main(int argc, char *argv[]){                                  
    

    signal(2, func);
    
    char buf[100] = {0};
    read(STDIN_FILENO, buf, sizeof(buf));

    printf("read over.\n");

    return 0;
}

