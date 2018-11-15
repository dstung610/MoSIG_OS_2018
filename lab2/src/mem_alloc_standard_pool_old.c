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
    mem_standard_free_block_t *current_free_block = (mem_standard_free_block_t *) pool->first_free;
    mem_standard_block_header_footer_t *temp = NULL;
    mem_standard_free_block_t *flag = current_free_block;
    size_t current_size = 0;
    printf("%p\n", pool->first_free);
    /*Find the block that fit the requested size*/
    do {
      temp = &current_free_block->header;
      current_size = get_block_size(temp);
      printf("Block Size: %lu\n", current_size);
      if (current_size >= size)
      {
        break;
      }
      current_free_block = current_free_block->next;
    } while(current_free_block != flag);

    /*Set the metadata for the header and footer
    of the remaining unused space in the current block if exists*/
    if (current_size - size <= (sizeof(mem_standard_block_header_footer_t) * 2) + (2 * sizeof(mem_standard_free_block_t))) {
      /*Set the metadata for the header and footer of the allocated block*/
      mem_standard_block_header_footer_t *cursor = &current_free_block->header;
      set_block_used(cursor);
      set_block_size(cursor, current_size);

      char *cursor2 = (char*) current_free_block;
      cursor2 = cursor2 + size + sizeof(mem_standard_block_header_footer_t);
      cursor = (mem_standard_block_header_footer_t*) cursor2;
      set_block_used(cursor);
      set_block_size(cursor, current_size);

      /*Update the free list*/
      mem_standard_free_block_t *prev_block = current_free_block->prev;
      mem_standard_free_block_t *next_block = current_free_block->next;
      if (prev_block == next_block) {
        pool->first_free = NULL;
      } else {
        prev_block->next = next_block;
        next_block->prev = prev_block;
        pool->first_free = (void*)current_free_block->next;
      }
    } else {
      size_t old_block_size = current_size;

      mem_standard_block_header_footer_t *cursor = &current_free_block->header;
      set_block_used(cursor);
      set_block_size(cursor, size);

      char *cursor2 = (char*) current_free_block;
      printf("C2:%p\n", cursor2);
      cursor2 = cursor2 + size + sizeof(mem_standard_block_header_footer_t);
      cursor = (mem_standard_block_header_footer_t*) cursor2;
      set_block_used(cursor);
      set_block_size(cursor, size);
      printf("C2:%p\n", cursor2);


      /*Set up a new free block*/
      cursor2 = cursor2 + sizeof(mem_standard_block_header_footer_t);
      mem_standard_free_block_t *new_free_block = (mem_standard_free_block_t *) cursor2;
      mem_standard_block_header_footer_t header;
      printf("C2:%p\n", cursor2);
      new_free_block->header = header;
      mem_standard_block_header_footer_t *temp2 = &header;
      set_block_free(temp2);
      size_t size_of_new_free_block = old_block_size - size - (2*sizeof(mem_standard_block_header_footer_t));
      set_block_size(temp2, size_of_new_free_block);
      printf("New free block size:%d\n", (int)size_of_new_free_block);

      cursor2 = cursor2 + size_of_new_free_block + sizeof(mem_standard_block_header_footer_t);
      printf("C2:%p\n", cursor2);
      temp2 = (mem_standard_block_header_footer_t *) cursor2;
      set_block_free(temp2);
      set_block_size(temp2, size_of_new_free_block);

      size_t sizetest = get_block_size(temp2);
      printf("New Block Size: %d\n", (int)sizetest);

      /*Update the free list*/
      mem_standard_free_block_t *prev_block = current_free_block->prev;
      mem_standard_free_block_t *next_block = current_free_block->next;
      printf("%p\n", prev_block);
      printf("%p\n", current_free_block);
      printf("%p\n", next_block);
      printf("\n");
      printf("%p\n", new_free_block);
      mem_standard_free_block_t* cur_block = (mem_standard_free_block_t*) pool->first_free;
      if (cur_block == cur_block->next)
      {
        printf("Same\n");
        //current_free_block = new_free_block;
        new_free_block->next = new_free_block;
        new_free_block->prev = new_free_block;
      }
      else
      {
        printf("Different\n");
        prev_block->next = new_free_block;
        next_block->prev = new_free_block;
        new_free_block->next = prev_block->next;
        new_free_block->prev = prev_block;
        //current_free_block = new_free_block;
      }
      printf("\n");
      printf("%p\n", new_free_block->prev);
      printf("%p\n", new_free_block);
      printf("%p\n", new_free_block->next);
      pool->first_free = (void*)new_free_block;
      void *ptr = pool->first_free;
      printf("first_free:%p\n", ptr);
      mem_standard_free_block_t *fb = (mem_standard_free_block_t*) ptr;
      mem_standard_block_header_footer_t *hd = &fb->header;
      size_t s = get_block_size(hd);
      printf("Size of first_free: %d\n", (int)s );

    }
    printf("___________________________\n");
    allocated = (void *)current_free_block + sizeof(mem_standard_block_header_footer_t);
    //current_free_block = new_free_block;
    // printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
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
    res = header->flag_and_size;
    // printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
    return res;
}
