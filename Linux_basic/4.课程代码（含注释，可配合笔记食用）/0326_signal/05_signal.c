#include <my_header.h>

void func(int num){
    printf("begin.\n");
    sleep(10);
    printf("end.\n");
}

void func2(int num){
    printf("func2 begin.\n");
    sleep(10);
    printf("func2 end.\n");
}

int main(int argc, char *argv[]){                                  
    
    signal(2, func);
    signal(3, func2);
    
    printf("while.\n");
    while(1);
    
    return 0;
}

