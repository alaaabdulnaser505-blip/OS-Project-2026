/*
 * dynamic_allocator.c
 *
 *  Created on: Sep 21, 2023
 *      Author: HP
 */
#include <inc/assert.h>
#include <inc/string.h>
#include "../inc/dynamic_allocator.h"

/************************************************************************************/
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
//TODO: [PROJECT'26.GM#1] DYNAMIC ALLOCATOR - [0] GIVEN FUNCTIONS
//==================================
// [1] INITIALIZE BUDDY FREE LISTS:
//==================================
void buddy_init()
{
	//panic("buddy_init is not implemented yet");

	for (int l = BUDDY_LOWER_LEVEL; l <= BUDDY_UPPER_LEVEL; ++l)
	{
		LIST_INIT(&(freeBlockLists[l]));
	}
}
//====================================
// [2] PRINT SIZE OF FREE LISTS:
//====================================
void PrintFreeBlockLists()
{
	for (int i = BUDDY_LOWER_LEVEL; i <= BUDDY_UPPER_LEVEL; ++i)
	{
		cprintf("Level#%d: %d\n", i, LIST_SIZE(&freeBlockLists[i]));
	}
}
/************************************************************************************/

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//
//===========================
// [1] GET BLOCK SIZE:
//===========================
__inline__ uint32 get_block_size(void *va)
{
	//TODO: [PROJECT'26.GM#1] DYNAMIC ALLOCATOR - [1] get_block_size
	//Your code is here
	struct BlockMetaData *BLOCK_META_DATA=(struct BlockMetaData*)((uint8 *)va - META_DATA_SIZE);
	return BLOCK_SIZE(BLOCK_META_DATA->level);
	//Comment the following line
	//panic("get_block_size() not implemented");

}
//===========================
// [2] ALLOCATE BLOCK:
//===========================
void *alloc_block(uint32 requiredSize)
{
//TODO: [PROJECT'26.GM#1] DYNAMIC ALLOCATOR - [2] alloc_block
//Your code is here
	uint32 TOTAL_SIZE = requiredSize + META_DATA_SIZE;
	uint32 MIN_SIZE_CAN_BE_ALLOCATED=BUDDY_LOWER_LEVEL;

	if(requiredSize ==0){
		return NULL;
	}

	for(;MIN_SIZE_CAN_BE_ALLOCATED<=BUDDY_UPPER_LEVEL;){
		if(BLOCK_SIZE(MIN_SIZE_CAN_BE_ALLOCATED)<TOTAL_SIZE)
		{
		  MIN_SIZE_CAN_BE_ALLOCATED++;
		}
		else{
			break;
		}
	}
	if(MIN_SIZE_CAN_BE_ALLOCATED>BUDDY_UPPER_LEVEL){
		return NULL;
	}
	uint32 PROPER_LEVEL = MIN_SIZE_CAN_BE_ALLOCATED;

	for(;PROPER_LEVEL<=BUDDY_UPPER_LEVEL;PROPER_LEVEL++){
		if(!LIST_EMPTY(&freeBlockLists[PROPER_LEVEL]))
			break;
	}
	if(PROPER_LEVEL>BUDDY_UPPER_LEVEL){
		return NULL;
	}

	struct BlockMetaData *MIN_BLOCK_TO_ALLOCATE=LIST_FIRST(&freeBlockLists[PROPER_LEVEL]);
	LIST_REMOVE(&freeBlockLists[PROPER_LEVEL],MIN_BLOCK_TO_ALLOCATE);

	for(;PROPER_LEVEL>MIN_SIZE_CAN_BE_ALLOCATED;){
		PROPER_LEVEL--;
		uint32 HALF_BLOCK_SIZE=BLOCK_SIZE(PROPER_LEVEL);
		struct BlockMetaData *BUDDY_BLOCK=(struct BlockMetaData*)((uint8*)MIN_BLOCK_TO_ALLOCATE+HALF_BLOCK_SIZE);
		BUDDY_BLOCK->level=PROPER_LEVEL;
		BUDDY_BLOCK->is_free=1;

		MIN_BLOCK_TO_ALLOCATE->level=PROPER_LEVEL;

		LIST_INSERT_HEAD(&freeBlockLists[PROPER_LEVEL],BUDDY_BLOCK);
	}

	MIN_BLOCK_TO_ALLOCATE->level=MIN_SIZE_CAN_BE_ALLOCATED;
	MIN_BLOCK_TO_ALLOCATE->is_free=0;
	return (void*)((uint8*)MIN_BLOCK_TO_ALLOCATE+META_DATA_SIZE);

	//Comment the following line
	//panic("alloc_block() not implemented");
	}

//===================================================
// [3] FREE BLOCK WITH COALESCING:
//===================================================
void free_block(void *va)
{
    if (va == NULL)
        return;

    struct BlockMetaData *currBlk =(struct BlockMetaData *)((uint8 *)va - META_DATA_SIZE);

    uint32 currLvl = currBlk->level;
    struct BlockMetaData*itr;
    struct BlockMetaData*BuddyBlk;
    int isFnd;
    currBlk->is_free = 1;

   for(;currLvl<BUDDY_UPPER_LEVEL;)
   {
	   BuddyBlk=(struct BlockMetaData *)((uint32)currBlk^BLOCK_SIZE(currLvl));

	   if(BuddyBlk->level!=currLvl)
		   break;

	   if(BuddyBlk->is_free!=1)
		   break;

	   isFnd=0;

	   for(itr=freeBlockLists[currLvl].lh_first;itr!=NULL;itr=itr->prev_next_info.le_next)
	   {
		   if(itr==BuddyBlk){
			   isFnd=1;
			   break;
		   }
	   }

	   if(isFnd==0)
		   break;

	   LIST_REMOVE(&freeBlockLists[currLvl],BuddyBlk);
	   if(BuddyBlk<currBlk){
		   currBlk=BuddyBlk;

	   }
	   currLvl++;
	   currBlk->level=currLvl;
	   currBlk->is_free=1;
   }

    LIST_INSERT_HEAD(&freeBlockLists[currLvl], currBlk);
}



//=================================================================================//
//============================== BONUS FUNCTION ===================================//
//=================================================================================//

//===========================
// [1] REALLOCATE BLOCK:
//===========================
void *realloc_block(void* va, uint32 newSize)
{
	//TODO: [PROJECT'26.BONUS#1] KERNEL REALLOC - realloc_block
	//Your code is here
	//Comment the following line
	panic("realloc_block() Not implemented yet");
}
