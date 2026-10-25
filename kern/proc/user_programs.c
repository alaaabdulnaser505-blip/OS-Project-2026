/*
 * user_programs.c
 *
 *  Created on: Oct 12, 2022
 *      Author: HP
 */
#include <kern/proc/user_environment.h>
#include <inc/string.h>
#include <inc/assert.h>

//User Programs Table
//The input for any PTR_START_OF macro must be the ".c" filename of the user program
struct UserProgramInfo userPrograms[] = {

		/******************/
		/*COMMON PROGRAMS */
		/******************/
		{ "fos_helloWorld", "Created by FOS team, fos@nowhere.com", PTR_START_OF(fos_helloWorld)},
		{ "fos_add", "Created by FOS team, fos@nowhere.com", PTR_START_OF(fos_add)},
		{ "fos_alloc", "Created by FOS team, fos@nowhere.com", PTR_START_OF(fos_alloc)},
		{ "fos_input", "Created by FOS team, fos@nowhere.com", PTR_START_OF(fos_input)},
		{ "fos_game", "Created by FOS team, fos@nowhere.com", PTR_START_OF(game)},
		{ "fos_static_data_section", "Created by FOS team, fos@nowhere.com", PTR_START_OF(fos_static_data_section)},
		{ "fos_data_on_stack", "Created by FOS team, fos@nowhere.com", PTR_START_OF(fos_data_on_stack)},
		{ "fib_memomize", "", PTR_START_OF(fib_memomize)},
		{ "fib_loop", "", PTR_START_OF(fib_loop)},
		{ "fact", "Factorial Recursive", PTR_START_OF(fos_factorial)},
		{ "fib", "Fibonacci Recursive", PTR_START_OF(fos_fibonacci)},
		{ "qs1", "Quicksort with NO memory leakage", PTR_START_OF(quicksort_noleakage)},
		{ "qs2", "Quicksort that cause memory leakage", PTR_START_OF(quicksort_leakage)},
		{ "mergesort", "mergesort a fixed size array of 800000", PTR_START_OF(mergesort_static)},
		{ "ms1", "Mergesort with NO memory leakage", PTR_START_OF(mergesort_noleakage)},
		{ "ms2", "Mergesort that cause memory leakage", PTR_START_OF(mergesort_leakage)},
		{ "arrop", "Apply set of array operations: scenario program to test semaphores & shared objects", PTR_START_OF(arrayOperations_Master)},
		{ "slave_qs", "SlaveOperation: quicksort", PTR_START_OF(arrayOperations_quicksort)},
		{ "slave_ms", "SlaveOperation: mergesort", PTR_START_OF(arrayOperations_mergesort)},
		{ "slave_stats", "SlaveOperation: stats", PTR_START_OF(arrayOperations_stats)},
		{ "midterm", "Midterm 2017: Example on shared resource and dependency", PTR_START_OF(MidTermEx_Master)},
		{ "midterm_a", "Midterm 2017 Example: Process A", PTR_START_OF(MidTermEx_ProcessA)},
		{ "midterm_b", "Midterm 2017 Example: Process B", PTR_START_OF(MidTermEx_ProcessB)},
		{ "matops", "Matrix Operations on two square matrices with NO memory leakage", PTR_START_OF(matrix_operations)},
		/****************/
		/*FAULT HANDLER */
		/****************/
		{ "tpp", "Tests the Page placement", PTR_START_OF(tst_placement)},
		{"tpplru1","LRU Approx: tests page placement in case the active list is not FULL",PTR_START_OF(tst_placement_1)},
		{"tpplru2","LRU Approx: tests page placement in case the active list is FULL, and the second active list is NOT FULL",PTR_START_OF(tst_placement_2)},
		{"tpplru3","LRU Approx: tests page faults on pages already exist in the second active list (ACCESS)",PTR_START_OF(tst_placement_3)},
		{ "tia", "tests handling of invalid memory access", PTR_START_OF(tst_invalid_access)},
		{ "tia_slave1", "tia: access kernel", PTR_START_OF(tst_invalid_access_slave1)},
		{ "tia_slave2", "tia: write on read only user page", PTR_START_OF(tst_invalid_access_slave2)},
		{ "tia_slave3", "tia: access an unmarked (non-reserved) user heap page", PTR_START_OF(tst_invalid_access_slave3)},
		{ "tia_slave4", "tia: access a non-exist page in page file, stack and heap", PTR_START_OF(tst_invalid_access_slave4)},
		/********************************************/
		{ "tpr1", "Tests page replacement (allocation of Memory and PageFile)", PTR_START_OF(tst_page_replacement_alloc)},
		{ "tpr2", "tests page replacement (handling new stack and modified pages)", PTR_START_OF(tst_page_replacement_stack)},
		{ "dummy_process", "[Slave program] contains nested loops with random bounds to consume time", PTR_START_OF(dummy_process)},
		{ "tfifo1", "Tests page replacement (FIFO algorithm 1)", PTR_START_OF(tst_page_replacement_FIFO_1)},
		{ "tfifo2", "Tests page replacement (FIFO algorithm 2)", PTR_START_OF(tst_page_replacement_FIFO_2)},
		{"tprlru1","LRU Approx: tests allocation in memory and page file after page replacement",PTR_START_OF(tst_page_replacement_LRU_Lists_1)},
		{"tprlru2","LRU Approx: tests the LRU algorithm by emitting a page from the second chance for page replacement",PTR_START_OF(tst_page_replacement_LRU_Lists_2)},
		{"tprlru3","LRU Approx: tests page replacement of stack (creating, modifying and reading them)",PTR_START_OF(tst_page_replacement_stack_LRU_Lists)},
		/********************************************/
		/************/
		/*USER HEAP */
		/************/
		{ "tm1", "tests malloc (1)", PTR_START_OF(tst_malloc_1)},
		{ "tm2", "tests malloc (2)", PTR_START_OF(tst_malloc_2)},
		{ "tm3", "tests malloc (3)", PTR_START_OF(tst_malloc_3)},
		/********************************************/
		{ "tf1", "tests free (1)", PTR_START_OF(tst_free_1)},
		{ "tf1_slave1", "tests free (1) slave1: try accessing values in freed spaces", PTR_START_OF(tst_free_1_slave1)},
		{ "tf1_slave2", "tests free (1) slave2: try accessing values in freed spaces that is not accessed before", PTR_START_OF(tst_free_1_slave2)},
		{ "tf2", "tests free (2)", PTR_START_OF(tst_free_2)},
		{ "tf3", "tests free (3)", PTR_START_OF(tst_free_3)},
		/********************************************/
		{ "tcf1", "tests custom fit (1): page allocator", PTR_START_OF(tst_custom_fit_1)},
		{ "tcf2", "tests custom fit (2): block allocator", PTR_START_OF(tst_custom_fit_2)},
		{ "tcf3", "tests custom fit (3): malloc, smalloc & sget", PTR_START_OF(tst_custom_fit_3)},
		/********************************************/
		{ "hp", "heap program (allocate and free from heap)", PTR_START_OF(heap_program)},
		{ "tfr", "tests freeRAM (1): run in specific scenario", PTR_START_OF(tst_freeRAM)},
		{ "tfr2", "tests freeRAM (2): run in specific scenario", PTR_START_OF(tst_freeRAM_2)},
		/********************************************/
		/****************/
		/*SHARED MEMORY */
		/****************/
		{ "tshr1", "Tests the shared variables [create]", PTR_START_OF(tst_sharing_1)},
		{ "tshr2", "Tests the shared variables [create, get and perms]", PTR_START_OF(tst_sharing_2master)},
		{ "shr2Slave1", "[Slave program1] of tst_sharing_2master", PTR_START_OF(tst_sharing_2slave1)},
		{ "shr2Slave2", "[Slave program2] of tst_sharing_2master", PTR_START_OF(tst_sharing_2slave2)},
		{ "tshr3", "Tests the shared variables [Special cases of create]", PTR_START_OF(tst_sharing_3)},
		/********************************************/
		/*****************/
		/*CPU SCHEDULING */
		/*****************/
		{ "tmlfq1", "Tests CPU scheduling using MLFQ", PTR_START_OF(tst_CPU_MLFQ_master_1)},
		{ "cpuMLFQ1Slave_1", "[Slave program 1] of Test tst_page_replacement_CPU_MLFQ_master_1", PTR_START_OF(tst_CPU_MLFQ_slave_1_1)},
		{ "cpuMLFQ1Slave_2", "[Slave program 2] of Test tst_page_replacement_CPU_MLFQ_master_1", PTR_START_OF(tst_CPU_MLFQ_slave_1_2)},
		{ "tmlfq", "[Master program] of MLFQ Scenario", PTR_START_OF(sc_CPU_MLFQ_Master_1)},
		{ "tmlfq_1", "[Slave program1] of MLFQ Scenario", PTR_START_OF(sc_CPU_MLFQ_slave_1_1)},
		{ "tmlfq_2", "[Slave program2] of MLFQ Scenario", PTR_START_OF(sc_CPU_MLFQ_slave_1_2)},
		/********************************************/
		/**************/
		/*CONCURRENCY */
		/**************/
		{ "tair", "", PTR_START_OF(tst_air)},
		{ "taircl", "", PTR_START_OF(tst_air_clerk)},
		{ "taircu", "", PTR_START_OF(tst_air_customer)},
		/********************************************/
		{ "tst_chan_all", "Tests sleep & wakeup ALL on a channel", PTR_START_OF(tst_chan_all_master)},
		{ "tstChanAllSlave", "Slave program of tst_chan_all", PTR_START_OF(tst_chan_all_slave)},
		{ "tst_chan_one", "Tests sleep & wakeup ONE on a channel", PTR_START_OF(tst_chan_one_master)},
		{ "tstChanOneSlave", "Slave program of tst_chan_one", PTR_START_OF(tst_chan_one_slave)},
		/********************************************/
		{ "tst_sleeplock", "Tests the acquire & release of sleep lock", PTR_START_OF(tst_sleeplock_master)},
		{ "tstSleepLockSlave", "Slave program of tst_sleeplock", PTR_START_OF(tst_sleeplock_slave)},
		/********************************************/
		{ "tst_protection", "Tests the protection of kernel shared DS (e.g. kernel heap)", PTR_START_OF(tst_protection)},
		{ "protection_slave1", "Slave program of tst_protection", PTR_START_OF(tst_protection_slave1)},
		/********************************************/
		/******************/
		/*EXIT (env_free) */
		/******************/
		{ "tef1", "test env_free#1: without user heap [PLACEMENT]", PTR_START_OF(tst_envfree1)},
		{ "tef2", "test env_free#2: using user heap [REPLACEMENT]", PTR_START_OF(tst_envfree2)},
		{ "tef3", "test env_free#3: using user heap & process kill iteself [REPLACEMENT]", PTR_START_OF(tst_envfree3)},
		{ "tef3_slave", "", PTR_START_OF(tst_envfree3_slave)},
		{ "tef4", "test env_free#4: using shared memory (create & get) [REPLACEMENT]", PTR_START_OF(tst_envfree4)},
		{ "tef5_1", "", PTR_START_OF(tst_envfree5_1)},
		{ "tef5_2", "test env_free#5: using shared memory (create, get & free) [PLACEMENT]", PTR_START_OF(tst_envfree5_2)},
		{ "tef6", "test env_free#6: using shared memory (create & get) + semaphores [PLACEMENT]", PTR_START_OF(tst_envfree6)},
		/********************************************/
		{ "ef_ms1", "", PTR_START_OF(ef_mergesort_noleakage)},
		{ "ef_ms2", "", PTR_START_OF(ef_mergesort_leakage)},
		{ "ef_tshr1", "", PTR_START_OF(ef_tst_sharing_1)},
		{ "ef_tshr2", "", PTR_START_OF(ef_tst_sharing_2master)},
		{ "ef_shr2Slave1", "", PTR_START_OF(ef_tst_sharing_2slave1)},
		{ "ef_tshr4", "", PTR_START_OF(ef_tst_sharing_4)},
		{ "ef_tshr5", "", PTR_START_OF(ef_tst_sharing_5_master)},
		{ "ef_tshr5slave", "", PTR_START_OF(ef_tst_sharing_5_slave)},
		{ "ef_tshr5slaveB1", "", PTR_START_OF(ef_tst_sharing_5_slaveB1)},
		{ "ef_tshr5slaveB2", "", PTR_START_OF(ef_tst_sharing_5_slaveB2)},
		{ "ef_midterm", "", PTR_START_OF(ef_MidTermEx_Master)},
		/********************************************/
		/************/
		/*SCENARIOS */
		/************/
		{ "sc_arrops", "Run multiple instances of array operations", PTR_START_OF(sc_arrayOperations)},
		{ "sc_arrops_slave", "Apply set of array operations with predefined input", PTR_START_OF(sc_arrayOperations_slave)},
		{ "slave_ms_static", "SlaveOperation: mergesort", PTR_START_OF(arrayOperations_mergesort_static)},
		{ "sc_air", "Air reservation app with predefined input", PTR_START_OF(sc_air)},
		{ "sc_qs_noleak", "", PTR_START_OF(sc_qs_noleak)},
		{ "sc_qs_leak", "", PTR_START_OF(sc_qs_leakage)},
		{ "sc_qs_leak_small", "", PTR_START_OF(sc_qs_leakage_small)},
		{ "sc_qs_static", "", PTR_START_OF(sc_qs_static)},
		{ "sc_ms_leak", "", PTR_START_OF(sc_ms_leak)},
		{ "sc_ms_leak_small", "", PTR_START_OF(sc_ms_leak_small)},
		{ "sc_ms_noleak", "", PTR_START_OF(sc_ms_noleak)},
		{ "sc_ms_noleak_small", "", PTR_START_OF(sc_ms_noleak_small)},
		{ "sc_fact_recursive", "", PTR_START_OF(sc_fact_recursive)},
		{ "sc_fib_recursive", "", PTR_START_OF(sc_fib_recursive)},
		{ "sc_fib_memomize", "", PTR_START_OF(sc_fib_memomize)},
		{ "sc_fib_loop", "", PTR_START_OF(sc_fib_loop)},
		{ "sc_matops", "", PTR_START_OF(sc_matrix_operations)},
		{ "sc_matops_small", "", PTR_START_OF(sc_matrix_operations_small)},
		{ "sc_repl_policies_1", "Compare OPT, CLK, MODCLK, LRU on QuickSort (static)", PTR_START_OF(sc_repl_policies_1)},
		{ "sc_repl_policies_2", "Compare OPT, CLK on QuickSort (static)", PTR_START_OF(sc_repl_policies_2)},
		{ "sc_FIFO_LRUList", "Compare FIFO vs LRU on QuickSort (leakage)", PTR_START_OF(sc_FIFO_vs_LRU)},
		{ "sc_LEAK_NOLEAK", "Compare LEAKAGE vs NO LEAKAGE on MergeSort", PTR_START_OF(sc_LEAK_vs_NOLEAK)},
		{ "sc_MultipleApps", "Run multiple apps with different nature (no leakage)", PTR_START_OF(sc_MultipleApps)},
		{ "sc_helloWorld", "", PTR_START_OF(sc_helloWorld)},
		{ "sc_fos_add", "", PTR_START_OF(sc_fos_add)},
		{ "sc_ms_static", "", PTR_START_OF(sc_ms_static)},
		{ "sc_alloc", "", PTR_START_OF(sc_alloc)},
		{ "sc_static_data_section", "", PTR_START_OF(sc_static_data_section)},
		{ "sc_data_on_stack", "", PTR_START_OF(sc_data_on_stack)},
		/********************************************/
};

///=========================================================

// To be used as extern in other files
struct UserProgramInfo* ptr_UserPrograms = &userPrograms[0];

// Number of user programs in the program table
int NUM_USER_PROGS = (sizeof(userPrograms)/sizeof(userPrograms[0]));

struct UserProgramInfo* get_user_program_info(char* user_program_name)
{
	int i;
	for (i = 0; i < NUM_USER_PROGS; i++) {
		if (strcmp(user_program_name, userPrograms[i].name) == 0)
			break;
	}
	if(i==NUM_USER_PROGS)
	{
		cprintf("Unknown user program '%s'\n", user_program_name);
		return 0;
	}

	return &userPrograms[i];
}

struct UserProgramInfo* get_user_program_info_by_env(struct Env* e)
{
	int i;
	for (i = 0; i < NUM_USER_PROGS; i++) {
		if ( strcmp( e->prog_name , userPrograms[i].name) ==0)
			break;
	}
	if(i==NUM_USER_PROGS)
	{
		cprintf("Unknown user program \n");
		return 0;
	}

	return &userPrograms[i];
}
