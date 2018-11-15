#include <assert.h>
#include <stdio.h>

#include "mem_alloc_fast_pool.h"
#include "my_mmap.h"
#include "mem_alloc.h"


void init_fast_pool(mem_pool_t *p, size_t size, size_t min_request_size, size_t max_request_size) {
    /* TO BE IMPLEMENTED */
    void *heap_pointer = my_mmap(size);
    p->start = heap_pointer;
    p->end = heap_pointer + size;
    p->pool_size = size;
    mem_fast_free_block_t *first_block = (mem_fast_free_block_t *) heap_pointer;
    mem_fast_free_block_t *current_metadata = first_block;
    mem_fast_free_block_t *new_metadata = first_block;
    char *cur_pointer = (char*)first_block;
    for (int i=0; i<(int)(size/max_request_size); ++i)
    {
      cur_pointer = cur_pointer + (int)(max_request_size);
      new_metadata = (mem_fast_free_block_t *)cur_pointer;
      current_metadata->next = new_metadata;
      current_metadata = new_metadata;
    }
    p->first_free = (void *) first_block;
    // printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
}


void *mem_alloc_fast_pool(mem_pool_t *pool, size_t size) {
    /* TO BE IMPLEMENTED */
    void *mem_allocated;
    mem_allocated = pool->first_free;
    mem_fast_free_block_t *old_free_block = (mem_fast_free_block_t*) mem_allocated;
    mem_fast_free_block_t *new_free_block = old_free_block->next ;
    pool->first_free = (void *) new_free_block;
    // printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
    return mem_allocated;
}

void mem_free_fast_pool(mem_pool_t *pool, void *b) {
    /* TO BE IMPLEMENTED */
    void * old_first_free;
    old_first_free = pool->first_free;
    mem_fast_free_block_t *new_free_block = (mem_fast_free_block_t*) b;
    new_free_block->next = (mem_fast_free_block_t *)old_first_free;
    pool->first_free = (void *) new_free_block;
    // printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
}

size_t mem_get_allocated_block_size_fast_pool(mem_pool_t *pool, void *addr) {
    size_t res;
    res = pool->max_request_size;
    return res;
}
