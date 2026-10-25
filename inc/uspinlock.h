// Mutual exclusion lock.
/*originally taken from xv6-x86 OS*/
#ifndef INC_USPINLOCK_H_
#define INC_USPINLOCK_H_

struct uspinlock {
  uint32 locked;       	// Is the lock held?
  char name[NAMELEN];	// Name of lock.
};
void init_uspinlock(struct uspinlock *lk, char *name, bool isOpened);
void acquire_uspinlock(struct uspinlock *lk);
void release_uspinlock(struct uspinlock *lk);
#endif /*INC_USPINLOCK_H_*/
