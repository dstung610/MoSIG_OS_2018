#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>


int main(void)
{
    int pagesize= getpagesize();
    void* addr;
    char c;
    
    printf("PID of the created process: %d\n", getpid());

    printf("The page size is: %d\n",pagesize);

    addr = malloc(pagesize);

    printf("The address of the block is \t %p\n",addr);

    printf("\nPress enter to terminate the program\n");

    fread(&c, 1, 1, stdin);

    return 0;
}
