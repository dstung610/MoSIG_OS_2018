#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
// #define NB_SONG 3
typedef struct config{
  int team;
  int repeat;
  char* song;
} config_t;

void *supporter (void *arg){
  // char      *str = (char *) arg ;
  config_t* config = (config_t*) arg;
  int n = config->team;
  int repeat = config->repeat;
  char* str = config->song;
  int       i ;
  int       pid ;
  pthread_t tid ;
  pid = getpid () ;
  tid = pthread_self () ;
  srand ((int) tid) ;


  for (i = 0; i < repeat; i++){
      printf ("Process %d Thread %x : %s \n", pid, (unsigned int) tid, str) ;
  }
  return (void *) tid ;
}

int main (int argc, char **argv){

  int team1;
  int team2;
  int repeat1;
  int repeat2;
  int i ;
  int nb_threads = 0 ;
  pthread_t *tids ;

  if (argc != 5){
    fprintf(stderr, "usage : %s team1 repeat1 team2 repeat2\n", argv[0]) ;
    exit (-1) ;
  }

  team1 = atoi (argv[1]);
  repeat1 = atoi (argv[2]);
  team2  = atoi (argv[3]);
  repeat2 = atoi (argv[4]);

  nb_threads = team1 + team2;
  tids = malloc (nb_threads*sizeof(pthread_t)) ;



  char* song1 =  "Allons enfants de la patrie";
  char* song2 =  "Swing low, sweet chariot";

  config_t config1 = {team1, repeat1, song1};
  config_t config2 = {team2, repeat2, song2};

  /* Create the threads for team1 */
  for (i = 0 ; i < team1; i++){
    pthread_create (&tids[i], NULL, supporter, (void*)&config1) ;
  }
  /* Create the other threads (ie. team2) */
  for ( ; i < nb_threads; i++){
    pthread_create (&tids[i], NULL, supporter, (void*)&config2) ;
  }

  /* Wait until every thread ended */
  for (i = 0; i < nb_threads; i++){
    pthread_join (tids[i], NULL) ;
  }

  free (tids) ;
  return EXIT_SUCCESS;
}
