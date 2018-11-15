#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(void)
{

    int pagesize= getpagesize();
    void* region;
    char c;
    int fd;

    printf("PID of created process: %d\n", getpid());

    if((fd = open("../lab3.pdf", O_RDONLY)) == -1){
        perror("failed to open file");
        return -1;
    }

    region=mmap(NULL, pagesize, PROT_READ, MAP_PRIVATE, fd, 0);

    printf("mmapped region at address %p\n", region);

    printf("\nPress enter to terminate the program\n");
    
    fread(&c, 1, 1, stdin);
    
    return 0;
}
