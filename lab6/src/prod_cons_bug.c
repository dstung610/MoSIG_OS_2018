#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define LOOPS 20
#define BUF_SIZE 2
#define NTHREADS 4

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

int buffer[BUF_SIZE];

int buf_first=0;
int buf_last=0;
int buf_count=0;


void produce(int val)
{
    pthread_mutex_lock(&mutex);
    while(buf_count == BUF_SIZE){
        pthread_cond_wait(&cond1, &mutex);
    }

    buffer[buf_first] = val;
    
    buf_count++;
    buf_first = (buf_first + 1) % BUF_SIZE;

    printf("\t producing value %d\n",val);

    pthread_cond_broadcast(&cond2);
    pthread_mutex_unlock(&mutex);
}

int consume(void)
{
    int val = -1;

    pthread_mutex_lock(&mutex);
    while(buf_count == 0){
        pthread_cond_wait(&cond2, &mutex);
    }

    val = buffer[buf_last];
    buf_count--;
    buf_last = (buf_last + 1) % BUF_SIZE;

    printf("\t\t consuming value %d\n",val);
       
    pthread_cond_broadcast(&cond1);
    pthread_mutex_unlock(&mutex);
    
    return val;
}

void* producer(void* arg)
{
    int i = 0;
    int new_value = 0;
    
    for(i=0; i<LOOPS; i++){
        new_value = rand()%10;
        produce(new_value);
    }

    return NULL;
}


void* consumer(void* arg)
{
    int i = 0;
    int value = 0;
    
    for(i=0; i<LOOPS; i++){
        value = consume();
    }

    return NULL;
}



int main(void)
{
    pthread_t tids[NTHREADS];
    int i=0;
    
    struct timespec tt;
    clock_gettime(CLOCK_MONOTONIC, &tt);
    /* seed for the random number generator */
    srand(tt.tv_sec);

    for(i=0; i< NTHREADS; i++){
        if(i%2 == 0){
            if(pthread_create (&tids[i], NULL, producer, NULL) != 0){
                fprintf(stderr,"Failed to create the using thread\n");
                return EXIT_FAILURE;
            }
        }
        else{
            if(pthread_create (&tids[i], NULL, consumer, NULL) != 0){
                fprintf(stderr,"Failed to create the generating thread\n");
                return EXIT_FAILURE;
            }
        }
    }
    
    /* Wait until every thread ended */ 
    for (i = 0; i < NTHREADS; i++){
        pthread_join (tids[i], NULL) ;
    }  
    
    return EXIT_SUCCESS;
}
