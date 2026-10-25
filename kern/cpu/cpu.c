/*
 * cpu.c
 *
 *  Created on: Sep 4, 2024
 *      Author: HP
 */
#include <kern/cpu/cpu.h>
#include <kern/cpu/sched.h>
#include <kern/cpu/lapic.h>
#include <kern/trap/trap.h>
#include <kern/proc/user_environment.h>
#include <kern/mem/memory_manager.h>
#include <inc/x86.h>
#include <inc/stdio.h>
#include <inc/assert.h>
#include <inc/string.h>
#include <kern/conc/bigkernellock.h>

extern void seg_init(void);
extern void idt_init(void);

//===================================
// 1) RETURN CURRENT RUNNING CPU:
//===================================
// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct CpuInfo* mycpu()
{
	//return &CPUS[0]; //main CPU

	/*this code is for multi-cpu
	 * ref: xv6 OS (x86 ver)
	 */
	int apicid, i;

	if(read_eflags()&FL_IF)
		panic("mycpu is called from CPU ID [%d] with interrupts enabled\n", cpunum());

	apicid = cpunum();

	//If no CPUs are initialized yet, return the main CPU
	if (ncpu == 0)
	{
		return &cpus[0];
	}

	// APIC IDs are not guaranteed to be contiguous. Maybe we should have
	// a reverse map, or reserve a register to store &cpus[i].
	for (i = 0; i < ncpu; ++i) {
		if (cpus[i].cpu_id == apicid)
		{
			return &cpus[i];
		}
	}
	panic("unknown apicid\n");
}


//===================================
// 2) RETURN CURRENT RUNNING PROCESS:
//===================================
// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
// Ref: xv6-x86 OS
struct Env* get_cpu_proc(void)
{
	struct CpuInfo *c;
	struct Env *p;
	pushcli();
	c = mycpu();
	p = c->proc;
	popcli();
	return p;
}
//===================================
// 3) SET PROCESS TO BE RUN ON CPU:
//===================================
// Disable interrupts so that we are not rescheduled
// while setting proc into the cpu structure
// Ref: xv6-x86 OS
void set_cpu_proc(struct Env* p)
{
	struct CpuInfo *c;
	pushcli();
	c = mycpu();
	c->proc = p;
	popcli();
}

//=============================================
// 4) ENABLE/DISABLE INTERRUPTS OF CURRENT CPU:
//=============================================
// Pushcli/popcli are like cli/sti except that they are matched:
// it takes two popcli to undo two pushcli.  Also, if interrupts
// are off, then pushcli, popcli leaves them off.
void pushcli(void)
{
	int eflags = read_eflags();
	cli();
	struct CpuInfo* c = mycpu();
	if(c->ncli == 0)
		c->intena = eflags & FL_IF;
	c->ncli += 1;
}
void popcli(void)
{
	if(read_eflags()&FL_IF)
		panic("popcli - interruptible");
	struct CpuInfo* c = mycpu();
	if(--c->ncli < 0)
		panic("popcli");
	if(c->ncli == 0 && c->intena)
		sti();
}

//===========================
// 5) COMMON CPU SETUP CODE
//===========================
void cpu_init(unsigned char cpuID)
{
	//if this is the main CPU, set #CPUs to ZERO
	if (cpuID == 0) ncpu = 0;

	struct CpuInfo* c = &(cpus[cpuID]);
	int cpuIndx = c->cpu_id;
	c->proc = NULL;
	c->ncli = 0;
	c->intena = read_eflags() & FL_IF ? 1 : 0;


	//cprintf("cpu_init: cpuIndx = %d\n", cpuIndx);

	//c->apicid = ?? ;

	//Initialize the CPU Context to NULL.
	//to be set later to the correct position on the stack during the
	//first switch from scheduler to the first process
	c->scheduler = NULL ;
	c->scheduler_status = SCH_UNINITIALIZED;

	//Initialize its sched stack
	c->stack = (char*)(KERN_STACK_TOP - (cpuIndx+1)*KERNEL_STACK_SIZE);

	//Initialize the TaskState to ZERO.
	//to be initialized later in init.c
	memset(&(c->ts), 0, sizeof(c->ts)) ;

	/*2025*///Moved to mp_main
	/*
  //initialize GDT & set it to this CPU
  seg_init();

  //initialize IDT
  idt_init();       // load idt register

  //Indicate it's started
  xchg(&(c->cpu_status), CPU_STARTED); // tell startothers() we're up
	 */
	//scheduler();     // start running processes
}


// While boot_aps is booting a given CPU, it communicates the per-core
// stack pointer that should be loaded by mpentry.S to that CPU in
// this variable.
extern void *mpentry_kstack;

extern uint32 entry_pgdir[NPDENTRIES];
extern uint32* tmp_ptr_page_directory ;

// Start the non-boot (AP) processors.
void boot_aps(void)
{
	/*2025 EL7 :) ============================================================================*/
	/*TEMPORARILY SET THE tmp_ptr_page_directory AT 1ST 4 MB OF KERNEL TO ptr_page_directory
	 * IN ORDER TO USE IT WHILE LOADING THE CR3 USING THE entry_pgdir (WHICH MAP ONLY THE
	 * 1ST 4 MB T BOTH 0 AND KERNEL_BASE)
	 */
	tmp_ptr_page_directory = ptr_page_directory;
	tmp_ptr_page_directory[0] = ptr_page_directory[PDX(KERNEL_BASE)];
	//cprintf("boot_aps(): tmp_phys_page_directory @%x = %x\n", &tmp_ptr_page_directory, tmp_ptr_page_directory);
	/*EL7 :) ================================================================================*/
	extern unsigned char mpentry_start[], mpentry_end[];
	void *code;
	struct CpuInfo *c;

	// Write entry code to unused memory at MPENTRY_PADDR
	code = STATIC_KERNEL_VIRTUAL_ADDRESS(MPENTRY_PADDR);

	memmove(code, mpentry_start, mpentry_end - mpentry_start);

	// Boot each AP one at a time
	for (c = cpus; c < cpus + ncpu; c++)
	{
		if (c == cpus + cpunum())  // We've started already.
			continue;

		//Initialize its CpuInfo
		cpu_init(c->cpu_id) ;

		// Tell mpentry.S what stack to use
		//mpentry_kstack = c->stack + KERN_STACK_SIZE;
		mpentry_kstack = ptr_stack_top - (c->cpu_id) * KERNEL_STACK_SIZE;
		//cprintf("boot_aps(): mpentry_kstack of CPU#%d = %x\n", (c->cpu_id), mpentry_kstack);
//		cprintf("boot_aps: entry_pgdir VA = %x\n", entry_pgdir);
//		cprintf("boot_aps: esp = %x\n", read_esp());
//		cprintf("boot_aps: cr3 = %x\n", rcr3());
//		cprintf("boot_aps: ptr_page_directory = %x\n", ptr_page_directory);
//
//		cprintf("entry[0] of tmp dir = %x\n", tmp_ptr_page_directory[0]);
//		cprintf("entry[%d] of tmp dir = %x\n", PDX(KERNEL_BASE), tmp_ptr_page_directory[PDX(KERNEL_BASE)]);

		// Start the CPU at mpentry_start
		lapic_startap(c->cpu_id, STATIC_KERNEL_PHYSICAL_ADDRESS(code));

		// Wait for the CPU to finish some basic setup in mp_main()
		while(c->cpu_status != CPU_STARTED)
			;
	}
	/*2025 EL7 :) ============================================================================*/
	/*RESTORE THE tmp_ptr_page_directory BY CLEARING THE 4MB-MAPPING AT VA 0
	 */
	tmp_ptr_page_directory[0] = 0;
	/*EL7 :) ================================================================================*/
}

// Setup code for APs
void
mp_main(void)
{
	//We are in high EIP now, safe to switch to ptr_page_directory BUT
	//using the tmp_ptr_page_directory since it's located in the first 4MB after KERNEL_BASE
	//so, it can be accessed using the current "entry_pgdir" which maps only the 1st 4MB
	lcr3(STATIC_KERNEL_PHYSICAL_ADDRESS(tmp_ptr_page_directory));

	cprintf_colored(TEXT_yellow|TEXTBG_cyan, "*********** WELCOME FROM mp_main() EL7AMDULELLAH :) :) ************\n");
//	char* ch = (char*)0xF00B8000;
//	char curChar = 'A';
//	while(ch < (char*)(0xF00B8000 + 0x200))
//		*ch++ = curChar;

	cprintf(">>>>>>> SMP: CPU %d starting...\n", cpunum());

	lapic_init();

	//initialize GDT & set it to this CPU
	seg_init(); env_init_percpu();

	//Initialize and load the per-CPU TSS and IDT
	trap_init_percpu();

	xchg(&(mycpu()->cpu_status), CPU_STARTED); // tell boot_aps() we're up

	// Now that we have finished some basic setup, call fos_scheduler()
	// to start running processes on this CPU.  But make sure that
	// only one CPU can enter the scheduler at a time!
	//
	// Your code here:
	lock_kernel();
	fos_scheduler();
}

