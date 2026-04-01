#include <my_header.h>

/* Usage:  */
int main(int argc, char *argv[]){                                  
   
    printf("kill begin\n");
    kill(getpid(), 2);
    printf("kill over\n");
    
    return 0;
}

