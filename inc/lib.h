// Main public header file for our user-land support library,
// whose code lives in the lib directory.
// This library is roughly our OS's version of a standard C library,
// and is intended to be linked into all user-mode applications
// (NOT the kernel or boot loader).

#ifndef FOS_INC_LIB_H
#define FOS_INC_LIB_H 1

#include <inc/types.h>
#include <inc/stdio.h>
#include <inc/stdarg.h>
#include <inc/string.h>
#include <inc/queue.h>
#include <inc/error.h>
#include <inc/assert.h>
#include <inc/x86.h>
#include <inc/environment_definitions.h>
#include <inc/memlayout.h>
#include <inc/syscall.h>
#include <inc/uheap.h>
#include <inc/dynamic_allocator.h>
#include <inc/uspinlock.h>

#define USED(x)		(void)(x)
#define RAND(s,e)	((get_virtual_time().low % (e-s) + s))

// libos.c or entry.S
extern volatile char *binaryname;
extern volatile struct Env *myEnv;
extern volatile struct Env envs[NENV];
void	__destroy(void);
void	exit(void);

/* concurrency.c */
void env_sleep(uint32 apprxMilliSeconds);
uint32 busy_wait(uint32 loopMax);
#define CYCLES_PER_MILLISEC 10000

/* panic.c */
void CheckWSArrayWithoutLastIndex(uint32 *expectedPages, int arraySize);

/* File open modes */
#define	O_RDONLY	0x0000		/* open for reading only */
#define	O_WRONLY	0x0001		/* open for writing only */
#define	O_RDWR		0x0002		/* open for reading and writing */
#define	O_ACCMODE	0x0003		/* mask for above modes */

#define	O_CREAT		0x0100		/* create if nonexistent */
#define	O_TRUNC		0x0200		/* truncate to zero length */
#define	O_EXCL		0x0400		/* error if already exists */
#define O_MKDIR		0x0800		/* create directory, not regular file */

#endif	// !FOS_INC_LIB_H
