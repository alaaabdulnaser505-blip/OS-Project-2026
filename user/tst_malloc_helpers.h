/*
 * tst_malloc_helpers.h
 *
 *  Created on: Nov 3, 2025
 *      Author: HP
 */

#ifndef USER_TST_MALLOC_HELPERS_H_
#define USER_TST_MALLOC_HELPERS_H_
#include <inc/lib.h>

#define ACTUAL_PAGE_ALLOC_START ((USER_HEAP_START + DYN_ALLOC_MAX_SIZE))
#define MAX_NUM_OF_ALLOCS 40
#define Mega  (1024*1024)
#define kilo (1024)
#define maxByte (0x7F)

int allocIndex;

void* ptr_allocations[MAX_NUM_OF_ALLOCS] = {0};
int lastIndices[MAX_NUM_OF_ALLOCS] = {0};
uint32 requestedSizes[MAX_NUM_OF_ALLOCS] = {0};
uint32 totalRequestedSize ;
bool allocSpaceInPageAlloc(int index, uint32 size, bool writeData, uint32 expectedNumOfTables);
bool freeSpaceInPageAlloc(int index, bool isDataWritten);
int initial_page_allocations();

int inRange(int val, int min, int max)
{
	return (val >= min && val <= max) ? 1 : 0;
}

bool allocSpaceInPageAlloc(int index, uint32 size, bool writeData, uint32 expectedNumOfTables)
{
	int correct = 1;
	int freeFrames = (int)sys_calculate_free_frames() ;
	int usedDiskPages = sys_pf_calculate_allocated_pages() ;
	char *byteArr;

	//Allocate the required size
	requestedSizes[index] = size ;
	uint32 expectedNumOfFrames = ROUNDUP(requestedSizes[index], PAGE_SIZE) / PAGE_SIZE ;
	{
		ptr_allocations[index] = malloc(requestedSizes[index]);
	}

	//Check allocation in RAM & Page File
	expectedNumOfFrames = expectedNumOfTables ;
	uint32 actualNumOfFrames = freeFrames - sys_calculate_free_frames();
	if (!inRange(actualNumOfFrames, expectedNumOfFrames, expectedNumOfFrames + 2 /*Block Alloc: max of 1 page & 1 table*/))
	{correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"1 Wrong allocation in alloc#%d: unexpected number of pages that are allocated in memory! Expected = [%d, %d], Actual = %d\n", index, expectedNumOfFrames, expectedNumOfFrames+2, actualNumOfFrames);}
	if ((sys_pf_calculate_allocated_pages() - usedDiskPages) != 0)
	{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"2 in alloc#%d: Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n", index); }

	lastIndices[index] = (size)/sizeof(char) - 1;
	if (writeData)
	{
		//Write in first & last pages
		freeFrames = sys_calculate_free_frames() ;
		byteArr = (char *) ptr_allocations[index];
		byteArr[0] = maxByte ;
		byteArr[lastIndices[index]] = maxByte ;

		//Check allocation in RAM & Page File
		expectedNumOfFrames = 1; /*table already created in malloc due to marking the allocated pages*/ ;
		if(size > PAGE_SIZE)
			expectedNumOfFrames++ ;

		actualNumOfFrames = (freeFrames - sys_calculate_free_frames()) ;
		if (!inRange(actualNumOfFrames, expectedNumOfFrames, expectedNumOfFrames + 2 /*Block Alloc: max of 1 page & 1 table*/))
		{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"3 Wrong fault handler in alloc#%d: pages are not loaded successfully into memory/WS. Expected diff in frames at least = %d, actual = %d\n", index, expectedNumOfFrames, actualNumOfFrames);}
		if ((sys_pf_calculate_allocated_pages() - usedDiskPages) != 0)
		{ correct = 0; correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"4 in alloc#%d: Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n", index); }

		//Check WS
		uint32 expectedVAs[2] = { ROUNDDOWN((uint32)(&(byteArr[0])), PAGE_SIZE), ROUNDDOWN((uint32)(&(byteArr[lastIndices[index]])), PAGE_SIZE)} ;
		if (sys_check_WS_list(expectedVAs, expectedNumOfFrames, 0, 2) != 1)
		{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"5 Wrong malloc in alloc#%d: page is not added to WS\n", index);}
	}
	return correct;

}

bool freeSpaceInPageAlloc(int index, bool isDataWritten)
{
	int correct = 1;
	int freeFrames = (int)sys_calculate_free_frames() ;
	int usedDiskPages = (int)sys_pf_calculate_allocated_pages() ;
	{
		free(ptr_allocations[index]);
	}

	uint32 expectedNumOfFrames = 0;
	if (isDataWritten)
	{
		expectedNumOfFrames = 1;
		if(requestedSizes[index] > PAGE_SIZE)
			expectedNumOfFrames++ ;
	}
	//Check allocation in RAM & Page File
	if ((usedDiskPages - sys_pf_calculate_allocated_pages()) != 0)
	{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"1 Wrong free in alloc#%d: Extra or less pages are removed from PageFile\n", index);}

	int actualNumOfFrames = (sys_calculate_free_frames() - freeFrames) ;
	if (!inRange(actualNumOfFrames, expectedNumOfFrames, expectedNumOfFrames + 2 /*max of: 1 page for KERN Block Alloc (WSelement) + 1 page for USER block alloc (private DS) */))
	{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"2 Wrong free in alloc#%d: WS pages in memory and/or page tables are not freed correctly\n", index);}

	if (isDataWritten)
	{
		//Check WS
		char* byteArr = (char *) ptr_allocations[index];
		uint32 notExpectedVAs[2] = { ROUNDDOWN((uint32)(&(byteArr[0])), PAGE_SIZE), ROUNDDOWN((uint32)(&(byteArr[lastIndices[index]])), PAGE_SIZE)} ;
		if (sys_check_WS_list(notExpectedVAs, expectedNumOfFrames, 0, 3) != 1)
		{ correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"3 Wrong free in alloc#%d: page is not removed from WS\n", index);}
	}
	return correct;
}

int initial_page_allocations()
{
	/*********************** NOTE ****************************
	 * WE COMPARE THE DIFF IN FREE FRAMES BY "AT LEAST" RULE
	 * INSTEAD OF "EQUAL" RULE SINCE IT'S POSSIBLE FOR SOME
	 * IMPLEMENTATIONS TO DYNAMICALLY ALLOCATE SPECIAL DATA
	 * STRUCTURE TO MANAGE THE PAGE ALLOCATOR.
	 *********************************************************/
	uint32 expectedVA = ACTUAL_PAGE_ALLOC_START; //UHS + 32MB + 4KB

	//malloc some spaces
	int i, freeFrames, usedDiskPages, expectedNumOfTables ;
	uint32 size = 0;
	char* ptr;
	int sums[20] = {0};
	totalRequestedSize = 0;

	int eval = 0;
	bool correct ;

	correct = 1;
	//Create some areas in PAGE allocators
	cprintf_colored(TEXT_cyan,"%~\n	1.1 Create some areas in PAGE allocators\n");
	{
		//4 MB
		allocIndex = 0;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 4*Mega - kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		expectedNumOfTables = 2;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1, expectedNumOfTables);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;
//		cprintf("%~allocation#%d with size %x is DONE\n", allocIndex, size);

		//3 MB
		allocIndex = 1;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 3*Mega - kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		expectedNumOfTables = 0;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1, expectedNumOfTables);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;
//		cprintf("%~allocation#%d with size %x is DONE\n", allocIndex, size);

		//2 MB
		allocIndex = 2;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 2*Mega ;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		expectedNumOfTables = 1;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1, expectedNumOfTables);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;
//		cprintf("%~allocation#%d with size %x is DONE\n", allocIndex, size);

		//4 MB
		allocIndex = 3;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 4*Mega - kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		expectedNumOfTables = 1;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1, expectedNumOfTables);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;
//		cprintf("%~allocation#%d with size %x is DONE\n", allocIndex, size);

		//1 MB
		allocIndex = 4;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 1*Mega - 3*kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		expectedNumOfTables = 0;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1, expectedNumOfTables);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 5;
		correct = 1;

		//1 MB
		allocIndex = 5;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 1*Mega - 2*kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		expectedNumOfTables = 0;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1, expectedNumOfTables);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 5;
		correct = 1;

		//1 MB
		allocIndex = 6;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 1*Mega - 1*kilo;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		expectedNumOfTables = 1; //since page allocator is started 1 page after the 32MB of Block Allocator
		correct = allocSpaceInPageAlloc(allocIndex, size, 1, expectedNumOfTables);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 5;
		correct = 1;

		//2 MB
		allocIndex = 7;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 2*Mega ;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		expectedNumOfTables = 0;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1, expectedNumOfTables);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//2 MB
		allocIndex = 8;
		expectedVA += ROUNDUP(size, PAGE_SIZE);
		size = 2*Mega ;
		totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
		expectedNumOfTables = 1;
		correct = allocSpaceInPageAlloc(allocIndex, size, 1, expectedNumOfTables);
		if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		if (correct) eval += 10;
		correct = 1;

		//ALLOCATIONS OF KILO BYTES
		{
			//3 KB
			allocIndex = 9;
			expectedVA += ROUNDUP(size, PAGE_SIZE);
			size = 3*kilo ;
			totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
			expectedNumOfTables = 0;
			correct = allocSpaceInPageAlloc(allocIndex, size, 1, expectedNumOfTables);
			if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

			//5 KB
			allocIndex = 10;
			expectedVA += ROUNDUP(size, PAGE_SIZE);
			size = 5*kilo ;
			totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
			expectedNumOfTables = 0;
			correct = allocSpaceInPageAlloc(allocIndex, size, 1, expectedNumOfTables);
			if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

			//3 KB
			allocIndex = 11;
			expectedVA += ROUNDUP(size, PAGE_SIZE);
			size = 3*kilo ;
			totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
			expectedNumOfTables = 0;
			correct = allocSpaceInPageAlloc(allocIndex, size, 1, expectedNumOfTables);
			if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }

			//9 KB
			allocIndex = 12;
			expectedVA += ROUNDUP(size, PAGE_SIZE);
			size = 9*kilo ;
			totalRequestedSize += ROUNDUP(size, PAGE_SIZE);
			expectedNumOfTables = 0;
			correct = allocSpaceInPageAlloc(allocIndex, size, 1, expectedNumOfTables);
			if ((uint32) ptr_allocations[allocIndex] != (expectedVA)) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.3 Wrong start address for the allocated space... Expected = %x, Actual = %x\n", allocIndex, expectedVA, ptr_allocations[allocIndex]); }
		}
		if (correct) eval += 15;
		correct = 1;
	}
	//Insufficient space
	cprintf_colored(TEXT_cyan,"%~\n	1.2 Insufficient Space\n");
	{
		allocIndex = 13;
		expectedVA = 0;
		freeFrames = (int)sys_calculate_free_frames() ;
		usedDiskPages = (int)sys_pf_calculate_allocated_pages() ;
		uint32 restOfUHeap = (USER_HEAP_MAX - ACTUAL_PAGE_ALLOC_START) - (totalRequestedSize) ;
		ptr_allocations[allocIndex] = malloc(restOfUHeap+1);
		if (ptr_allocations[allocIndex] != NULL) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.1 Allocating insufficient space: should return NULL\n", allocIndex); }
		if (((int)sys_pf_calculate_allocated_pages() - usedDiskPages) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.2 Page file is changed while it's not expected to. (pages are wrongly allocated/de-allocated in PageFile)\n", allocIndex); }
		if ((freeFrames - (int)sys_calculate_free_frames()) != 0) { correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"%~%d.3 Wrong allocation: pages are not loaded successfully into memory\n", allocIndex); }
	}
	if (correct)	eval+=10 ;

	return eval;
}


#endif /* USER_TST_MALLOC_HELPERS_H_ */
