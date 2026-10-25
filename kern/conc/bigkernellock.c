// Mutual exclusion spin locks.
/*originally taken from xv6-x86 OS
 * USED ONLY FOR PROTECTION IN MULTI-CORE
 * Not designed for protection in a single core
 * */
#include "inc/types.h"
#include "inc/x86.h"
#include "inc/memlayout.h"
#include "inc/mmu.h"
#include "inc/string.h"
#include "inc/environment_definitions.h"
#include "inc/assert.h"
#include "bigkernellock.h"
#include "../cpu/cpu.h"
#include "../proc/user_environment.h"
#include "kern/trap/kdebug.h"

/************************************************************************************/
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
//TODO: [PROJECT'26.ALL] PROTECTION - [0] GIVEN BIG KERN LOCK

// The big kernel lock
struct __spinlock__ kernel_lock = {
#ifdef DEBUG_SPINLOCK
	.name = "kernel_lock"
#endif
};

#ifdef DEBUG_SPINLOCK
// Record the current call stack in pcs[] by following the %ebp chain.
static void
get_caller_pcs(uint32 pcs[])
{
	uint32 *ebp;
	int i;

	ebp = (uint32 *)read_ebp();
	for (i = 0; i < 10; i++){
		if (ebp == 0 || ebp < (uint32 *)USER_LIMIT)
			break;
		pcs[i] = ebp[1];          // saved %eip
		ebp = (uint32 *)ebp[0]; // saved %ebp
	}
	for (; i < 10; i++)
		pcs[i] = 0;
}

// Check whether this CPU is holding the lock.
static int
holding(struct __spinlock__ *lock)
{
	return lock->locked && lock->cpu == mycpu();
}
#endif

void
__spin_initlock(struct __spinlock__ *lk, char *name)
{
	lk->locked = 0;
#ifdef DEBUG_SPINLOCK
	lk->name = name;
	lk->cpu = 0;
#endif
}

// Acquire the lock.
// Loops (spins) until the lock is acquired.
// Holding a lock for a long time may cause
// other CPUs to waste time spinning to acquire it.
void
spin_lock(struct __spinlock__ *lk)
{
#ifdef DEBUG_SPINLOCK
	if (holding(lk))
		panic("CPU %d cannot acquire %s: already holding", cpunum(), lk->name);
#endif

	//cprintf(">>>>>>>>>>>CPU[%d] try to acquire big kernel lock\n", cpunum());

	// The xchg is atomic.
	// It also serializes, so that reads after acquire are not
	// reordered before it.
	while (xchg(&lk->locked, 1) != 0)
		asm volatile ("pause");

	//cprintf(">>>>>>>>>>>big kernel lock is ACQUIRED  by CPU[%d]\n", cpunum());

	// Record info about lock acquisition for debugging.
#ifdef DEBUG_SPINLOCK
	lk->cpu = mycpu();
	get_caller_pcs(lk->pcs);
#endif
}

// Release the lock.
void
spin_unlock(struct __spinlock__ *lk)
{
#ifdef DEBUG_SPINLOCK
	if (!holding(lk)) {
//		int i;
//		uint32 pcs[10];
//		// Nab the acquiring EIP chain before it gets released
//		memmove(pcs, lk->pcs, sizeof pcs);
//		cprintf("CPU %d cannot release %s: held by CPU %d\nAcquired at:",
//			cpunum(), lk->name, lk->cpu->cpu_id);
//		for (i = 0; i < 10 && pcs[i]; i++) {
//			struct Eipdebuginfo info;
//			if (debuginfo_eip(pcs[i], &info) >= 0)
//				cprintf("  %08x %s:%d: %.*s+%x\n", pcs[i],
//					info.eip_file, info.eip_line,
//					info.eip_fn_namelen, info.eip_fn_name,
//					pcs[i] - info.eip_fn_addr);
//			else
//				cprintf("  %08x\n", pcs[i]);
//		}
		panic("spin_unlock");
	}

	lk->pcs[0] = 0;
	lk->cpu = 0;
#endif

	// The xchg instruction is atomic (i.e. uses the "lock" prefix) with
	// respect to any other instruction which references the same memory.
	// x86 CPUs will not reorder loads/stores across locked instructions
	// (vol 3, 8.2.2). Because xchg() is implemented using asm volatile,
	// gcc will not reorder C statements across the xchg.
	//cprintf(">>>>>>>>>>>big kernel lock WILL BE RELEASED by CPU[%d]\n", cpunum());
	xchg(&lk->locked, 0);
}
