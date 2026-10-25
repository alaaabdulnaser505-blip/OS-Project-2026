#include <inc/memlayout.h>
#include "shared_memory_manager.h"

#include <inc/mmu.h>
#include <inc/error.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/queue.h>
#include <inc/syscall.h>
#include <inc/environment_definitions.h>
#include <kern/cpu/cpu.h>
#include <kern/proc/user_environment.h>
#include "kheap.h"
#include "memory_manager.h"

void free_share(struct Share* ptrShare);
struct Share* alloc_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable);

/************************************************************************************/
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
//TODO: [PROJECT'26.IM#6] SHARED MEMORY - [0] GIVEN FUNCTIONS
//===========================
// [1] INITIALIZE SHARES:
//===========================
//Initialize the list and the corresponding lock
void sharing_init()
{
#if USE_KHEAP
	LIST_INIT(&AllShares.shares_list) ;
	init_kspinlock(&AllShares.shareslock, "shares lock");
	//init_sleeplock(&AllShares.sharessleeplock, "shares sleep lock");
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}
//===========================
// [2] Allocate ShareHandle:
//===========================
//Allocate ShareHandle object from the local storage of the given env
//Return:
//	a) if found: new ShareHandle object
//	b) else: NULL
struct ShareHandle* allocate_share_handle(struct Env* e)
{
	struct ShareHandle* handle = LIST_FIRST(&(e->__fshl__));
	if (handle == NULL) return NULL;
	LIST_REMOVE(&(e->__fshl__), handle);
	return handle;
}
//===========================
// [3] Free ShareHandle:
//===========================
//Free ShareHandle object by adding it to the local storage of the given env
void free_share_handle(struct Env* e, struct ShareHandle* shareHandle)
{
	if (shareHandle == NULL) return ;
	memset(shareHandle, 0, sizeof(struct ShareHandle));
	LIST_INSERT_HEAD(&(e->__fshl__), shareHandle);
}
//=========================
// [4] Find Share Object:
//=========================
//Search for the given shared object in the "shareHandles" of the owner env
//Return:
//	a) if found: ID of the shared object
//	b) else: E_SHARED_MEM_NOT_EXISTS
int find_share(int32 ownerID, char* name)
{
#if USE_KHEAP
	struct Env* ownerEnv;
	envid2env(ownerID, &ownerEnv, 0);

	int ret = E_SHARED_MEM_NOT_EXISTS;
	struct ShareHandle* shrHandle = NULL;
	acquire_kspinlock(&(ownerEnv->shareHandles.handleslock));
	{
		LIST_FOREACH(shrHandle, &(ownerEnv->shareHandles.share_handles_list), ShareHandle)
		{
			//cprintf("shared var name = %s compared with %s\n", name, shr->name);
			if(strcmp(name, shrHandle->name)==0)
			{
				//cprintf("%s found\n", name);
				ret = shrHandle->ID;
				break;
			}
		}
	}
	release_kspinlock(&(ownerEnv->shareHandles.handleslock));
	return ret;
#else
	panic("not handled when KERN HEAP is disabled");
#endif
}
/************************************************************************************/


//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=====================================
// [1] Alloc & Initialize Share Object:
//=====================================
//Allocates a new shared object and initialize its member
//It dynamically creates the "framesStorage"
//Return: allocatedObject (pointer to struct Share) passed by reference
struct Share* alloc_share(int32 ownerID, char* shareName, uint32 size, uint8 isWritable)
{
	//TODO: [PROJECT'26.IM#6] SHARED MEMORY - [1] alloc_share
	//Your code is here
	//Comment the following line
	panic("alloc_share() is not implemented yet...!!");
}


//=========================
// [2] Create Share Object:
//=========================
int create_shared_object(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address)
{
	//TODO: [PROJECT'26.IM#6] SHARED MEMORY - [4] create_shared_object
	//Your code is here
	//Comment the following line
	panic("create_shared_object() is not implemented yet...!!");

	struct Env* myenv = get_cpu_proc(); //The calling environment
}

//==============================
// [3] Get Info of Share Object:
//==============================
int get_share_info(int32 ownerID, char* shareName, int *size)
{
	//TODO: [PROJECT'26.IM#6] SHARED MEMORY - [2] get_share_info
	//Your code is here
	//Comment the following line
	panic("get_share_info() is not implemented yet...!!");
}

//======================
// [4] Get Share Object:
//======================
int get_shared_object(int shareID, void* virtual_address)
{
	//TODO: [PROJECT'26.IM#6] SHARED MEMORY - [6] get_shared_object
	//Your code is here
	//Comment the following line
	panic("get_shared_object() is not implemented yet...!!");

	struct Env* myenv = get_cpu_proc(); //The calling environment
}

//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//
//=========================
// [1] Delete Share Object:
//=========================
//delete the given shared object from the "shares_list"
//it should free its framesStorage and the share object itself
void free_share(struct Share* ptrShare)
{
	//TODO: [PROJECT'26.BONUS#4] EXIT #2 - [4] free_share
	//Your code is here
	//Comment the following line
	panic("free_share() is not implemented yet...!!");
}

//=========================
// [2] Free Share Object:
//=========================
int delete_shared_object(struct Env* e, int shareID, void *startVA)
{
	//TODO: [PROJECT'26.BONUS#4] EXIT #2 - [5] delete_shared_object
	//Your code is here
	//Comment the following line
	panic("delete_shared_object() is not implemented yet...!!");
}
