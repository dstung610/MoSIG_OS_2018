#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <dlfcn.h>

#include "mem_alloc.h"
#include "mem_alloc_types.h"
#include "mem_alloc_fast_pool.h"
#include "mem_alloc_standard_pool.h"
#include "my_mmap.h"

/* Number of memory pools managed by the allocator */
#define NB_MEM_POOLS 4

#define ULONG(x)((long unsigned int)(x))


/* Array of memory pool descriptors (indexed by pool id) */
static mem_pool_t mem_pools[NB_MEM_POOLS];

/* Note: the other fields will be setup by the init procedure */
static mem_pool_t fast_pool_1_64 = {
    .pool_id=0,
    .pool_name = "pool-0-fast (1_64)",
    .pool_size = MEM_POOL_0_SIZE,
    .min_request_size = 1,
    .max_request_size = 64,
    .pool_type = FAST_POOL
};

/* Note: the other fields will be setup by the init procedure */
static mem_pool_t fast_pool_65_256 = {
    .pool_id=1,
    .pool_name = "pool-1-fast (65_256)",
    .pool_size = MEM_POOL_1_SIZE,
    .min_request_size = 65,
    .max_request_size = 256,
    .pool_type = FAST_POOL
};

/* Note: the other fields will be setup by the init procedure */
static mem_pool_t fast_pool_257_1024 = {
    .pool_id=2,
    .pool_name = "pool-2-fast (257_1024)",
    .pool_size = MEM_POOL_2_SIZE,
    .min_request_size = 257,
    .max_request_size = 1024,
    .pool_type = FAST_POOL
};

/* Note: the other fields will be setup by the init procedure */
static mem_pool_t standard_pool_1025_and_above = {
    .pool_id=3,
    .pool_name = "pool-3-std (1024_Max)",
    .pool_size = MEM_POOL_3_SIZE,
    .min_request_size = 1025,
    .max_request_size = SIZE_MAX,
    .pool_type = STANDARD_POOL
};

/* This function is automatically called upon the termination of a process. */
void run_at_exit(void)
{
    for (size_t i = 0; i < NB_MEM_POOLS; i++) {
      my_munmap(mem_pools[i].start, mem_pools[i].pool_size);
    }
    fprintf(stderr,"YEAH B-)\n");
    /* You are encouraged to insert more useful code ... */
}

void print_state_array(int flag_arr[], int size){
  for (int i = 0; i < size; ++i)
  {
    if (flag_arr[i] == 1) {
      printf("X");
    } else {
      printf(".");
    }
  }
  printf("\n");
}

void print_fast_pool(mem_pool_t mem){
  size_t pool_size = mem.pool_size;
  printf("Memory State of Pool %d:\n", mem.pool_id);
  int flag_arr[pool_size];
  for (int i = 0; i < pool_size; i++)
  {
    flag_arr[i] = 1;
  }
  void* current_free = mem.first_free;
  // void* anchor = mem.first_free;
  void* start_pool = mem.start;
  void* end_pool = mem.end;
  int current_free_index = 0;
  if (current_free != NULL) {
    do {
      current_free_index = current_free - start_pool;
      for (int i = current_free_index; i < current_free_index + mem.max_request_size; i++) {
        flag_arr[i] = 0;
      }
      mem_fast_free_block_t *current_free_block = (mem_fast_free_block_t*)current_free;
      mem_fast_free_block_t *next_free_block = current_free_block->next;
      current_free = (void *) next_free_block;
    } while(current_free != end_pool);
  }
  print_state_array(flag_arr, pool_size);
}

void print_header_footer(){
  for (size_t i = 0; i < sizeof(mem_standard_block_header_footer_t); i++) {
    printf("X");
  }
}

void print_standard_pool(mem_pool_t mem){
  printf("Memory State of Pool %d:\n", mem.pool_id);
  void* start_pool = mem.start;
  void* end_pool = mem.end;
  char* cursor = (char*)start_pool;
  mem_standard_block_header_footer_t *current_header = NULL;
  int current_block_size = 0;
  while ((void*)cursor - end_pool < 0) {
      current_header = (mem_standard_block_header_footer_t*)cursor;
      current_block_size = get_block_size(current_header);
      printf("[");
      print_header_footer();
      printf("|");
      if (is_block_free(current_header)) {
        for (int i = 0; i < current_block_size; i++) {
          printf(".");
        }
      } else {
        for (int i = 0; i < current_block_size; i++) {
          printf("X");
        }
      }
      printf("|");
      print_header_footer();
      printf("]");
      cursor = cursor + current_block_size + (sizeof(mem_standard_block_header_footer_t) * 2);
  }
  printf("\n");
}

void print_mem_state(void)
{
    print_fast_pool(mem_pools[0]);
    print_fast_pool(mem_pools[1]);
    print_fast_pool(mem_pools[2]);
    print_standard_pool(mem_pools[3]);
    // printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
}

void debug(){
  printf("----------DEBUG---------\n");
  mem_standard_free_block_t *first_free = (mem_standard_free_block_t*) mem_pools[3].first_free;
  printf("Free list:\n");
  do {
    printf("prev:%p\n", first_free->prev);
    printf("curr:%p\n", first_free);
    printf("next:%p\n", first_free->next);
    printf("|\n");
    first_free = first_free->next;
  } while(first_free != NULL);
  printf("\n");
  printf("------------------------\n");

}


/*
 * Returns the id of the pool in charge of a given block size.
 * Note:
 * We assume that the contents of the pools are consistent
 * (pools covering all possible block sizes,
 * sorted by increasing sizes,
 * no overlap).
 */
 static int find_pool_from_block_size(size_t size) {
    int i;
    int res = -1;
    debug_printf("size=%lu\n", size);
    for (i = 0; i < NB_MEM_POOLS; i++) {
        if ((size >= mem_pools[i].min_request_size) && (size <= mem_pools[i].max_request_size)) {
            res = i;
            break;
        }
    }
    debug_printf("will return %d\n", res);
    assert(res >= 0);
    return res;
}

/*
 * Returns the id of the pool in charge of a given block.
 */
int find_pool_from_block_address(void *addr) {
    int i;
    int res = -1;
    debug_printf("enter - addr = %p\n", addr);
    for (i = 0; i < NB_MEM_POOLS; i++) {
        if ((addr >= mem_pools[i].start) && (addr <= mem_pools[i].end)) {
            res = i;
            break;
        }
    }
    debug_printf("will return %d\n", res);
    return res;
}

void memory_init(void){
    int i;

    /* Register the function that will be called when the process exits. */
    atexit(run_at_exit);

    /* Init all the pools */
    mem_pools[0] = fast_pool_1_64;
    mem_pools[1] = fast_pool_65_256;
    mem_pools[2] = fast_pool_257_1024;
    mem_pools[3] = standard_pool_1025_and_above;


    for (i = 0; i < 3; i++) {
        init_fast_pool(&(mem_pools[i]), mem_pools[i].pool_size, mem_pools[i].min_request_size, mem_pools[i].max_request_size);
    }
    init_standard_pool(&(mem_pools[3]), mem_pools[3].pool_size, mem_pools[3].min_request_size, mem_pools[3].max_request_size);

    /* checks that the pools request sizes are not overlapping */
    assert(mem_pools[0].min_request_size == 1);
    for (i = 0; i < NB_MEM_POOLS - 1; i++) {
        assert(mem_pools[i].max_request_size + 1 == mem_pools[i+1].min_request_size);
        debug_printf("mem_pools[%d]: size=%lu, min_request_size=%lu, max_request_size=%lu\n", i, mem_pools[i].pool_size, mem_pools[i].min_request_size, mem_pools[i].max_request_size);
    }
    debug_printf("mem_pools[%d]: size=%lu, min_request_size=%lu, max_request_size=%lu\n", i, mem_pools[i].pool_size, mem_pools[i].min_request_size, mem_pools[i].max_request_size);
    assert(mem_pools[NB_MEM_POOLS - 1].max_request_size == SIZE_MAX);

}

/*
 * Entry point for allocation requests.
 * Forwards the request to the appopriate pool.
 */
void *memory_alloc(size_t size){
    int i;
    void *alloc_addr=NULL;
    i = find_pool_from_block_size(size);
    switch (mem_pools[i].pool_type) {
        case FAST_POOL:
            alloc_addr = mem_alloc_fast_pool(&(mem_pools[i]), size);
            break;
        case STANDARD_POOL:
            alloc_addr = mem_alloc_standard_pool(&(mem_pools[i]), size);
            break;
        default: /* we should never reach this case */
            assert(0);
    }
    if (alloc_addr == NULL) {
        print_alloc_error(size);
        exit(0);
    } else {
        print_alloc_info(alloc_addr, size);
    }
    return alloc_addr;
}

/*
 * Entry point for deallocation requests.
 * Forwards the request to the appopriate pool.
 */
void memory_free(void *p){
    int i;
    i = find_pool_from_block_address(p);
    switch (mem_pools[i].pool_type) {
        case FAST_POOL:
            mem_free_fast_pool(&(mem_pools[i]), p);
            break;
        case STANDARD_POOL:
            mem_free_standard_pool(&(mem_pools[i]), p);
            break;
        default: /* we should never reach this case */
            assert(0);
    }
    print_free_info(p);
}


/* Returns the payload size of an allocated block */
size_t memory_get_allocated_block_size(void *addr) {
    int i;
    size_t res;
    i = find_pool_from_block_address(addr);
    assert(i >= 0);
    switch (mem_pools[i].pool_type) {
        case FAST_POOL:
            res = mem_get_allocated_block_size_fast_pool(&(mem_pools[i]), addr);
            break;
        case STANDARD_POOL:
            res = mem_get_allocated_block_size_standard_pool(&(mem_pools[i]), addr);
            break;
        default: /* we should never reach this case */
            assert(0);
    }
    return res;
}


void print_free_info(void *addr){
    if(addr){
        int i;
        i = find_pool_from_block_address(addr);

        fprintf(stderr, "FREE  at : %lu -- pool %d\n", ULONG(((char*)(addr)) - ((char*)(mem_pools[i].start))), mem_pools[i].pool_id);
    }
    else{
        fprintf(stderr, "FREE  at : %lu \n", ULONG(0));
    }
}

void print_alloc_info(void *addr, int size)
{
    if(addr){
        int i;
        i = find_pool_from_block_address(addr);

        fprintf(stderr, "ALLOC at : %lu (%d byte(s)) -- pool %d\n",
                ULONG(((char*)addr) - ((char*)(mem_pools[i].start))), size, mem_pools[i].pool_id);
    }
    else{
        print_alloc_error(size);
    }
}



void print_alloc_error(int size)
{
    fprintf(stderr, "ALLOC error : We can't allocate %d bytes\n", size);
}



#ifdef MAIN
/* The main function can be changed (to perform simple tests).
 * It is NOT involved in the automated tests
 */
int main(int argc, char **argv){

  memory_init();

  int i ;
  for( i = 0; i < 10; i++){
    char *b = memory_alloc(rand()%8);
    memory_free(b);
  }

  char * a = memory_alloc(15);
  memory_free(a);


  a = memory_alloc(10);
  memory_free(a);

  fprintf(stderr,"%lu\n",(long unsigned int) (memory_alloc(9)));
  return EXIT_SUCCESS;
}
#endif /* MAIN */
