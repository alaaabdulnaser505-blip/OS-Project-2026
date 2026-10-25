/*
 * cpu.h
 *
 *  Created on: Sep 4, 2024
 *      Author: HP
 */

#ifndef KERN_CPU_CPU_H_
#define KERN_CPU_CPU_H_
#include <inc/mmu.h>
#include <inc/x86.h>

// Values of status in struct CpuInfo
enum {
	CPU_UNUSED = 0,
	CPU_STARTED,
	CPU_HALTED,
};

// Per-CPU state
struct CpuInfo {
  unsigned char cpu_id;			// Local APIC ID
  struct Context *scheduler;   	// context_switch() here to enter scheduler
  char* stack;					//Bottom of sched kernel stack for this CPU
								//Its first bottom page is ALWAYS used as a GUARD PAGE
  	  	  	  	  	  	  	  	//(i.e. unmapped)
  struct Taskstate ts;         	// Used by x86 to find stack for interrupt
  struct Segdesc gdt[NSEGS];   	// x86 global descriptor table
  volatile unsigned cpu_status; // The status of the CPU
  int ncli;                    	// Depth of pushcli nesting (for locking).
  int intena;                  	// Were interrupts enabled before pushcli? (for locking)
  struct Env *proc;           	// The process running on this cpu or null
  int scheduler_status ;		// Status of the scheduler at this CPU
};

// Initialized in mpconfig.c
struct CpuInfo cpus[NCPUS];
int ncpu;                    // Total number of CPUs in the system
struct CpuInfo *bootcpu;     // The boot-strap processor (BSP)

struct CpuInfo* mycpu();
int cpunum(void);
struct Env* get_cpu_proc(void);		// get the the current running process on the CPU
void set_cpu_proc(struct Env* p);	// set the process to be run on CPU
void popcli(void);					//enable interrupt on current CPU
void pushcli(void);					//disable interrupt on current CPU

void cpu_init(unsigned char cpuID);
void mp_init(void);
void boot_aps(void);
extern void context_switch(struct Context ** old, struct Context * new); //switch CPU regs & stack between two contexts


#endif /* KERN_CPU_CPU_H_ */
