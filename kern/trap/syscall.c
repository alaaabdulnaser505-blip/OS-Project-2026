/* See COPYRIGHT for copyright information. */

#include <inc/x86.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/semaphore.h>
#include <inc/syscall.h>
#include <inc/syscall.h>

#include <kern/proc/user_environment.h>
#include "trap.h"
#include <kern/cons/console.h>
#include <kern/conc/channel.h>
#include <kern/cpu/sched.h>
#include <kern/cpu/cpu.h>
#include <kern/disk/pagefile_manager.h>
#include <kern/mem/memory_manager.h>
#include <kern/mem/shared_memory_manager.h>
#include <kern/tests/utilities.h>
#include <kern/tests/test_working_set.h>

extern uint8 bypassInstrLength ;
//struct Env* cur_env ;
/*******************************/
/* STRING I/O SYSTEM CALLS */
/*******************************/
//TODO [PROJECT.NOTES]: in all 3 string system calls: Due to the "Big Kern Lock", other CPUs can't get into here while one inside. el7 :)
//			However, DISABLE INT is required to prevent the current CPU from scheduling another process, after clock interrupt, before finishing the job
//			This is to protect the shared console buffer while accessed via user system call
//			since CLOCK INT can occur while accessing it causing the CPU to schedule another process
//			which in turn can cause race condition on this buffer... el7 :)

// Print a string to the system console.
// The string is exactly 'len' characters long.
// Destroys the environment on memory errors.
void sys_cputs(const char *s, uint32 len, uint8 printProgName, int color)
{
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);
	//TODO [PROJECT.NOTES]: Due to the "Big Kern Lock", other CPUs can't get into here while one inside. el7 :)
	//			 However, DISABLE INT is required to prevent the current CPU from scheduling another process, after clock interrupt, before finishing the print
	pushcli();	//disable interrupts
	{
		// Check that the user has permission to read memory [s, s+len).
		// Destroy the environment if not.

		// LAB 3: Your code here.

		current_text_color = color; // setting text color to be used in cga_putc()
		{
			// Print the string supplied by the user.
			if (printProgName)
				cprintf("[%s %d @CPU[%d]] ",cur_env->prog_name, cur_env->env_id, cpunum());
			cprintf("%.*s",len, s);
		}
		current_text_color = TEXT_DEFAULT_CLR; //restore default text color
	}
	popcli();	//enable interrupts
}


// Print a char to the system console.
 void sys_cputc(const char c)
{
	// Print the char supplied by the user.
	pushcli();	//disable interrupts
	{
		cprintf("%c",c);
	}
	popcli();	//enable interrupts
}


// Read a character from the system console.
// Returns the character.
int sys_cgetc(void)
{
	int c;

	if (KB_IO_METHOD != PROGRAMMED_IO) /*Interrupt-Enabled I/O*/
	{
		//cprintf("\n(((((((Interrupt-Enabled I/O))))))\n");

		// The cons_getc2() primitive doesn't wait for a character
		pushcli();	//disable interrupts
		{
			c = cons_getc2();
		}
		popcli();	//enable interrupts

		while (c == 0)
		{
			//should sleep (i.e. blocked) until a IRQ1 (KB) interrupt occur
			if (KB_IO_METHOD == INT_SLEEP)
			{
				acquire_kspinlock(&KBDlock);
				{
					sleep(&KBDchannel, &KBDlock);
				}
				release_kspinlock(&KBDlock);
			}
			else if (KB_IO_METHOD == INT_SEMAPHORE)
			{
				wait_ksemaphore(&KBDsem);
			}
			else
			{
				panic("sys_cgetc: unhandled lock method for the Interrupt-Enabled I/O");
			}
			pushcli();	//disable interrupts
			{
				c = cons_getc2();
			}
			popcli();	//enable interrupts
		}
	}
	else	/*Programmed I/O*/
	{
		//cprintf("\n(((((((Programmed I/O))))))\n");
		// The cons_getc() primitive doesn't wait for a character,
		// but the sys_cgetc() system call, in user side, does.
		pushcli();	//disable interrupts
		{
			c = cons_getc();
		}
		popcli();	//enable interrupts
		//while ((c = cons_getc()) == 0)
		{
			//cprintf("do nothing\n");
			/* do nothing */;
		}
	}
	//cprintf("\nCHAR %d is READ from KB, IEN = %d\n", c, read_eflags() & FL_IF);

	return c;
}

//Lock the console so that no other processes can read from KB or output to the monitor
void sys_lock_cons(void)
{
	cons_lock();
}
//Unlock the console so that other processes can read from KB or output to the monitor
void sys_unlock_cons(void)
{
	cons_unlock();
}

/*******************************/
/* MEMORY SYSTEM CALLS */
/*******************************/
// Allocate a page of memory and map it at 'va' with permission
// 'perm' in the address space of the current environment.
// The page's contents are set to 0.
// If a page is already mapped at 'va', that page is unmapped as a side effect.
//
// perm -- PERM_USER | PERM_PRESENT must be set, PERM_AVAIL | PERM_WRITEABLE may or may not be set,
//         but no other bits may be set.
//
// Return 0 on success, < 0 on error.  Errors are:
//	E_BAD_ENV if environment doesn't currently exist,
//	E_INVAL if va >= USER_TOP, or va is not page-aligned.
//	E_INVAL if perm is inappropriate (see above).
//	E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
int __sys_allocate_page(void *va, int perm)
{
	// Hint: This function is a wrapper around allocate_frame() and
	//   map_frame() from kern/mem/memory_manager.c.
	//   Most of the new code you write should be to check the
	//   parameters for correctness.
	//   If map_frame() fails, remember to free the page you allocated!

	struct Env* cur_env = get_cpu_proc();
	if(cur_env == NULL)
		return E_BAD_ENV;

	//check virtual address to be paged_aligned and < USER_TOP
	if ((uint32)va >= USER_TOP || (uint32)va % PAGE_SIZE != 0)
		return E_INVAL;

	//check permissions to be appropriate
	if ((perm & (~PERM_AVAILABLE & ~PERM_WRITEABLE)) != (PERM_USER))
		return E_INVAL;

	int ret = alloc_page(cur_env->env_page_directory, (uint32)va, perm, 1);
	if (ret == E_NO_MEM)
		return ret ;

	return 0 ;
}

// Map the page of memory at 'srcva' in srcenvid's address space
// at 'dstva' in dstenvid's address space with permission 'perm'.
// Perm has the same restrictions as in sys_page_alloc, except
// that it also must not grant write access to a read-only
// page.
//
// Return 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if srcenvid and/or dstenvid doesn't currently exist,
//		or the caller doesn't have permission to change one of them.
//	-E_INVAL if srcva >= UTOP or srcva is not page-aligned,
//		or dstva >= UTOP or dstva is not page-aligned.
//	-E_INVAL is srcva is not mapped in srcenvid's address space.
//	-E_INVAL if perm is inappropriate (see sys_page_alloc).
//	-E_INVAL if (perm & PTE_W), but srcva is read-only in srcenvid's
//		address space.
//	-E_NO_MEM if there's no memory to allocate the new page,
//		or to allocate any necessary page tables.
int __sys_map_frame(int32 srcenvid, void *srcva, int32 dstenvid, void *dstva, int perm)
{
	// Hint: This function is a wrapper around page_lookup() and
	//   page_insert() from kern/pmap.c.
	//   Again, most of the new code you write should be to check the
	//   parameters for correctness.
	//   Use the third argument to page_lookup() to
	//   check the current permissions on the page.

	// LAB 4: Your code here.
	panic("sys_map_frame not implemented");
	return 0;

}

// Unmap the page of memory at 'va' in the address space of the current environment.
// If no page is mapped, the function silently succeeds.
//
// Return 0 on success, < 0 on error.  Errors are:
//	E_BAD_ENV if environment doesn't currently exist,
//	E_INVAL if va >= USER_TOP, or va is not page-aligned.
int __sys_unmap_frame(uint32 va)
{
	struct Env* cur_env = get_cpu_proc();
	if(cur_env == NULL)
		return E_BAD_ENV;

	if (va >= USER_TOP || va % PAGE_SIZE != 0)
		return E_INVAL;

	unmap_frame(cur_env->env_page_directory, va);
	return 0;
}

uint32 sys_calculate_required_frames(uint32 start_virtual_address, uint32 size)
{
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);

	return calculate_required_frames(cur_env->env_page_directory, start_virtual_address, size);
}

uint32 sys_calculate_free_frames()
{
	struct freeFramesCounters counters = calculate_available_frames();
	//	cprintf("Free Frames = %d : Buffered = %d, Not Buffered = %d\n", counters.freeBuffered + counters.freeNotBuffered, counters.freeBuffered ,counters.freeNotBuffered);
	return counters.freeBuffered + counters.freeNotBuffered;
}
uint32 sys_calculate_modified_frames()
{
	struct freeFramesCounters counters = calculate_available_frames();
	//	cprintf("================ Modified Frames = %d\n", counters.modified) ;
	return counters.modified;
}

uint32 sys_calculate_notmod_frames()
{
	struct freeFramesCounters counters = calculate_available_frames();
	//	cprintf("================ Not Modified Frames = %d\n", counters.freeBuffered) ;
	return counters.freeBuffered;
}

int sys_calculate_pages_tobe_removed_ready_exit(uint32 WS_or_MEMORY_flag)
{
	return calc_no_pages_tobe_removed_from_ready_exit_queues(WS_or_MEMORY_flag);
}

void sys_scarce_memory(void)
{
	scarce_memory();
}

void sys_clearFFL()
{
	int size;
	acquire_kspinlock(&MemFrameLists.mfllock);
	{
		size = LIST_SIZE(&MemFrameLists.free_frame_list) ;
		struct FrameInfo* ptr_tmp_FI ;
		for (int i = 0; i < size ; i++)
		{
			allocate_frame(&ptr_tmp_FI) ;
		}
	}
	release_kspinlock(&MemFrameLists.mfllock);
}

/*******************************/
/* PAGE FILE SYSTEM CALLS */
/*******************************/
int sys_pf_calculate_allocated_pages(void)
{
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);

	return pf_calculate_allocated_pages(cur_env);
}

/*******************************/
/* USER HEAP SYSTEM CALLS */
/*******************************/
void sys_free_user_mem(uint32 virtual_address, uint32 size)
{
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);
	if(isBufferingEnabled())
	{
		__free_user_mem_with_buffering(cur_env, virtual_address, size);
	}
	else
	{
		free_user_mem(cur_env, virtual_address, size);
	}
	return;
}

void sys_allocate_user_mem(uint32 virtual_address, uint32 size)
{
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);
	allocate_user_mem(cur_env, virtual_address, size);
	return;
}

void sys_allocate_chunk(uint32 virtual_address, uint32 size, uint32 perms)
{
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);
	allocate_chunk(cur_env->env_page_directory, virtual_address, size, perms);
	return;
}

//2014
void sys_move_user_mem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size)
{
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);

	move_user_mem(cur_env, src_virtual_address, dst_virtual_address, size);
	return;
}

//2015
uint32 sys_get_heap_strategy()
{
	return _UHeapPlacementStrategy ;
}
void sys_set_uheap_strategy(uint32 heapStrategy)
{
	_UHeapPlacementStrategy = heapStrategy;
}

/*******************************/
/* SEMAPHORES SYSTEM CALLS */
/*******************************/

/*******************************/
/* SHARED MEMORY SYSTEM CALLS */
/*******************************/
int sys_create_shared_object(char* shareName, uint32 size, uint8 isWritable, void* startVA)
{
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);
	return create_shared_object(cur_env->env_id, shareName, size, isWritable, startVA);;
}

int sys_get_share_info(uint32 ownerID, char* shareName, int *size)
{
	return get_share_info(ownerID, shareName, size);
}

int sys_get_shared_object(int shareID, void* startVA)
{
	return get_shared_object(shareID, startVA);
}

int sys_delete_shared_object(int shareID, void *startVA)
{
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);
	return delete_shared_object(cur_env, shareID, startVA);
}

/*********************************/
/* USER ENVIRONMENT SYSTEM CALLS */
/*********************************/
// Returns the current environment's envid.
//2017
int32 sys_getenvid(void)
{
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);

	return cur_env->env_id;
}

//2017
int32 sys_getenvindex(void)
{
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);

	//return cur_env->env_id;
	return (cur_env - envs) ;
}

//2017
int32 sys_getparentenvid(void)
{
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);

	return cur_env->env_parent_id;
}

// Destroy a given environment whatever its state & DON'T place it in EXIT
// if envid=0, destroy the currently running environment --> schedule the next (if any)
//
// Returns 0 on success, < 0 on error.  Errors are:
//	-E_BAD_ENV if environment envid doesn't currently exist,
//		or the caller doesn't have permission to change envid.
int sys_destroy_env(int32 envid)
{
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);

	int r;
	struct Env *e;
	if (envid == 0)
	{
		e = cur_env ;
	}
	else if ((r = envid2env(envid, &e, 0)) < 0)
	{
		return r;
	}

	if (e == cur_env)
	{
		cprintf("[%08x] exiting gracefully\n", cur_env->env_id);
	}
	else
	{
		cprintf("[%08x] destroying %08x\n", cur_env->env_id, e->env_id);
	}
	//2015
	sched_kill_env(e->env_id);

	return 0;
}

//Just place the current env into the EXIT queue & schedule the next one
void sys_exit_env()
{
	//2015
	env_exit();
}

//New update in 2020
//Create a new env & add it to the NEW queue
int sys_create_env(char* programName, unsigned int page_WS_size,unsigned int LRU_second_list_size, unsigned int percent_WS_pages_to_remove)
{
	//cprintf("\nAttempt to create a new env\n");

	struct Env* env =  env_create(programName, page_WS_size, LRU_second_list_size, percent_WS_pages_to_remove);
	if(env == NULL)
	{
		return E_ENV_CREATION_ERROR;
	}
	//cprintf("\nENV %d is created\n", env->env_id);

	//2015
	sched_new_env(env);

	//cprintf("\nENV %d is scheduled as NEW\n", env->env_id);

	return env->env_id;
}

//Place a new env into the READY queue
void sys_run_env(int32 envId)
{
	sched_run_env(envId);
}

//Calculate the number of page faults for the OPTIMAL replacement
int sys_get_optimal_num_faults()
{
#if USE_KHEAP
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);

	struct WorkingSetElement *ptrWSE ;
	int w = 0;
	LIST_FOREACH(ptrWSE, &(cur_env->page_WS_active_list),WorkingSetElement)
	{
		if (ROUNDDOWN(ptrWSE->virtual_address, PAGE_SIZE) != ROUNDDOWN(cur_env->prepagedVAs[w++], PAGE_SIZE))
		{
			panic("sys_get_optimal_num_faults(): page working set is changed during the OPTIMAL replacement while it's not expected to");
		}
	}
	return get_optimal_num_faults(&(cur_env->page_WS_active_list), cur_env->page_WS_max_size, &(cur_env->referenceStreamList));
#else
	panic("MUST ENABLE KHEAP");
#endif
	return 0;
}

//====================================
/*******************************/
/* ETC... SYSTEM CALLS */
/*******************************/
uint32 sys_rcr2()
{
	return rcr2();
}
void sys_bypassPageFault(uint8 instrLength)
{
	bypassInstrLength = instrLength;
}


/**************************************************************************/
/************************* SYSTEM CALLS HANDLER ***************************/
/**************************************************************************/
// Dispatches to the correct kernel function, passing the arguments.
inline uint32 syscall(uint32 syscallno, uint32 a1, uint32 a2, uint32 a3, uint32 a4, uint32 a5)
{
	struct Env* cur_env = get_cpu_proc();
	assert(cur_env != NULL);

	//cprintf("syscallno = %d\n", syscallno);
	// Call the function corresponding to the 'syscallno' parameter.
	// Return any appropriate return value.
	switch(syscallno)
	{
	case SYS_allocate_user_mem:
		sys_allocate_user_mem(a1, a2);
		return 0;
		break;
	case SYS_free_user_mem:
		sys_free_user_mem(a1, a2);
		return 0;
		break;
	case SYS_cputs:
		sys_cputs((const char*)a1,a2,(uint8)a3, a4);
		return 0;
		break;
	case SYS_cgetc:
		return sys_cgetc();
		break;
	case SYS_cputc:
		sys_cputc((const char)a1);
		return 0;
		break;
	case SYS_lock_cons:
		sys_lock_cons();
		return 0;
		break;
	case SYS_unlock_cons:
		sys_unlock_cons();
		return 0;
		break;
	case SYS_calc_req_frames:
		return sys_calculate_required_frames(a1, a2);
		break;
	case SYS_calc_free_frames:
		return sys_calculate_free_frames();
		break;
	case SYS_calc_modified_frames:
		return sys_calculate_modified_frames();
		break;
	case SYS_calc_notmod_frames:
		return sys_calculate_notmod_frames();
		break;

	case SYS_pf_calc_allocated_pages:
		return sys_pf_calculate_allocated_pages();
		break;
	case SYS_calculate_pages_tobe_removed_ready_exit:
		return sys_calculate_pages_tobe_removed_ready_exit(a1);
		break;
	case SYS_scarce_memory:
		sys_scarce_memory();
		return 0;
		break;
	case SYS_allocate_chunk_in_mem:
		sys_allocate_chunk(a1, (uint32)a2, a3);
		return 0;
		break;

		//======================
	case SYS_allocate_page:
		return __sys_allocate_page((void*)a1, a2);
		break;
	case SYS_map_frame:
		__sys_map_frame(a1, (void*)a2, a3, (void*)a4, a5);
		return 0;
		break;
	case SYS_unmap_frame:
		return __sys_unmap_frame(a1);
		break;

	case SYS_clearFFL:
		sys_clearFFL((const char)a1);
		return 0;
		break;

	case SYS_create_shared_object:
		return (uint32)sys_create_shared_object((char*)a1, a2, a3, (void*)a4);
		break;

	case SYS_get_share_info:
		return sys_get_share_info(a1,(char*)a2, (int*)a3);
		break;

	case SYS_get_shared_object:
		return sys_get_shared_object((int)a1,(void*)a2);
		break;

	case SYS_free_shared_object:
		return sys_delete_shared_object((int)a1, (void *)a2);
		break;

	case SYS_create_env:
		return sys_create_env((char*)a1, (uint32)a2, (uint32)a3, (uint32)a4);
		break;

	case SYS_run_env:
		sys_run_env((int32)a1);
		return 0;
		break;
	case SYS_getenvindex:
		return sys_getenvindex();
		break;
	case SYS_getenvid:
		return sys_getenvid();
		break;
	case SYS_getparentenvid:
		return sys_getparentenvid();
		break;
	case SYS_destroy_env:
		return sys_destroy_env(a1);
		break;
	case SYS_exit_env:
		sys_exit_env();
		return 0;
		break;
	case SYS_move_user_mem:
		sys_move_user_mem(a1, a2, a3);
		return 0;
		break;
	case SYS_rcr2:
		return sys_rcr2();
		break;
	case SYS_bypassPageFault:
		sys_bypassPageFault(a1);
		return 0;

	case SYS_rsttst:
		rsttst();
		return 0;
	case SYS_inctst:
		inctst();
		return 0;
	case SYS_chktst:
		chktst(a1);
		return 0;
	case SYS_gettst:
		return gettst();
	case SYS_testNum:
		tst(a1, a2, a3, (char)a4, a5);
		return 0;

	case SYS_get_heap_strategy:
		return sys_get_heap_strategy();

	case SYS_set_heap_strategy:
		sys_set_uheap_strategy(a1);
		return 0;

	case SYS_check_LRU_lists:
		return sys_check_LRU_lists((uint32*)a1, (uint32*)a2, (int)a3, (int)a4);

	case SYS_check_LRU_lists_free:
		return sys_check_LRU_lists_free((uint32*)a1, (int)a2);

	case SYS_check_WS_list:
		return sys_check_WS_list((uint32*)a1, (int)a2, (uint32)a3, (bool)a4);

	case SYS_utilities:
		sys_utilities((char*)a1, (int)a2);
		return 0;

	case SYS_get_optimal_num_faults:
		return sys_get_optimal_num_faults();

	case SYS_ipc_connect:
		return sys_ipc_connect(a1, (char*)a2, (uint8) a3);

	case SYS_ipc_disconnect:
		return sys_ipc_disconnect(a1);

	case SYS_ipc_create_mailbox:
		return sys_ipc_create_mailbox((char*)a1);

	case SYS_ipc_delete_mailbox:
		return sys_ipc_delete_mailbox(a1);

	case SYS_ipc_send:
		return sys_ipc_send(a1, (struct Message*) a2);

	case SYS_ipc_receive:
		return sys_ipc_receive(a1, (struct Message*) a2);

	case SYS_ipc_broadcast:
		return sys_ipc_broadcast(a1, (struct Message*) a2);

	case NSYSCALLS:
		return 	-E_INVAL;
		break;
	}
	//panic("syscall not implemented");
	return -E_INVAL;
}
