/*
 * test_lists_managment.c

 *
 *  Created on: Oct 6, 2022
 *  Updated on: Sept 20, 2023
 *      Author: HP
 */
#include <inc/queue.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/dynamic_allocator.h>
#include <inc/memlayout.h>

//NOTE: ALL tests in this file shall work with USE_KHEAP = 0

/***********************************************************************************************************************/
#define Mega  (1024*1024)
#define kilo (1024)
#define numOfLevels (BUDDY_UPPER_LEVEL - BUDDY_LOWER_LEVEL + 1)

int check_dynalloc_datastruct(uint32 curSize, uint32 numOfBlksAtCurSize)
{
	panic("Not Implemented");
}
/***********************************************************************************************************************/

void test_initialize_dynamic_allocator()
{
	panic("Not Implemented");
}

int test_initial_alloc()
{
	panic("Not Implemented");
}

void test_alloc_block()
{
	panic("Not Implemented");
}

void test_free_block()
{
	panic("Not Implemented");
}

void test_realloc_block()
{
	panic("unseen test");

#if USE_KHEAP
	panic("test_realloc_block_COMPLETE: the kernel heap should be disabled. make sure USE_KHEAP = 0");
	return;
#endif
}


/********************Helper Functions***************************/
