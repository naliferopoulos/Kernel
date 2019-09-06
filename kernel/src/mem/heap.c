#include <mem/heap.h>
#include <mem/pmm.h>
#include <mem/vmm.h>
#include <libk/stdbool.h>
#include <libk/dbg.h>
#include <task/spinlock.h>

uint32_t* heapPos = (uint32_t*)HEAP_POS;

static spinlock_t heap_lock = {.lock = 0};

static void* heap_base_ptr=NULL;

typedef struct blck_metadata_t {
	size_t size;
	struct blck_metadata_t* next;
	struct blck_metadata_t* prev;
	bool is_free;
	void* validation_ptr;
	char blck_metadata_t_end[0]; // http://gcc.gnu.org/onlinedocs/gcc/Zero-Length.html
} blck_metadata_t;

#define blck_metadata_t_SIZE sizeof(blck_metadata_t)

static inline size_t align_long(size_t size) 
{
	if (size & (sizeof(long) - 1)) 
	{
		size += sizeof(long) - (size & (sizeof(long) - 1));
	}

	return size;
}

static inline bool is_suitable_block(blck_metadata_t* block, size_t size) 
{
	return block->is_free && block->size >= size;
}

static blck_metadata_t* get_free_block(blck_metadata_t** last, size_t size) 
{
	blck_metadata_t* block_runner = (blck_metadata_t*)heap_base_ptr;
	
	while (block_runner && !(is_suitable_block(block_runner, size))) 
	{
		*last = block_runner;
		block_runner = block_runner->next;
	}

	return block_runner;
}

static blck_metadata_t* extend_heap(blck_metadata_t* last, size_t size) 
{
	blck_metadata_t* block = (blck_metadata_t*) heapPos;

	for (size_t sz = 0; sz < size; sz += PAGE_SIZE)
	{
		// Fetch a physical block.
		void* p = pmm_alloc_block ();
		if (!p)
			return 0;

		// Map it to the heap.
		vmmngr_map_page(p, (void *)heapPos);

		// Increment the heap pointer.
		heapPos += PAGE_SIZE;
	}

	block->size = size;
	block->next = NULL;
	block->prev = last;
	block->validation_ptr = block->blck_metadata_t_end;

	if (last != NULL) 
	{
		last->next = block;
	}

	block->is_free = false;

	// If we can split the block, then split it.
	if(blck_metadata_t_SIZE * 2 + size < PAGE_SIZE)
	{
		blck_metadata_t* split_block = (blck_metadata_t*) (block->blck_metadata_t_end + size);
		split_block->size = PAGE_SIZE - (blck_metadata_t_SIZE * 2) - size;
		split_block->next = NULL;
		split_block->prev = block;
		split_block->validation_ptr = split_block->blck_metadata_t_end;
		split_block->is_free = true;
		block->next = split_block;
	}

	return block;
}

static void split_block(blck_metadata_t* block, size_t size) 
{
	blck_metadata_t* new_block = (blck_metadata_t*) (block->blck_metadata_t_end + size); // important to have the blck_metadata_t_end as char[0] for the pointer arithmetic
	new_block->size = block->size - size - blck_metadata_t_SIZE;
	new_block->next = block->next;
	new_block->prev = block;
	new_block->is_free = true;
	new_block->validation_ptr = new_block->blck_metadata_t_end;
	block->size = size;
	block->next = new_block;

	if (new_block->next) 
	{
		new_block->next->prev = new_block;
	}
}

void* kmalloc(size_t size) 
{
	acquire_spinlock(&heap_lock);

	size_t aligned_size = align_long(size);
	blck_metadata_t* block_to_return = NULL;

	if (!heap_base_ptr) 
	{ // first allocation
    	block_to_return = extend_heap(NULL, aligned_size);
    	if (!block_to_return) 
    	{
      		return NULL;
    	}
    
    	heap_base_ptr = block_to_return;
  	} 
  	else 
  	{
    	blck_metadata_t* last_visited_block = (blck_metadata_t*)heap_base_ptr;
    	block_to_return = get_free_block(&last_visited_block, aligned_size);
    	if (block_to_return) 
    	{ 
      		if (block_to_return->size - aligned_size >= blck_metadata_t_SIZE + sizeof(long)) 
      		{
        		split_block(block_to_return, aligned_size);
      		}
      
      		block_to_return->is_free = false;
    	} 
    	else 
    	{ // No free blocks, we must map more memory
      		block_to_return = extend_heap(last_visited_block, aligned_size);
      		if (!block_to_return) {
        		return NULL;
      		}			
    	}
  	}

  release_spinlock(&heap_lock);

  return block_to_return->blck_metadata_t_end;
}

static blck_metadata_t* merge_block_with_next(blck_metadata_t* block) 
{
	if (block->next && block->next->is_free) 
	{
		block->size += blck_metadata_t_SIZE + block->next->size;
		block->next = block->next->next;

		if (block->next) 
		{
		  block->next->prev = block;
		}
	}

	return block;
}

static blck_metadata_t* get_blck_metadata_t_from_ptr(void* ptr) 
{
	char* aux = (char*)ptr; 
	return (blck_metadata_t*)(aux - blck_metadata_t_SIZE);
}

static bool is_valid_ptr(void* ptr) 
{
  if (heap_base_ptr) 
  {
	if (ptr > heap_base_ptr && ptr < (void *)heapPos) 
	{ 
		// heap range check
		return (ptr == get_blck_metadata_t_from_ptr(ptr)->validation_ptr); 
	}
  }

  return false;
}

void kfree(void* ptr) 
{
	acquire_spinlock(&heap_lock);

	ASSERT(is_valid_ptr(ptr));

	blck_metadata_t* block = get_blck_metadata_t_from_ptr(ptr);
	
	ASSERT(!block->is_free);

	block->is_free = true;

	if (block->prev && block->prev->is_free) 
	{
		block = merge_block_with_next(block->prev);
	}

	if (block->next) 
	{
		block = merge_block_with_next(block);
	} 
	else 
	{ // last block of the heap
		if (block->prev) 
		{ // There are more blocks to the left
      		block->prev->next = NULL;
    	} 
    	else 
    	{
      		heap_base_ptr = NULL;
    	}

    	heapPos = (void *)block;
  	}

  	release_spinlock(&heap_lock);
}