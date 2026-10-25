#ifndef FOS_SHARED_MEMORY_MANAGER_H
#define FOS_SHARED_MEMORY_MANAGER_H
#ifndef FOS_KERNEL
# error "This is a FOS kernel header; user programs should not #include it"
#endif
#include <inc/environment_definitions.h>
#include <kern/conc/kspinlock.h>
#include <kern/conc/sleeplock.h>

/******************************************************************************/
//TODO: [PROJECT'26.IM#6] SHARED MEMORY - [0] GIVEN DATA
struct Share
{
	/*Global Info*/
	//ID of the owner environment
	int32 ownerID ;
	//share name
	char name[NAMELEN];
	//share size
	int size;
	//Unique ID for this Share object
	//Should be set to the VA of the created object
	//after masking the most significant bit (to make it +ve)
	int32 ID ;
	/*OS Info*/
	//references, number of envs looking at this shared mem object
	uint32 references;
	//sharing permissions (0: ReadOnly, 1:Writable)
	uint8 isWritable;
	//to store frames to be shared
	struct FrameInfo** framesStorage;
	//Pointer to the corresponding handle in the env
	struct ShareHandle* handle;
	// list link pointers
	LIST_ENTRY(Share) prev_next_info;

} __attribute__((packed));

//List of all shared objects
LIST_HEAD(Share_List, Share);		// Declares 'struct Share_List'

#if USE_KHEAP == 0
//max number of shared objects
#define MAX_SHARES 100
struct Share shares[MAX_SHARES] ;
#else
struct
{
	struct Share_List shares_list ;	//List of all share variables created by any process
	struct kspinlock shareslock;		//Use it to protect the shares_list in the kernel
	//struct sleeplock sharessleeplock;	//Use it to protect the shares_list in the kernel
}AllShares;
void sharing_init();
#endif
/******************************************************************************/

struct ShareHandle* allocate_share_handle(struct Env* e);
void free_share_handle(struct Env* e, struct ShareHandle* shareHandle);

int create_shared_object(int32 ownerID, char* shareName, uint32 size, uint8 isWritable, void* virtual_address);
int get_share_info(int32 ownerID, char* shareName, int *size);
int get_shared_object(int shareID, void* virtual_address);
int delete_shared_object(struct Env* e, int shareID, void *startVA);

#endif /* FOS_SHARED_MEMORY_MANAGER_H */
