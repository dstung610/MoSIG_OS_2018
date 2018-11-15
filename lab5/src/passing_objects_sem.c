#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define LOOPS 20

#define BUF_SIZE 4

int buffer[BUF_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t emptyCount;
sem_t count;
int in = 0;
int out = 0;

void pushing(int val)
{
    /* TO DO */

    sem_wait(&emptyCount);
    buffer[in] = val;
    in = (in + 1) % BUF_SIZE;
    printf("pushing value %d\n",val);
    sem_post(&count);


    /* NOTE: pay attention to where to put the call to printf to be
       sure it can be helpful for debugging */

}

int fetching(void)
{
    int val=0;

    /* TO DO */
    sem_wait(&count);
    val = buffer[out];
    out = (out + 1) % BUF_SIZE;
    printf("\t fetched value %d\n",val);
    sem_post(&emptyCount);

    /* NOTE: pay attention to where to put the call to printf to be
       sure it can be helpful for debugging */


    return val;
}

void* thread_generating(void* arg)
{
    int i = 0;
    int new_value = 0;

    for(i=0; i<LOOPS; i++){
        new_value = rand()%10;
        pushing(new_value);
    }

    return NULL;
}


void* thread_using(void* arg)
{
    int i = 0;
    int value = 0;

    for(i=0; i<LOOPS; i++){
        value = fetching();
    }

    return NULL;
}



int main(void)
{
    pthread_t tids[2];
    int i=0;
    sem_init(&count,0,0);
    sem_init(&emptyCount,0,BUF_SIZE);
    struct timespec tt;
    clock_gettime(CLOCK_MONOTONIC, &tt);
    /* seed for the random number generator */
    srand(tt.tv_sec);

    if(pthread_create (&tids[0], NULL, thread_using, NULL) != 0){
        fprintf(stderr,"Failed to create the using thread\n");
        return EXIT_FAILURE;
    }

    if(pthread_create (&tids[1], NULL, thread_generating, NULL) != 0){
        fprintf(stderr,"Failed to create the generating thread\n");
        return EXIT_FAILURE;
    }

    /* Wait until every thread ended */
    for (i = 0; i < 2; i++){
        pthread_join (tids[i], NULL) ;
    }

    return EXIT_SUCCESS;
}
