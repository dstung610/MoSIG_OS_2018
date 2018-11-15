#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "mem_alloc_types.h"
#include "mem_alloc_standard_pool.h"
#include "my_mmap.h"
#include "mem_alloc.h"


/////////////////////////////////////////////////////////////////////////////

#ifdef STDPOOL_POLICY
    /* Get the value provided by the Makefile */
    std_pool_placement_policy_t std_pool_policy = STDPOOL_POLICY;
#else
    std_pool_placement_policy_t std_pool_policy = DEFAULT_STDPOOL_POLICY;
#endif

/////////////////////////////////////////////////////////////////////////////


void init_standard_pool(mem_pool_t *p, size_t size, size_t min_request_size, size_t max_request_size) {
    /* TO BE IMPLEMENTED */
    void *heap_pointer = my_mmap(size);
    p->start = heap_pointer;
    p->end = heap_pointer + size;
    p->pool_size = size;
    mem_standard_free_block_t *first_free_block = (mem_standard_free_block_t *) heap_pointer;
    mem_standard_block_header_footer_t header;

    mem_standard_block_header_footer_t *cursor = &header;
    first_free_block->header = header;
    set_block_free(cursor);
    set_block_size(cursor, size - (2*sizeof(mem_standard_block_header_footer_t)));
    char *cursor2 = (char*) heap_pointer;
    cursor2 = cursor2 + size - sizeof(mem_standard_block_header_footer_t);
    cursor = (mem_standard_block_header_footer_t*) cursor2;
    set_block_free(cursor);
    set_block_size(cursor, size - (2*sizeof(mem_standard_block_header_footer_t)));

    first_free_block->header = header;
    first_free_block->next = first_free_block;
    first_free_block->prev = first_free_block;
    p->first_free = (void *) first_free_block;
    // printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
}


void *mem_alloc_standard_pool(mem_pool_t *pool, size_t size) {
    /* TO BE IMPLEMENTED */
    void *allocated;
    mem_standard_free_block_t *first_free = (mem_standard_free_block_t*) pool->first_free;
    mem_standard_free_block_t *loop_cursor = (mem_standard_free_block_t*) pool->first_free;
    mem_standard_block_header_footer_t *loop_header = NULL;
    size_t size_current_free_block = 0;
    int have_space = 0;
    do {
      loop_header = &loop_cursor->header;
      size_current_free_block = get_block_size(loop_header);
      if (size_current_free_block >= size) {
        have_space = 1;
        break;
      } else {
        loop_cursor = loop_cursor->next;
      }
    } while(loop_cursor != first_free);

    mem_standard_used_block_t *allocated_block = (mem_standard_used_block_t*) loop_cursor;
    mem_standard_block_header_footer_t *header_allocated = &allocated_block->header;
    mem_standard_block_header_footer_t *footer_allocated = NULL;
    mem_standard_free_block_t *new_free_block = NULL;
    mem_standard_block_header_footer_t *header_new_free_block = NULL;
    mem_standard_block_header_footer_t *footer_new_free_block = NULL;
    char *mem_cursor = NULL;

    if (have_space) {
      set_block_used(header_allocated);
      set_block_size(header_allocated, size);
      size_t new_free_block_size = size_current_free_block - size -(2 * sizeof(mem_standard_block_header_footer_t));
      if (new_free_block_size > 0){
        /*Case where the remaining can create a free block*/
        // Setup for allocated block footer
        mem_cursor = (char*) allocated_block;
        mem_cursor = mem_cursor + sizeof(mem_standard_block_header_footer_t) + size;
        footer_allocated = (mem_standard_block_header_footer_t*) mem_cursor;
        set_block_used(footer_allocated);
        set_block_size(footer_allocated, size);

        // Setup for new free block
        mem_cursor = mem_cursor + sizeof(mem_standard_block_header_footer_t);
        new_free_block = (mem_standard_free_block_t*) mem_cursor;
        header_new_free_block = &new_free_block->header;
        set_block_free(header_new_free_block);
        set_block_size(header_new_free_block, new_free_block_size);

        mem_cursor = mem_cursor + sizeof(mem_standard_block_header_footer_t) + new_free_block_size;
        footer_new_free_block = (mem_standard_block_header_footer_t*) mem_cursor;
        set_block_free(footer_new_free_block);
        set_block_size(footer_new_free_block, new_free_block_size);

        // Setup pointer for free list
        if (loop_cursor->next == loop_cursor->prev) {
          new_free_block->next = new_free_block;
          new_free_block->prev = new_free_block;
        } else {
          new_free_block->prev = loop_cursor->prev;
          new_free_block->next = loop_cursor->next;
          mem_standard_free_block_t *prev = loop_cursor->prev;
          mem_standard_free_block_t *next = loop_cursor->next;
          prev->next = new_free_block;
          next->prev = new_free_block;
        }
        pool->first_free = new_free_block;
        allocated = (void*)allocated_block + sizeof(mem_standard_block_header_footer_t);

      } else {
        /*Case where the remaining is too small and give them all to user*/
      }
    } else {
      allocated = NULL;
    }

    return allocated;
}


void mem_free_standard_pool(mem_pool_t *pool, void *addr) {
    /* TO BE IMPLEMENTED */

    printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
}

size_t mem_get_allocated_block_size_standard_pool(mem_pool_t *pool, void *addr) {
    /* TO BE IMPLEMENTED */
    size_t res;
    mem_standard_block_header_footer_t *header = (mem_standard_block_header_footer_t*) addr;
    res = get_block_size(header);
    // printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
    return res;
}
