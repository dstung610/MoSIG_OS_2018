#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define LOOPS 1000000

int count = 0;

void spinlock_lock(void)
{
    /* TODO */
}

void spinlock_unlock(void)
{
    /* TODO */
}


void* thread_routine(void* arg)
{
    int i =0;
    
    for(i=0; i<LOOPS; i++){
        spinlock_lock();
        
        count++;
        
        spinlock_unlock();
    }
    
    return NULL;
}

int main(int argc, char **argv)
{
    int i ;
    int nb_threads = 0 ;
    pthread_t *tids;
    
    if (argc != 2){
        fprintf(stderr, "usage : %s nb_threads\n", argv[0]) ;
        exit (-1) ;
    }
    
    nb_threads = atoi (argv[1]) ;
    tids = malloc (nb_threads*sizeof(pthread_t)) ;
    
    for (i = 0 ; i < nb_threads; i++){
        if(pthread_create (&tids[i], NULL, thread_routine, NULL) != 0){
            fprintf(stderr,"Failed to create thread number %d\n",i);
        }
    }
    
    /* Wait until every thread ended */ 
    for (i = 0; i < nb_threads; i++){
        pthread_join (tids[i], NULL) ;
    }
    
    free (tids) ;
    
    printf("the result of the computation is %d\n",count);
    
    return EXIT_SUCCESS;
}
