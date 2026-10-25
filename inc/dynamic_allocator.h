#ifndef FOS_INC_DYNBLK_MANAGE_H
#define FOS_INC_DYNBLK_MANAGE_H
#include <inc/queue.h>
#include <inc/types.h>
#include <inc/environment_definitions.h>

/******************************************************************************/
//TODO: [PROJECT'26.GM#1] DYNAMIC ALLOCATOR - [0] GIVEN DATA
/*Data*/
/*Max Size for the Dynamic Allocator*/
#define BUDDY_LOWER_LEVEL 4
#define BUDDY_UPPER_LEVEL 11
#define DYN_ALLOC_MAX_BLOCK_SIZE (1<<BUDDY_UPPER_LEVEL)		//BuddyLimit = 2^U
#define DYN_ALLOC_MIN_BLOCK_SIZE (1<<BUDDY_LOWER_LEVEL)		//BuddyLimit = 2^U
#define DYN_ALLOC_MAX_SIZE (0) 					//0
struct BlockMetaData
{
	uint8 level;		//block level
	uint8 is_free;		//is_free block?
	LIST_ENTRY(BlockMetaData) prev_next_info;	/* linked list links */
} __attribute__((packed));

#define META_DATA_SIZE (sizeof(struct BlockMetaData))
#define BLOCK_SIZE(LEVEL) (1<<LEVEL)

//Array of lists; Each list is a level that contains the available block in it
LIST_HEAD(BlockList, BlockMetaData);		// Declares 'struct BlockList'
struct BlockList freeBlockLists[BUDDY_UPPER_LEVEL + 1] ;
/******************************************************************************/

/*Functions*/
//Required Functions
void buddy_init();
__inline__ uint32 get_block_size(void *va);
void *alloc_block(uint32 size);
void free_block(void* va);
void *realloc_block(void* va, uint32 new_size);
//In KernelHeap: should be implemented inside kern/mem/kheap.c
//In UserHeap: should be implemented inside lib/uheap.c
void* create_buddy_space();
void  delete_buddy_space(void *va);

#endif
