// Mutual exclusion lock.
/*originally taken from xv6-x86 OS
 * USED ONLY FOR PROTECTION IN MULTI-CORE
 * Not designed for protection in a single core
 * */
#ifndef KERN_CONC_BIGKERNELLOCK_H_
#define KERN_CONC_BIGKERNELLOCK_H_

// Comment this to disable spinlock debugging
//#define DEBUG_SPINLOCK

// Mutual exclusion lock.
struct __spinlock__ {
	unsigned locked;       // Is the lock held?

#ifdef DEBUG_SPINLOCK
	// For debugging:
	char *name;            // Name of lock.
	struct CpuInfo *cpu;   // The CPU holding the lock.
	uint32 pcs[10];     // The call stack (an array of program counters)
	                       // that locked the lock.
#endif
};

void __spin_initlock(struct __spinlock__ *lk, char *name);
void spin_lock(struct __spinlock__ *lk);
void spin_unlock(struct __spinlock__ *lk);

#define spin_initlock(lock)   __spin_initlock(lock, #lock)

extern struct __spinlock__ kernel_lock;

static inline void lock_kernel(void)
{
	spin_lock(&kernel_lock);
}

static inline void unlock_kernel(void)
{
	spin_unlock(&kernel_lock);

	// Normally we wouldn't need to do this, but QEMU only runs
	// one CPU at a time and has a long time-slice.  Without the
	// pause, this CPU is likely to reacquire the lock before
	// another CPU has even been given a chance to acquire it.
	asm volatile("pause");
}


#endif /*KERN_CONC_BIGKERNELLOCK_H_*/
