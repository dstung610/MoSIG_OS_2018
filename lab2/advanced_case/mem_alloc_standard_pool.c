#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <math.h>

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
    first_free_block->next = NULL;
    first_free_block->prev = NULL;
    p->first_free = (void *) first_free_block;
    p->previous_free = p->first_free;
    // printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
}


void *mem_alloc_standard_pool(mem_pool_t *pool, size_t size_origin) {
    /* TO BE IMPLEMENTED */
    void *allocated;
    size_t size = new_block_size_with_alignment(size_origin);
    mem_standard_free_block_t *first_free = (mem_standard_free_block_t*) pool->first_free;
    mem_standard_free_block_t *loop_cursor = NULL;
    mem_standard_block_header_footer_t *loop_header = NULL;
    size_t size_current_free_block = 0;
    int have_space = 0;

    if (std_pool_policy==FIRST_FIT) {
      loop_cursor = (mem_standard_free_block_t*) pool->first_free;
      do {
        loop_header = &loop_cursor->header;
        size_current_free_block = get_block_size(loop_header);
        if (size_current_free_block >= size) {
          have_space = 1;
          break;
        } else {
          loop_cursor = loop_cursor->next;
        }
      } while(loop_cursor != NULL);
    }

    if (std_pool_policy==NEXT_FIT) {
      int is_turn_over = 0;
      loop_cursor = (mem_standard_free_block_t*) pool->previous_free;
      do {
        loop_header = &loop_cursor->header;
        size_current_free_block = get_block_size(loop_header);
        if (size_current_free_block >= size) {
          have_space = 1;
          break;
        } else {
          loop_cursor = loop_cursor->next;
        }
        if (loop_cursor == NULL) {
          is_turn_over = 1;
          loop_cursor = (mem_standard_free_block_t*) pool->first_free;
        }
      } while(loop_cursor != (mem_standard_free_block_t*) pool->previous_free  && !is_turn_over);
    }

    mem_standard_used_block_t *allocated_block = (mem_standard_used_block_t*) loop_cursor;
    mem_standard_block_header_footer_t *header_allocated = &allocated_block->header;
    mem_standard_block_header_footer_t *footer_allocated = NULL;
    mem_standard_free_block_t *new_free_block = NULL;
    mem_standard_block_header_footer_t *header_new_free_block = NULL;
    mem_standard_block_header_footer_t *footer_new_free_block = NULL;
    char *mem_cursor = NULL;

    if (have_space) {
      size_t new_free_block_size = size_current_free_block - size -(2 * sizeof(mem_standard_block_header_footer_t));
      if ((int)new_free_block_size > 0){
        /*Case where the remaining can create a free block*/
        // Setup for allocated block footer
        debug_printf("Case 1\n");
        set_block_used(header_allocated);
        set_block_size(header_allocated, size);
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
          new_free_block->next = NULL;
          new_free_block->prev = NULL;
        } else {
          new_free_block->prev = loop_cursor->prev;
          new_free_block->next = loop_cursor->next;
          mem_standard_free_block_t *prev = loop_cursor->prev;
          mem_standard_free_block_t *next = loop_cursor->next;
          if (prev != NULL) {
            prev->next = new_free_block;
          }
          if (next != NULL) {
            next->prev = new_free_block;
          }
        }

        if ((char*)loop_cursor - (char*)first_free <= 0) {
          pool->first_free = (void*)new_free_block;
        }

        if ((char*)new_free_block - (char*)first_free <= 0) {
          pool->first_free = (void*)new_free_block;
        }

        if (std_pool_policy==NEXT_FIT) {
          pool->previous_free = (void*) new_free_block;
        }

      } else {
        /*Case where the remaining is too small and give them all to user*/
        debug_printf("Case 2\n");
        set_block_used(header_allocated);
        set_block_size(header_allocated, size_current_free_block);

        if (loop_cursor->next == loop_cursor->prev) {
        } else {
          mem_standard_free_block_t *prev = loop_cursor->prev;
          mem_standard_free_block_t *next = loop_cursor->next;
          if (prev != NULL) {
            prev->next = next;
          }
          if (next != NULL) {
            next->prev = prev;
          }
        }

        mem_cursor = (char*) allocated_block;
        mem_cursor = mem_cursor + sizeof(mem_standard_block_header_footer_t) + size_current_free_block;
        footer_allocated = (mem_standard_block_header_footer_t*) mem_cursor;
        set_block_used(footer_allocated);
        set_block_size(footer_allocated, size_current_free_block);

        if (loop_cursor->next == loop_cursor->prev) {
          pool->first_free = NULL;
        } else {
          if (loop_cursor->next != NULL) {
            pool->first_free = loop_cursor->next;
          } else {
            pool->first_free = loop_cursor->prev;
          }
        }

        if (std_pool_policy==NEXT_FIT) {
          if (loop_cursor->next == NULL) {
            pool->previous_free = pool->first_free;
          }
        }
      }
    } else {
      allocated = NULL;
    }


    allocated = (void*)allocated_block + sizeof(mem_standard_block_header_footer_t);
    debug_printf("%lu\n", sizeof(mem_standard_block_header_footer_t));

    return allocated;
}


void mem_free_standard_pool(mem_pool_t *pool, void *addr) {
    /* TO BE IMPLEMENTED */
    char *mem_cursor = (char*) addr;
    mem_cursor = mem_cursor - sizeof(mem_standard_block_header_footer_t);
    mem_standard_used_block_t *about_to_free_block = (mem_standard_used_block_t*) mem_cursor;
    mem_standard_block_header_footer_t *header_about_to_free_block = &about_to_free_block->header;
    mem_standard_block_header_footer_t *footer_about_to_free_block = NULL;
    size_t about_to_free_block_size = get_block_size(header_about_to_free_block);
    int left_block_is_free = 0;
    int right_block_is_free = 0;
    mem_cursor = mem_cursor + sizeof(mem_standard_block_header_footer_t) + about_to_free_block_size;
    footer_about_to_free_block = (mem_standard_block_header_footer_t*) mem_cursor;

    mem_standard_block_header_footer_t *header_footer_temp = NULL;

    mem_cursor = (char*)header_about_to_free_block;
    mem_cursor = mem_cursor - sizeof(mem_standard_block_header_footer_t);
    header_footer_temp = (mem_standard_block_header_footer_t*)mem_cursor;
    if (mem_cursor - (char*)pool->start > 0 && is_block_free(header_footer_temp)) {
      left_block_is_free = 1;
    }

    mem_cursor = (char*)footer_about_to_free_block;
    mem_cursor = mem_cursor + sizeof(mem_standard_block_header_footer_t);
    header_footer_temp = (mem_standard_block_header_footer_t*)mem_cursor;
    if ((char*)pool->end - mem_cursor > 0 && is_block_free(header_footer_temp)) {
      right_block_is_free = 1;
    }

    debug_printf("Left: %d - Right: %d\n", left_block_is_free, right_block_is_free);

    mem_standard_free_block_t *new_free_block = NULL;
    size_t new_free_block_size = 0;
    if (left_block_is_free && right_block_is_free) {
      debug_printf("Case 1\n");
      mem_cursor = (char*)header_about_to_free_block;
      mem_cursor = mem_cursor - sizeof(mem_standard_block_header_footer_t);
      header_footer_temp = (mem_standard_block_header_footer_t*) mem_cursor;
      size_t left_block_size = get_block_size(header_footer_temp);

      mem_cursor = (char*)footer_about_to_free_block;
      mem_cursor = mem_cursor + sizeof(mem_standard_block_header_footer_t);
      header_footer_temp = (mem_standard_block_header_footer_t*) mem_cursor;
      mem_standard_free_block_t *right_free_block = (mem_standard_free_block_t*) mem_cursor;
      size_t right_block_size = get_block_size(header_footer_temp);

      new_free_block_size = left_block_size + right_block_size + (4*sizeof(mem_standard_block_header_footer_t));

      mem_cursor = (char*)header_about_to_free_block;
      mem_cursor = mem_cursor - sizeof(mem_standard_block_header_footer_t);
      mem_cursor = mem_cursor - left_block_size - sizeof(mem_standard_block_header_footer_t);
      header_footer_temp = (mem_standard_block_header_footer_t*) mem_cursor;
      set_block_size(header_footer_temp, new_free_block_size);
      mem_standard_free_block_t *left_free_block = (mem_standard_free_block_t*) mem_cursor;
      left_free_block->next = right_free_block->next;

      mem_cursor = mem_cursor + sizeof(mem_standard_block_header_footer_t) + new_free_block_size;
      header_footer_temp = (mem_standard_block_header_footer_t*) mem_cursor;
      set_block_size(header_footer_temp, new_free_block_size);

      new_free_block = left_free_block;
      if ((void*)left_free_block == pool->previous_free || (void*)right_free_block == pool->previous_free) {
        pool->previous_free = (void*) left_free_block;
      }
    }

    if (left_block_is_free || right_block_is_free) {
      if (left_block_is_free) {
        debug_printf("Case 2\n");
        mem_cursor = (char*) header_about_to_free_block;
        mem_cursor = mem_cursor - sizeof(mem_standard_block_header_footer_t);
        header_footer_temp = (mem_standard_block_header_footer_t*) mem_cursor;
        size_t left_block_size = get_block_size(header_footer_temp);
        new_free_block_size = left_block_size + (2*sizeof(mem_standard_block_header_footer_t)) + about_to_free_block_size;

        mem_cursor = mem_cursor - left_block_size - sizeof(mem_standard_block_header_footer_t);
        header_footer_temp = (mem_standard_block_header_footer_t*) mem_cursor;
        set_block_size(header_footer_temp, new_free_block_size);

        set_block_free(footer_about_to_free_block);
        set_block_size(footer_about_to_free_block, new_free_block_size);

        new_free_block = (mem_standard_free_block_t*) mem_cursor;
      } else {
        debug_printf("Case 3\n");
        mem_cursor = (char*) footer_about_to_free_block;
        mem_cursor = mem_cursor + sizeof(mem_standard_block_header_footer_t);
        header_footer_temp = (mem_standard_block_header_footer_t*) mem_cursor;
        size_t right_block_size = get_block_size(header_footer_temp);
        // printf("%lu\n", right_block_size);
        new_free_block_size = right_block_size + (2*sizeof(mem_standard_block_header_footer_t)) + about_to_free_block_size;
        mem_standard_free_block_t* right_free_block = (mem_standard_free_block_t*) mem_cursor;
        new_free_block = (mem_standard_free_block_t*) about_to_free_block;
        new_free_block->next = right_free_block->next;
        new_free_block->prev = right_free_block->prev;

        mem_standard_free_block_t *prev = right_free_block->prev;
        mem_standard_free_block_t *next = right_free_block->next;
        if (prev != NULL) {
          prev->next = new_free_block;
        }
        if (next != NULL) {
          next->prev = new_free_block;
        }

        mem_cursor = mem_cursor + right_block_size + sizeof(mem_standard_block_header_footer_t);
        header_footer_temp = (mem_standard_block_header_footer_t*) mem_cursor;
        set_block_size(header_footer_temp, new_free_block_size);
        set_block_free(header_about_to_free_block);
        set_block_size(header_about_to_free_block, new_free_block_size);

        if ((void*)right_free_block == pool->previous_free) {
          pool->previous_free = (void*) new_free_block;
        }
      }

    }

    if (!left_block_is_free && !right_block_is_free) {
      debug_printf("Case 4\n");
      mem_standard_free_block_t *loop_current_free = (mem_standard_free_block_t*) pool->first_free;
      mem_standard_free_block_t *closest = loop_current_free;
      size_t min_distance = abs((char*)loop_current_free - (char*)about_to_free_block);
      do {
        if ((char*)loop_current_free - (char*)about_to_free_block <= min_distance) {
          closest = loop_current_free;
        }
        loop_current_free = loop_current_free->next;
      } while(loop_current_free != NULL);

      mem_standard_free_block_t *new_free = (mem_standard_free_block_t*) about_to_free_block;
      mem_standard_free_block_t *prev = closest->prev;
      mem_standard_free_block_t *next = closest->next;
      int diff = (int)((char*)new_free-(char*)closest);
      if (diff < 0) {
        new_free->next = closest;
        new_free->prev = closest->prev;
        closest->prev = new_free;
        if (prev != NULL) {
          prev->next = new_free;
        }
      } else {
        new_free->next = closest->next;
        new_free->prev = closest;
        closest->next = new_free;
        if (next != NULL) {
          next->prev = new_free;
        }
      }

      set_block_free(header_about_to_free_block);
      set_block_size(header_about_to_free_block, about_to_free_block_size);

      set_block_free(footer_about_to_free_block);
      set_block_size(footer_about_to_free_block, about_to_free_block_size);

      new_free_block = (mem_standard_free_block_t*) about_to_free_block;
    }

    if ((char*)new_free_block - (char*) pool->first_free < 0) {
      pool->first_free = (void*) new_free_block;
    }
    // printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
}

size_t mem_get_allocated_block_size_standard_pool(mem_pool_t *pool, void *addr) {
    /* TO BE IMPLEMENTED */
    size_t res;
    mem_standard_block_header_footer_t *header = (mem_standard_block_header_footer_t*) addr;
    res = get_block_size(header);
    // printf("%s:%d: Please, implement me!\n", __FUNCTION__, __LINE__);
    return res;
}

size_t new_block_size_with_alignment(size_t size){
    int res = size % MEM_ALIGNMENT;
    if (res == 0)
      return size;
    else
      return size + (MEM_ALIGNMENT - res);
}
