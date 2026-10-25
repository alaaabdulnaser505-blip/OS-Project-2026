#ifndef FOS_INC_SYSCALL_H
#define FOS_INC_SYSCALL_H
#include <inc/semaphore.h>
#include <inc/ipc.h>

/* system call numbers */
enum
{
	SYS_cputs = 0,
	SYS_cputc,
	SYS_cgetc,
	SYS_lock_cons,
	SYS_unlock_cons,
	SYS_getenvid,
	SYS_getenvindex,
	SYS_getparentenvid,
	SYS_allocate_page,
	SYS_map_frame,
	SYS_unmap_frame,
	SYS_calc_req_frames,
	SYS_calc_free_frames,
	SYS_calc_modified_frames,
	SYS_calc_notmod_frames,
	SYS_pf_calc_allocated_pages,
	SYS_calculate_pages_tobe_removed_ready_exit,
	SYS_scarce_memory,
	SYS_allocate_chunk_in_mem,
	SYS_move_user_mem,
	SYS_clearFFL,
	SYS_create_shared_object,
	SYS_get_share_info,
	SYS_get_shared_object,
	SYS_free_shared_object,
	SYS_create_env,
	SYS_run_env,
	SYS_destroy_env,
	SYS_exit_env,
	SYS_get_virtual_time,
	SYS_rcr2,
	SYS_bypassPageFault,
	SYS_testNum,
	SYS_rsttst,
	SYS_chktst,
	SYS_inctst,
	SYS_gettst,
	SYS_get_heap_strategy,
	SYS_set_heap_strategy,
	SYS_check_LRU_lists,
	SYS_check_LRU_lists_free,
	SYS_check_WS_list,
	SYS_utilities,
	SYS_get_optimal_num_faults,
	SYS_ipc_connect,
	SYS_ipc_disconnect,
	SYS_ipc_create_mailbox,
	SYS_ipc_delete_mailbox,
	SYS_ipc_send,
	SYS_ipc_receive,
	SYS_ipc_broadcast,
	SYS_free_user_mem,
	SYS_allocate_user_mem,
	NSYSCALLS
};

inline uint32 syscall(uint32 num, uint32 a1, uint32 a2, uint32 a3, uint32 a4, uint32 a5);

//Cons
void	sys_cputs(const char *string, uint32 len, uint8 printProgName, int color);
int		sys_cgetc(void);
void 	sys_cputc(const char c);
void 	sys_lock_cons(void);
void 	sys_unlock_cons(void);

//User Env
int32	sys_getenvindex(void);
int32	sys_getenvid(void);
int32	sys_getparentenvid(void);
void 	sys_exit_env();
//2016. Edited @ 2018 @2020 to add secondlist size for LRU list approximation
int 	sys_create_env(char* programName, unsigned int page_WS_size,unsigned int LRU_second_list_size,unsigned int percent_WS_pages_to_remove);
int		sys_destroy_env(int32 envId);
void	sys_run_env(int32 envId);

//Memory
int 	__sys_allocate_page(void *va, int perm);
int 	__sys_map_frame(int32 srcenv, void *srcva, int32 dstenv, void *dstva, int perm);
int 	__sys_unmap_frame(uint32 va);
uint32 	sys_calculate_required_frames(uint32 start_virtual_address, uint32 size);
uint32 	sys_calculate_free_frames();
uint32 	sys_calculate_modified_frames();
uint32 	sys_calculate_notmod_frames();
int		sys_calculate_pages_tobe_removed_ready_exit(uint32 WS_or_MEMORY_flag);
void 	sys_scarce_memory();

//Dynamic Allocator
void* 	sys_sbrk(int numOfPages);
//******************

//User Heap
void 	sys_free_user_mem(uint32 startVA, uint32 size);
void	sys_allocate_user_mem(uint32 startVA, uint32 size);
void	sys_allocate_chunk(uint32 startVA, uint32 size, uint32 perms);
void 	sys_move_user_mem(uint32 src_virtual_address, uint32 dst_virtual_address, uint32 size);
uint32 	sys_get_uheap_strategy();
void 	sys_set_uheap_strategy(uint32 heapStrategy);

//Page File
int 	sys_pf_calculate_allocated_pages(void);

//Semaphores

//Sharing
int sys_create_shared_object(char* shareName, uint32 size, uint8 isWritable, void* startVA);
int sys_get_share_info(uint32 ownerID, char* shareName, int *size);
int sys_get_shared_object(int shareID, void* startVA );
int sys_delete_shared_object(int shareID, void *startVA);

//etc...
uint32	sys_rcr2();
void 	sys_bypassPageFault(uint8 instrLength);
void 	rsttst();
void 	inctst();
uint32 	gettst();
void 	chktst(uint32 n);
void 	tst(uint32 n, uint32 v1, uint32 v2, char c, int inv);
//2020
int 	sys_check_LRU_lists(uint32* active_list_content, uint32* second_list_content, int actual_active_list_size, int actual_second_list_size);
int 	sys_check_LRU_lists_free(uint32* list_content, int list_size);
//2023
int 	sys_check_WS_list(uint32* WS_list_content, int actual_WS_list_size, uint32 last_WS_element_content, bool chk_in_order);
//2024
void 	sys_utilities(char* utilityName, int value);
//2025
int 	sys_get_optimal_num_faults();

#endif /* !FOS_INC_SYSCALL_H */
