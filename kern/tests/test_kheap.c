#include "test_kheap.h"

#include <inc/memlayout.h>
#include <inc/queue.h>
#include <inc/dynamic_allocator.h>
#include <kern/cpu/sched.h>
#include <kern/disk/pagefile_manager.h>
#include "../mem/kheap.h"
#include "../mem/memory_manager.h"


/**********************************************************************************************/
/************************ GLOBAL DATA & FUNCTIONS FOR ALL TESTS *******************************/
/**********************************************************************************************/

#define Mega  (1024*1024)
#define kilo (1024)

extern uint32 sys_calculate_free_frames() ;
extern void sys_bypassPageFault(uint8);
extern uint32 sys_rcr2();
extern int execute_command(char *command_string);
extern char end_of_kernel[];
extern int CB(uint32 *ptr_dir, uint32 va, int bn);


/*KMALLOC*/
int test_kmalloc_FF_page();
int test_kmalloc_NF_page();
int test_kmalloc_BF_page();
int test_kmalloc_WF_page();
int test_kmalloc_CF_page();

int test_kmalloc_FF_block();
int test_kmalloc_NF_block();
int test_kmalloc_BF_block();
int test_kmalloc_WF_block();
int test_kmalloc_CF_block();

int test_kmalloc_FF_both();
int test_kmalloc_NF_both();
int test_kmalloc_BF_both();
int test_kmalloc_WF_both();
int test_kmalloc_CF_both();

/*KFREE*/
int test_kfree_FF_page();
int test_kfree_NF_page();
int test_kfree_BF_page();
int test_kfree_WF_page();
int test_kfree_CF_page();

int test_kfree_FF_block();
int test_kfree_NF_block();
int test_kfree_BF_block();
int test_kfree_WF_block();
int test_kfree_CF_block();

int test_kfree_FF_both();
int test_kfree_NF_both();
int test_kfree_BF_both();
int test_kfree_WF_both();
int test_kfree_CF_both();

/*KREALLOC*/
int test_krealloc_FF_page();
int test_krealloc_NF_page();
int test_krealloc_BF_page();
int test_krealloc_WF_page();
int test_krealloc_CF_page();

int test_krealloc_FF_block();
int test_krealloc_NF_block();
int test_krealloc_BF_block();
int test_krealloc_WF_block();
int test_krealloc_CF_block();

int test_krealloc_FF_both();
int test_krealloc_NF_both();
int test_krealloc_BF_both();
int test_krealloc_WF_both();
int test_krealloc_CF_both();

/*FAST PAGE ALLOCATOR*/
int test_fast_FF();
int test_fast_NF();
int test_fast_BF();
int test_fast_WF();
int test_fast_CF(uint32 allocSize);

int test_kmalloc(uint32 ALLOC_TYPE)
{
	if (get_kheap_strategy() == KHP_PLACE_FIRSTFIT)
	{
		//Test FIRST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_FF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_FF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_FF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_BESTFIT)
	{
		//Test BEST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_BF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_BF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_BF_both();
	}
	else if(get_kheap_strategy() == KHP_PLACE_NEXTFIT)
	{
		//Test NEXT FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_NF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_NF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_NF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_WORSTFIT)
	{
		//Test WORST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_WF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_WF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_WF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_CUSTOMFIT)
	{
		//Test CUSTOM FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kmalloc_CF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kmalloc_CF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kmalloc_CF_both();
	}
	return 0;
}

int test_kfree(uint32 ALLOC_TYPE)
{
	if (get_kheap_strategy() == KHP_PLACE_FIRSTFIT)
	{
		//Test FIRST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_FF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_FF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_FF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_BESTFIT)
	{
		//Test BEST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_BF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_BF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_BF_both();
	}
	else if(get_kheap_strategy() == KHP_PLACE_NEXTFIT)
	{
		//Test NEXT FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_NF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_NF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_NF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_WORSTFIT)
	{
		//Test WORST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_WF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_WF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_WF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_CUSTOMFIT)
	{
		//Test CUSTOM FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_kfree_CF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_kfree_CF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_kfree_CF_both();
	}
	return 0;
}
int test_krealloc(uint32 ALLOC_TYPE)
{
	if (get_kheap_strategy() == KHP_PLACE_FIRSTFIT)
	{
		//Test FIRST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_FF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_FF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_FF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_BESTFIT)
	{
		//Test BEST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_BF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_BF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_BF_both();
	}
	else if(get_kheap_strategy() == KHP_PLACE_NEXTFIT)
	{
		//Test NEXT FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_NF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_NF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_NF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_WORSTFIT)
	{
		//Test WORST FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_WF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_WF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_WF_both();
	}
	else if (get_kheap_strategy() == KHP_PLACE_CUSTOMFIT)
	{
		//Test CUSTOM FIT allocation
		if (ALLOC_TYPE == TST_PAGE_ALLOC)
			test_krealloc_CF_page();
		else if (ALLOC_TYPE == TST_BLOCK_ALLOC)
			test_krealloc_CF_block();
		else if (ALLOC_TYPE == TST_BOTH_ALLOC)
			test_krealloc_CF_both();
	}
	return 0;
}

void* ptr_fast_allocations[(KERNEL_HEAP_MAX - KERNEL_HEAP_START)/PAGE_SIZE] = {0};
int test_fast_page_alloc()
{
	if (get_kheap_strategy() == KHP_PLACE_FIRSTFIT)
	{
		test_fast_FF();
	}
	else if (get_kheap_strategy() == KHP_PLACE_NEXTFIT)
	{
		test_fast_NF();
	}
	else if (get_kheap_strategy() == KHP_PLACE_BESTFIT)
	{
		test_fast_BF();
	}
	else if (get_kheap_strategy() == KHP_PLACE_WORSTFIT)
	{
		test_fast_WF();
	}
	else if (get_kheap_strategy() == KHP_PLACE_CUSTOMFIT)
	{
		//Test with large allocation size
		uint32 allocSize = 16*kilo;
		//uint32 allocSize = 64*kilo;
		cprintf_colored(TEXT_green ,"\nTEST FAST ALLOCATION WITH ALLOCATION SIZES OF %dKB\n\n", allocSize/kilo);
		test_fast_CF(allocSize);

	}
	return 0;
}


/**********************************************************************************************/
/*************************** INITIAL ALLOCATION FOR ALL TESTS *********************************/
/**********************************************************************************************/

/**********************************************************************************************/
/********************************** KMALLOC TESTING AREA **************************************/
/**********************************************************************************************/
int test_kmalloc_FF_page()
{
	panic("not implemented function");
}
int test_kmalloc_NF_page()
{
	panic("not implemented function");
}
int test_kmalloc_BF_page()
{
	panic("not implemented function");
}
int test_kmalloc_WF_page()
{
	panic("not implemented function");
}

int test_kmalloc_CF_page()
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/
	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");

	panic("not implemented function");
}

int test_kmalloc_FF_block()
{
	panic("not implemented function");
}
int test_kmalloc_NF_block()
{
	panic("not implemented function");
}
int test_kmalloc_BF_block()
{
	panic("not implemented function");
}
int test_kmalloc_WF_block()
{
	panic("not implemented function");
}
int test_kmalloc_CF_block()
{
	panic("UPDATE IS REQUIRED");

	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");
	panic("not implemented function");
}

int test_kmalloc_FF_both()
{
	panic("not implemented function");
}
int test_kmalloc_NF_both()
{
	panic("not implemented function");
}
int test_kmalloc_BF_both()
{
	panic("not implemented function");
}
int test_kmalloc_WF_both()
{
	panic("not implemented function");
}
int test_kmalloc_CF_both()
{
	panic("not implemented function");
}

/**********************************************************************************************/
/*********************************** KFREE TESTING AREA ***************************************/
/**********************************************************************************************/
int test_kfree_FF_page()
{
	panic("not implemented function");
}
int test_kfree_NF_page()
{
	panic("not implemented function");
}
int test_kfree_BF_page()
{
	panic("not implemented function");
}
int test_kfree_WF_page()
{
	panic("not implemented function");
}
int test_kfree_CF_page()
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/
	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");
	panic("not implemented function");

}

int test_kfree_FF_block()
{
	panic("not implemented function");
}
int test_kfree_NF_block()
{
	panic("not implemented function");
}
int test_kfree_BF_block()
{
	panic("not implemented function");
}
int test_kfree_WF_block()
{
	panic("not implemented function");
}
int test_kfree_CF_block()
{
	panic("not implemented function");
}

int test_kfree_FF_both()
{
	panic("not implemented function");
}
int test_kfree_NF_both()
{
	panic("not implemented function");
}
int test_kfree_BF_both()
{
	panic("not implemented function");
}
int test_kfree_WF_both()
{
	panic("not implemented function");
}
int test_kfree_CF_both()
{
	panic("not implemented function");
}

/**********************************************************************************************/
/****************************** ADDRESS CONVERSION TEST AREA **********************************/
/**********************************************************************************************/
int test_kheap_phys_addr()
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/
	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");
	panic("not implemented function");
}

int test_kheap_virt_addr()
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/

	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");
	panic("not implemented function");
}


/**********************************************************************************************/
/********************************** KREALLOC TESTING AREA *************************************/
/**********************************************************************************************/
int test_krealloc_FF_page()
{
	panic("not implemented function");
}
int test_krealloc_NF_page()
{
	panic("not implemented function");
}
int test_krealloc_BF_page()
{
	panic("not implemented function");
}
int test_krealloc_WF_page()
{
	panic("not implemented function");
}
int test_krealloc_CF_page()
{
	panic("not implemented function");
}

int test_krealloc_FF_block()
{
	panic("not implemented function");
}
int test_krealloc_NF_block()
{
	panic("not implemented function");
}
int test_krealloc_BF_block()
{
	panic("not implemented function");
}
int test_krealloc_WF_block()
{
	panic("not implemented function");
}
int test_krealloc_CF_block()
{
	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");
	panic("not implemented function");
}

int test_krealloc_FF_both()
{
	panic("not implemented function");
}
int test_krealloc_NF_both()
{
	panic("not implemented function");
}
int test_krealloc_BF_both()
{
	panic("not implemented function");
}
int test_krealloc_WF_both()
{
	panic("not implemented function");
}
int test_krealloc_CF_both()
{
	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");
	panic("not implemented function");
}

/**********************************************************************************************/
/*************************** FAST PAGE ALLOCATOR TESTING AREA *********************************/
/**********************************************************************************************/
int test_fast_FF()
{
	panic("not implemented function");
}
int test_fast_NF()
{
	panic("not implemented function");
}
int test_fast_BF()
{
	panic("not implemented function");
}
int test_fast_WF()
{
	panic("not implemented function");
}

int test_fast_CF(uint32 allocSize)
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/

	cprintf_colored(TEXT_yellow, "==============================================\n");
	cprintf_colored(TEXT_yellow, "MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow, "==============================================\n");
	panic("not implemented function");
}




/**********************************************************************************************/
/******************************** OLD IMPLEMENTATION AREA *************************************/
/**********************************************************************************************/

int initFreeFrames;
int initFreeDiskFrames ;
uint8 firstCall = 1 ;
int test_three_creation_functions()
{
	panic("not implemented function");
}


int test_kfreeall()
{
	panic("not implemented function");
}


int test_kexpand()
{
	panic("not implemented function");
}

int test_kshrink()
{
	panic("not implemented function");
}


int test_kfreelast()
{
	panic("not implemented function");
}

