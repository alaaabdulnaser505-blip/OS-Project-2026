/* *********************************************************** */
/* MAKE SURE PAGE_WS_MAX_SIZE = 20 */
/* *********************************************************** */

#include <inc/lib.h>
uint32 expectedInitialVAs[15] = {
		0x800000, 0x801000, 0x802000,		//Code
		0x803000,0x804000,0x805000,0x806000,0x807000,0x808000,0x809000,0x80a000,0x80b000, 	//Data
		0xeebfd000, 0xedbfd000 /*will be created during the call of sys_check_WS_list*/,	//Stack
		0x81b000 /*for the text color variable*/} ;


void _main(void)
{
#if USE_KHEAP
	//	cprintf_colored(TEXT_cyan,"envID = %d\n",envID);

	char arr[PAGE_SIZE*1024*4];
	bool found ;
	cprintf_colored(TEXT_cyan,"STEP 0: checking Initial WS entries ...\n");
	{
		found = sys_check_WS_list(expectedInitialVAs, 15, 0, 1);
		if (found != 1) panic("InitialWSError2: INITIAL PAGE WS entry checking failed! Review size of the WS..!!");

		/*NO NEED FOR THIS IF REPL IS "LRU"*/
		if( myEnv->page_last_WS_element !=  NULL)
			panic("INITIAL PAGE last WS checking failed! Review size of the WS..!!");
		/*====================================*/
	}
	int eval = 0;
	bool is_correct = 1;

	cprintf_colored(TEXT_cyan,"\nSTEP 1: checking USER KERNEL STACK... [20%]\n");
	{
		uint32 stackIsCorrect;
		sys_utilities("__CheckUserKernStack__", (uint32)(&stackIsCorrect));
		if (!stackIsCorrect)
		{
			is_correct = 0;
			cprintf_colored(TEXT_TESTERR_CLR,"User Kernel Stack is not set correctly\n");
		}
	}
	if (is_correct) eval += 20;

	cprintf_colored(TEXT_cyan,"\nSTEP 2: checking PLACEMENT...\n");
	{
		int usedDiskPages = sys_pf_calculate_allocated_pages() ;
		int freePages = sys_calculate_free_frames();
		//2 stack pages & page table
		int i=PAGE_SIZE*1024;
		for(;i<=(PAGE_SIZE*1024 + PAGE_SIZE);i++)
		{
			arr[i] = 1;
		}

		//2 stack pages & page table
		i=PAGE_SIZE*1024*2;
		for(;i<=(PAGE_SIZE*1024*2 + PAGE_SIZE);i++)
		{
			arr[i] = 2;
		}

		//2 stack pages & page table
		i=PAGE_SIZE*1024*3;
		for(;i<=(PAGE_SIZE*1024*3 + PAGE_SIZE);i++)
		{
			arr[i] = 3;
		}

		is_correct = 1;
		cprintf_colored(TEXT_cyan,"	STEP A: checking PLACEMENT fault handling... [30%] \n");
		{
			if( arr[PAGE_SIZE*1024] !=  1)  { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"PLACEMENT of stack page failed\n");}
			if( arr[PAGE_SIZE*1025] !=  1)  { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"PLACEMENT of stack page failed\n");}

			if( arr[PAGE_SIZE*1024*2] !=  2)  { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"PLACEMENT of stack page failed\n");}
			if( arr[PAGE_SIZE*1024*2 + PAGE_SIZE] !=  2)  { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"PLACEMENT of stack page failed\n");}

			if( arr[PAGE_SIZE*1024*3] !=  3)  { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"PLACEMENT of stack page failed\n");}
			if( arr[PAGE_SIZE*1024*3 + PAGE_SIZE] !=  3)  { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"PLACEMENT of stack page failed\n");}

			if( (sys_pf_calculate_allocated_pages() - usedDiskPages) !=  0) { is_correct = 0; cprintf_colored(TEXT_cyan,"new stack pages should NOT be written to Page File until evicted as victim\n");}

			int expected = 6 /*pages*/ + 3 /*tables*/;
			if( (freePages - sys_calculate_free_frames() ) != expected )
			{ is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"allocated memory size incorrect. Expected Difference = %d, Actual = %d\n", expected, (freePages - sys_calculate_free_frames() ));}
		}
		cprintf_colored(TEXT_cyan,"	STEP A finished: PLACEMENT fault handling !\n\n\n");

		if (is_correct)
		{
			eval += 30;
		}
		is_correct = 1;

		cprintf_colored(TEXT_cyan,"	STEP B: checking WS entries... [30%]\n");
		{
			uint32 expectedPages[21] ;
			{
				expectedPages[0] = 0x800000 ;
				expectedPages[1] = 0x801000 ;
				expectedPages[2] = 0x802000 ;
				expectedPages[3] = 0x803000 ;
				expectedPages[4] = 0x804000 ;
				expectedPages[5] = 0x805000 ;
				expectedPages[6] = 0x806000 ;
				expectedPages[7] = 0x807000 ;
				expectedPages[8] = 0x808000 ;
				expectedPages[9] = 0x809000 ;
				expectedPages[10] = 0x80a000 ;
				expectedPages[11] = 0x80b000 ;
				expectedPages[12] = 0xeebfd000 ;
				expectedPages[13] = 0xedbfd000 ;
				expectedPages[14] = 0x81b000 ;
				expectedPages[15] = 0xedffd000 ;
				expectedPages[16] = 0xedffe000 ;
				expectedPages[17] = 0xee3fd000 ;
				expectedPages[18] = 0xee3fe000 ;
				expectedPages[19] = 0xee7fd000 ;
				expectedPages[20] = 0xee7fe000 ;
			}
			found = sys_check_WS_list(expectedPages, 21, 0, 1);
			if (found != 1)
			{ is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"PAGE WS entry checking failed... trace it by printing page WS before & after fault\n");}
		}
		cprintf_colored(TEXT_cyan,"	STEP B finished: WS entries test \n\n\n");
		if (is_correct)
		{
			eval += 30;
		}
		is_correct = 1;
		cprintf_colored(TEXT_cyan,"	STEP C: checking working sets WHEN BECOMES FULL... [20%]\n");
		{
			/*NO NEED FOR THIS IF REPL IS "LRU"*/
			if(myEnv->page_last_WS_element != NULL)
			{ is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"wrong PAGE WS pointer location... trace it by printing page WS before & after fault\n");}

			//1 stack page
			i=PAGE_SIZE*1024*3 + 2*PAGE_SIZE;
			arr[i] = 4;

			if( arr[i] != 4)  { is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"PLACEMENT of stack page failed\n");}

			//		uint32 expectedPages[20] = {
			//				0x200000,0x201000,0x202000,0x203000,0x204000,0x205000,0x206000,0x207000,
			//				0x800000,0x801000,0x802000,0x803000,
			//				0xeebfd000,0xedbfd000,0xedbfe000,0xedffd000,0xedffe000,0xee3fd000,0xee3fe000,0xee7fd000};
			uint32 expectedPages[22] ;
			{
				expectedPages[0] = 0x800000 ;
				expectedPages[1] = 0x801000 ;
				expectedPages[2] = 0x802000 ;
				expectedPages[3] = 0x803000 ;
				expectedPages[4] = 0x804000 ;
				expectedPages[5] = 0x805000 ;
				expectedPages[6] = 0x806000 ;
				expectedPages[7] = 0x807000 ;
				expectedPages[8] = 0x808000 ;
				expectedPages[9] = 0x809000 ;
				expectedPages[10] = 0x80a000 ;
				expectedPages[11] = 0x80b000 ;
				expectedPages[12] = 0xeebfd000 ;
				expectedPages[13] = 0xedbfd000 ;
				expectedPages[14] = 0x81b000 ;
				expectedPages[15] = 0xedffd000 ;
				expectedPages[16] = 0xedffe000 ;
				expectedPages[17] = 0xee3fd000 ;
				expectedPages[18] = 0xee3fe000 ;
				expectedPages[19] = 0xee7fd000 ;
				expectedPages[20] = 0xee7fe000 ;
				expectedPages[21] = 0xee7ff000 ;
			}
			found = sys_check_WS_list(expectedPages, 22, 0x800000, 1);
			if (found != 1)
			{ is_correct = 0; cprintf_colored(TEXT_TESTERR_CLR,"PAGE WS entry checking failed... trace it by printing page WS before & after fault\n");}
		}
		cprintf_colored(TEXT_cyan, "	STEP C finished: WS is FULL now\n\n\n");
		if (is_correct)
		{
			eval += 20;
		}
	}
	cprintf_colored(TEXT_light_green, "%~\nTest of KERNEL STACK & PAGE PLACEMENT completed. Eval = %d%\n\n", eval);

	return;
#endif
}

