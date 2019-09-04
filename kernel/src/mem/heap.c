#include <mem/heap.h>
#include <mem/pmm.h>
#include <mem/vmm.h>
#include <libk/stdio.h>
#include <libk/stdbool.h>

uint32_t* heapPos = (uint32_t*)HEAP_POS;

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

/*
 *  All pointers returned by malloc will be aligned for long type.
 *    8 aligned for x86-64 and 4 for x86-32
 *
 *  TODO: find out why glibc malloc returns 16-byte aligned pointers
 *  (http://stackoverflow.com/a/3994235/3051060) 
 */
static inline size_t align_long(size_t size) 
{
	/* This alignment function was borrowed from redis zmalloc */
	if (size & (sizeof(long) - 1)) 
	{
		size += sizeof(long) - (size & (sizeof(long) - 1));
	}

	return size;
}

/*
 *  a suitable block is one that is free and has at least the size we are asking for
 */
static inline bool is_suitable_block(blck_metadata_t* block, size_t size) 
{
	return block->is_free && block->size >= size;
}

/**
 *  Returns a block if it finds a suitable one or NULL otherwise
 *
 *  @param blck_metadata_t* last is used just to make it easy to extend
 *    the heap in case no suitable block is found
 */
blck_metadata_t* get_free_block(blck_metadata_t** last, size_t size) 
{
	blck_metadata_t* block_runner = (blck_metadata_t*)heap_base_ptr;
	
	while (block_runner && !(is_suitable_block(block_runner, size))) 
	{
		*last = block_runner;
		block_runner = block_runner->next;
	}

	return block_runner;
}

/*
 *  Extends the heap using srbk like system calls
 * 
 *  @param blck_metadata_t* last: pointer to the last block allocated before a call to extend_heap
 *  @param size_t size: size of the requested new block
 */
blck_metadata_t* extend_heap(blck_metadata_t* last, size_t size) 
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
	return block;
}

/**
 *  Splits @block to have the @size size and creates a new block with the remaining space
 *
 *  @parm blck_metadata_t* block: block to be splitted
 *  @size_t size new block size
 */
void split_block(blck_metadata_t* block, size_t size) 
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

/*
 *  Malloc implementation using 'first fit' algorithm
 */
void* kmalloc(size_t size) 
{
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

  return block_to_return->blck_metadata_t_end;
}


/*
 *  If a block is next to other empty blocks, merges them into one
 *    This is a way to minimize memory fragmentatio
 *
 *  @param blck_metadata_t* block
 */
blck_metadata_t* merge_block_with_next(blck_metadata_t* block) 
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

/*
 *  Given a @ptr, it returns a pointer the it's blck_metadata_t
 *
 *  @param void* ptr
 */
blck_metadata_t* get_blck_metadata_t_from_ptr(void* ptr) 
{
	char* aux = (char*)ptr; // needed for the pointer arithmetic on the return statement
	return (blck_metadata_t*)(aux - blck_metadata_t_SIZE);
}

bool is_valid_ptr(void* ptr) 
{
  if (heap_base_ptr) 
  {
	if (ptr > heap_base_ptr && ptr < (void *)heapPos) 
	{ // heap range check
		return (ptr == get_blck_metadata_t_from_ptr(ptr)->validation_ptr); // using the validation_ptr we are able to validate @ptr
	}
  }

  return false;
}

/*
 *  Free implementation
 *
 *   It's mandatory that the free function can:
 *     1) Validate the input pointer (is it really a malloc’ed pointer?)
 *     2) Find the meta-data pointer
 */
void kfree(void* ptr) 
{
	if (!is_valid_ptr(ptr)) 
	{
		printf("\nInvalid ptr passed to 'free'\n");
		return;
	}

	blck_metadata_t* block = get_blck_metadata_t_from_ptr(ptr);
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
}