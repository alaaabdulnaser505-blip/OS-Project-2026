/* See COPYRIGHT for copyright information. */

#ifndef FOS_INC_ENV_H
#define FOS_INC_ENV_H

#include <inc/types.h>
#include <inc/queue.h>
#include <inc/trap.h>
#include <inc/memlayout.h>
#include <inc/x86.h>
#include <inc/fixed_point.h>
#include <inc/ipc.h>
#include <kern/conc/kspinlock.h>

// An environment ID 'envid_t' has three parts:
//
// +1+---------------21-----------------+--------10--------+
// |0|          Uniqueifier             |   Environment    |
// | |                                  |      Index       |
// +------------------------------------+------------------+
//                                       \--- ENVX(eid) --/
//
// The environment index ENVX(eid) equals the environment's offset in the
// 'envs[]' array.  The uniqueifier distinguishes environments that were
// created at different times, but share the same environment index.
//
// All real environments are greater than 0 (so the sign bit is zero).
// envid_ts less than 0 signify errors.

//Sizes of working sets & LRU 2nd list [if NO KERNEL HEAP]
#define __TWS_MAX_SIZE 	50
#define __PWS_MAX_SIZE 	5000
//2020
#define __LRU_SNDLST_SIZE 500
//====================================
unsigned int _ModifiedBufferLength;

//2017: Max length of program name
#define PROGNAMELEN 64

//2018 Percentage of the pages to be removed from the WS [either for scarce RAM or Full WS]
#define DEFAULT_PERCENT_OF_PAGE_WS_TO_REMOVE	10	// 10% of the loaded pages is required to be removed

// Values of env_status in struct Env
#define ENV_FREE		0
#define ENV_READY		1
#define ENV_RUNNING		2
#define ENV_BLOCKED		3
#define ENV_NEW			4
#define ENV_EXIT		5
#define ENV_KILLED		6
#define ENV_UNKNOWN		7

LIST_HEAD(Env_Queue, Env);		// Declares 'struct Env_Queue'
LIST_HEAD(Env_list, Env);		// Declares 'struct Env_list'

uint32 old_pf_counter;
//uint32 mydblchk;

/******************************************************************************/
//TODO: [PROJECT'26.GM#3] FAULT HANDLER I - [0] GIVEN DATA
//TODO: [PROJECT'26.IM#1] FAULT HANDLER II - [0] GIVEN DATA
struct WorkingSetElement {
	unsigned int virtual_address;
	uint8 empty;
	//2012
	unsigned int time_stamp ;
	//2021
	unsigned int sweeps_counter;
	//2020
	LIST_ENTRY(WorkingSetElement) prev_next_info;	// list link pointers
};
//2020
LIST_HEAD(WS_List, WorkingSetElement);		// Declares 'struct WS_list'
/******************************************************************************/

/******************************************************************************/
//TODO: [PROJECT'26.IM#6] SHARED MEMORY - [0] GIVEN DATA
#define SHARE_MAX_HANDLES 	32
//ShareHandle: object name, ID
struct ShareHandle
{
	//share name
	char name[NAMELEN];
	//Unique ID for this Share object
	int32 ID ;
	// list link pointers
	LIST_ENTRY(ShareHandle) prev_next_info;
} __attribute__((packed));
//List of handlers for all shared objects
LIST_HEAD(ShareHandle_List, ShareHandle);		// Declares 'struct ShareHandler_List'
/******************************************************************************/

/******************************************************************************/
//TODO: [PROJECT'26.IM#5] IPC#2 - [0] GIVEN DATA
/*Mailbox Handle Data Structure*/
struct MailboxHandle
{
	uint32 mailboxVA;
	uint8 connectionType;
	/*linked list links*/
	LIST_ENTRY(MailboxHandle) prev_next_info;
} __attribute__((packed));

//List of handlers for all message boxes
LIST_HEAD(MailboxHandle_List, MailboxHandle);	// Declares 'struct MailboxHandle_List'
/******************************************************************************/

/*2025*/
struct PageRefElement {
	unsigned int virtual_address;
	LIST_ENTRY(PageRefElement) prev_next_info;	// list link pointers
};
LIST_HEAD(PageRef_List, PageRefElement);		// Declares 'struct PageRef_List'
//======================================================================

//2024 (ref: xv6 OS - x86 version)
// Saved registers for kernel context switches.
// Don't need to save all the segment registers (%cs, etc),
// because they are constant across kernel contexts.
// Don't need to save %eax, %ecx, %edx, because the
// x86 convention is that the caller has saved them.
// Contexts are stored at the bottom of the stack they
// describe; the stack pointer is the address of the context.
// The layout of the context matches the layout of the stack in swtch.S
// at the "Switch stacks" comment. Switch doesn't save eip explicitly,
// but it is on the stack and initialize_environment() manipulates it.
struct Context {
  uint32 edi;
  uint32 esi;
  uint32 edx;
  uint32 ecx;
  uint32 ebx;
  uint32 eax;
  uint32 ebp;
  uint32 eip;
};
struct Env {
	//==================
	/*GENERAL INFO...*/
	//==================
	int32 env_id;				// Unique environment identifier
	int32 env_parent_id;		// env_id of this env's parent
	unsigned env_status;		// Status of the environment
	int priority;				// Current priority
	char prog_name[PROGNAMELEN];// Program name (to print it via USER.cprintf in multitasking)
	LIST_ENTRY(Env) prev_next_info;	// Free list link pointers

	//================
	/*CPU THREAD...*/
	//================
	struct Trapframe *env_tf;// Saved registers during the trap (at the top of the user kernel stack)
	struct Context *context; // Saved registers for context switching (env <--> scheduler) (below the trap frame at the user kernel stack)
	void* channel;			 // Address of the channel that it's blocked (sleep) on it

	//================
	/*ADDRESS SPACE*/
	//================
	uint32 *env_page_directory;	// Kernel virtual address of page dir
	uint32 env_cr3;				// Physical address of page dir
	uint32 initNumStackPages ;	// Initial number of allocated stack pages
	char* kstack;				//Bottom of kernel stack for this process
								//(to be dynamically allocated during the process creation)
								//Its first page is ALWAYS used as a GUARD PAGE (i.e. unmapped)

	//for page file management
	uint32* disk_env_pgdir;
	//2016
	unsigned int disk_env_pgdir_PA;

	//for table file management
	uint32* disk_env_tabledir;
	//2016
	unsigned int disk_env_tabledir_PA;

	//================
	/*WORKING SET*/
	//================
	/******************************************************************************/
	//TODO: [PROJECT'26.GM#3] FAULT HANDLER I - [0] GIVEN DATA
	//TODO: [PROJECT'26.IM#1] FAULT HANDLER II - [0] GIVEN DATA
	//page working set management
#if USE_KHEAP
	struct WS_List page_WS_active_list ;			//Active list of WS elements
	struct WS_List page_WS_second_list ;			//Second List that should work as LRU (for LRU Approx)
	unsigned int page_WS_max_size;					//Max allowed size of all WS Lists
	unsigned int page_2nd_WS_size ;					//Max allowed size of SecondList
	struct WorkingSetElement* page_last_WS_element;	//ptr to WS element after the last inserted one
	struct PageRef_List referenceStreamList;		//List of page references stream to be used for OPTIMAL replacement strategy
	uint32 *prepagedVAs;							//Initial virtual addresses after fetching the process into RAM
	uint32 numOfPrepagedVAs;						//Number of prepaged VAs
#else
	struct WorkingSetElement ptr_pageWorkingSet[__PWS_MAX_SIZE];
	//uint32 page_last_WS_index;
	unsigned int page_WS_max_size;					//Max allowed size of all WS Lists
	struct WS_List PageWorkingSetList ;				//LRU Approx: List of available WS elements
#endif
	/******************************************************************************/
	//table working set management
	struct WorkingSetElement __ptr_tws[__TWS_MAX_SIZE];
	uint32 table_last_WS_index;
	//2016
	struct WorkingSetElement* __uptr_pws;

	//Percentage of WS pages to be removed [either for scarce RAM or Full WS]
	unsigned int percentage_of_WS_pages_to_be_removed;
	int isExpandedBefore;

	//================
	/*SHARED MEMORY*/
	//================
	/******************************************************************************/
	//TODO: [PROJECT'26.IM#6] SHARED MEMORY - [0] GIVEN DATA
	//Storage array (and corresponding free list) of objects for all share handles for this process
	struct ShareHandle __shs__[SHARE_MAX_HANDLES];
	struct ShareHandle_List __fshl__;//List of all free share handles
	struct
	{
		//List of all share handles created by this process
		struct ShareHandle_List share_handles_list;
		//Lock to protect the list
		struct kspinlock handleslock;
	}shareHandles;
	/******************************************************************************/
	//==================
	/*IPC...*/
	//==================
	/******************************************************************************/
	//TODO: [PROJECT'26.IM#4] IPC#1 - [0] GIVEN DATA
	struct Mailbox *myMailBox;

	//TODO: [PROJECT'26.IM#5] IPC#2 - [0] GIVEN DATA
	//Storage array (and corresponding free list) of objects
	//for all mailbox handles for this process
	struct MailboxHandle __mbhs__[MB_MAX_HANDLES];
	struct MailboxHandle_List __fmbhl__;
	struct {
		//List of all mailbox handlers created/registered by this process
		struct MailboxHandle_List mailbox_handles_list;
		//Lock to protect the list
		struct kspinlock handleslock;
	}mailboxHandles;
	/******************************************************************************/

	//==================
	/*CPU...*/
	//==================

	//================
	/*STATISTICS...*/
	//================
	uint32 nClocks ;			// Num of clocks taken during the execution
	uint32 nSchedPerCPU[NCPUS];	// Num of times environment has been scheduled on each CPU
	uint32 nSysCalls;			// Num of times environment invoke system calls
	uint32 pageFaultsCounter;	// Num of page faults occur during the execution
	uint32 nPageIn, nPageOut, nNewPageAdded;
	uint32 tableFaultsCounter;
	uint32 freeingFullWSCounter;
	uint32 freeingScarceMemCounter;
	uint32 nModifiedPages;
	uint32 nNotModifiedPages;

};

#define PRIORITY_LOW    		1
#define PRIORITY_BELOWNORMAL    2
#define PRIORITY_NORMAL		    3
#define PRIORITY_ABOVENORMAL    4
#define PRIORITY_HIGH		    5

//#define NENV			(1 << LOG2NENV)
#define NENV			( (PTSIZE/4) / sizeof(struct Env) )
/*2022: UPDATED*/
#define NEARPOW2NENV	(nearest_pow2_ceil(NENV))
#define LOG2NENV		(log2_ceil(NENV))
#define ENVGENSHIFT		LOG2NENV	// >= LOGNENV
#define ENVX(envid)		((envid) & (NEARPOW2NENV - 1))

#endif // !FOS_INC_ENV_H
