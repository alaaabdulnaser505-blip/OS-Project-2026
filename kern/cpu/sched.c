#include "sched.h"

#include <inc/assert.h>

#include <kern/proc/user_environment.h>
#include <kern/trap/trap.h>
#include <kern/mem/kheap.h>
#include <kern/mem/memory_manager.h>
#include <kern/tests/utilities.h>
#include <kern/cmd/command_prompt.h>
#include <kern/cpu/cpu.h>
#include <kern/cpu/picirq.h>
#include <kern/cpu/lapic.h>
#include <kern/conc/bigkernellock.h>


uint32 isSchedMethodRR(){return (scheduler_method == SCH_RR);}
uint32 isSchedMethodMLFQ(){return (scheduler_method == SCH_MLFQ); }
uint32 isSchedMethodBSD(){return(scheduler_method == SCH_BSD); }
uint32 isSchedMethodPRIRR(){return(scheduler_method == SCH_PRIRR); }

//===================================================================================//
//============================ SCHEDULER FUNCTIONS ==================================//
//===================================================================================//
static struct Env* (*sched_next[])(void) = {
		[SCH_RR]    fos_scheduler_RR,
		[SCH_MLFQ]  fos_scheduler_MLFQ,
		[SCH_BSD]   fos_scheduler_BSD,
		[SCH_PRIRR]   fos_scheduler_PRIRR,

};

//===================================
// [1] Default Scheduler Initializer:
//===================================
void sched_init()
{
	old_pf_counter = 0;

	sched_init_RR(INIT_QUANTUM_IN_MS);

	init_queue(&ProcessQueues.env_new_queue);
	init_queue(&ProcessQueues.env_exit_queue);

	mycpu()->scheduler_status = SCH_STOPPED;

	/*2024: initialize lock to protect these Qs in MULTI-CORE case only*/
	init_kspinlock(&ProcessQueues.qlock, "process queues lock");
}

//=========================
// [2] Main FOS Scheduler:
//=========================
extern uint32 lapicr(int index);

void
fos_scheduler(void)
{
	//ensure that the scheduler is invoked while interrupt is disabled
	if (read_eflags() & FL_IF)
		panic("fos_scheduler: called while the interrupt is enabled!");

	//cprintf("inside scheduler - timer cnt = %d\n", kclock_read_cnt0());
	struct Env *p;
	struct CpuInfo *c = mycpu();
	c->proc = 0;
	c->scheduler_status = SCH_STARTED;

	//This variable should be set to the next environment to be run (if any)
	struct Env* next_env = NULL;

	int is_any_process = 0;
	acquire_kspinlock(&(ProcessQueues.qlock));  //lock: to protect ready & blocked Qs in multi-CPU
	{
		// Check ready queue(s) looking for process to run.
		do
		{
			//Get next env according to the current scheduler
			next_env = sched_next[scheduler_method]() ;

			if(next_env != NULL)
			{
				cpuclk_pause();
				next_env->nSchedPerCPU[cpunum()]++ ;
				//cprintf_colored(TEXT_cyan, "fos_scheduler: CPU[%d] schedule process %s[%d] - timer cur cnt = %d, timer init cnt = %d\n", c->cpu_id, next_env->prog_name, next_env->env_id, lapic[TCCR], lapic[TICR]);

				// Switch to chosen process. It is the process's job to release qlock
				// and then reacquire it before jumping back to us.
				set_cpu_proc(next_env);
				switchuvm(next_env);

				//Change its status to RUNNING
				next_env->env_status = ENV_RUNNING;

				//Context switch to it
				context_switch(&(c->scheduler), next_env->context);

				//ensure that the qlock is still held after returning from the process
				if(!holding_kspinlock(&ProcessQueues.qlock))
				{
					printcallstack(&ProcessQueues.qlock);
					panic("fos_scheduler(): qlock is either not held or held by another CPU!");
				}

				// Process is done running for now. It should have changed its p->status before coming back.
				//If no process on CPU, switch to the kernel
				assert(get_cpu_proc() == c->proc);
				int status = c->proc->env_status ;
				assert(status != ENV_RUNNING);
				if (status == ENV_READY)
				{
					//OK... will be placed to the correct ready Q in the next iteration
				}
				else
				{
					//cprintf("scheduler: process %d is BLOCKED/EXITED\n", c->proc->env_id);
					switchkvm();
					struct Env* __e__ = c->proc;
					set_cpu_proc(NULL);
				}
			}
		} while(next_env);

		//no ready process is found!
		//check if there's any process in the system?
		is_any_process = 0;
		for (int i = 0; i < NENV; ++i)
		{
			if (envs[i].env_status != ENV_EXIT && envs[i].env_status != ENV_FREE)
			{
				is_any_process = 1;
				break;
			}
		}
	}
	release_kspinlock(&ProcessQueues.qlock);  //release lock: to protect ready & blocked Qs in multi-CPU

	//CPU[..] WILL BE HALTED and release the big kernel lock
	if (is_any_process)
	{
		//cprintf_colored(TEXT_light_red, "\nfos_scheduler: CPU[%d] WILL BE HALTED and release the big kernel lock\n", c->cpu_id);
		// sched_halt never returns
		sched_halt();
	}
	//No more envs... curenv doesn't exist any more! return back to command prompt
	else
	{
		cprintf("[sched] no envs - nothing more to do!\n");
		get_into_prompt();
	}
}

//=============================
// [3] Halt the CPU:
//=============================
// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	// Disable interrupts on this processor to avoid it while setting CPU registers
	cli();
	int i;
	// Mark that no environment is running on this CPU
	set_cpu_proc(NULL);

	//Switch to the KERN VM at this CPU
	switchkvm();

	// Mark that this CPU is in the HALT state, so that when
	// timer interrupts come in, we know we should re-acquire the
	// big kernel lock
	struct CpuInfo *thiscpu = mycpu();
	xchg(&(thiscpu->cpu_status), CPU_HALTED);

	//Restart the clock of this CPU so we can schedule on it again after the clock interrupt
	cpuclk_restart();

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Reset stack pointer to the CPU Scheduler Stack, enable interrupts and then halt.
	asm volatile (
			"movl $0, %%ebp\n"
			"movl %0, %%esp\n"
			"pushl $0\n"
			"pushl $0\n"
			// Uncomment the following line after completing exercise 13
			"sti\n"
			"1:\n"
			"hlt\n"
			"jmp 1b\n"
			: : "a" ((uint32)thiscpu->stack + KERNEL_STACK_SIZE));

	while (1);
}

//=============================
// [4] Initialize RR Scheduler:
//=============================
void sched_init_RR(uint8 quantum)
{
	// Create 1 ready queue for the RR
	num_of_ready_queues = 1;
#if USE_KHEAP
	sched_delete_ready_queues();
	ProcessQueues.env_ready_queues = kmalloc(sizeof(struct Env_Queue));
	quantums = kmalloc(num_of_ready_queues * sizeof(uint8)) ;
#endif
	quantums[0] = quantum;
	//Will be set for each CPU during the FOS initialization
	init_queue(&(ProcessQueues.env_ready_queues[0]));
	//=========================================
	//DON'T CHANGE THESE LINES=================
	mycpu()->scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_RR;
	//=========================================
	//=========================================
}

//===============================
// [4] Initialize MLFQ Scheduler:
//===============================
void sched_init_MLFQ(uint8 numOfLevels, uint8 *quantumOfEachLevel, uint32 starvThresh)
{
	//=========================================
	//DON'T CHANGE THESE LINES=================
	sched_delete_ready_queues();
	//=========================================
	//=========================================

	//TODO: [PROJECT'26.IM#3] CPU SCHEDULING - [2] sched_init_MLFQ
	{
		//Your code is here
		//Comment the following line
		panic("sched_init_MLFQ() is not implemented yet...!!");

	}

	//=========================================
	//DON'T CHANGE THESE LINES=================
	mycpu()->scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_MLFQ;
	//=========================================
	//=========================================
}

//===============================
// [5] Initialize BSD Scheduler:
//===============================
void sched_init_BSD(uint8 numOfLevels, uint8 quantum)
{
	panic("Not implemented yet");

	//=========================================
	//DON'T CHANGE THESE LINES=================
	mycpu()->scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_BSD;
	//=========================================
	//=========================================
}

//======================================
// [6] Initialize PRIORITY RR Scheduler:
//======================================
void sched_init_PRIRR(uint8 numOfPriorities, uint8 quantum, uint32 starvThresh)
{
	panic("Not implemented yet");

	//=========================================
	//DON'T CHANGE THESE LINES=================
	mycpu()->scheduler_status = SCH_STOPPED;
	scheduler_method = SCH_PRIRR;
	//=========================================
	//=========================================
}

//=========================
// [7] RR Scheduler:
//=========================
struct Env* fos_scheduler_RR()
{
	// Implement simple round-robin scheduling.
	// Pick next environment from the ready queue,
	// and switch to such environment if found.
	// It's OK to choose the previously running env if no other env
	// is runnable.
	/*To protect process Qs (or info of current process) in multi-CPU************************/
	if(!holding_kspinlock(&ProcessQueues.qlock))
		panic("fos_scheduler_RR: q.lock is not held by this CPU while it's expected to be.");
	/****************************************************************************************/
	struct Env *next_env = NULL;
	struct Env *cur_env = get_cpu_proc();
	//If the curenv is still exist, then insert it again in the ready queue
	if (cur_env != NULL)
	{
		//cprintf("RR: [%d] with status %d will be added to ready Q", cur_env->env_id, cur_env->env_status);
		enqueue(&(ProcessQueues.env_ready_queues[0]), cur_env);
	}

	//Pick the next environment from the ready queue
	next_env = dequeue(&(ProcessQueues.env_ready_queues[0]));

	//Reset the quantum
	cpuclk_set_counter(quantums[0]);
	return next_env;
}

//=========================
// [8] MLFQ Scheduler:
//=========================
struct Env* fos_scheduler_MLFQ()
{
	//Apply the MLFQ with the specified levels to pick up the next environment
	//Note: the "curenv" (if exist) should be placed in its correct queue
	/*To protect process Qs (or info of current process) in multi-CPU************************/
	if(!holding_kspinlock(&ProcessQueues.qlock))
		panic("fos_scheduler_MLFQ: q.lock is not held by this CPU while it's expected to be.");
	/****************************************************************************************/

	//TODO: [PROJECT'26.IM#3] CPU SCHEDULING - [3] fos_scheduler_MLFQ
	//Your code is here
	//Comment the following line
	panic("fos_scheduler_MLFQ() is not implemented yet...!!");
}

//=========================
// [9] BSD Scheduler:
//=========================
struct Env* fos_scheduler_BSD()
{
	/*To protect process Qs (or info of current process) in multi-CPU************************/
	if(!holding_kspinlock(&ProcessQueues.qlock))
		panic("fos_scheduler_BSD: q.lock is not held by this CPU while it's expected to be.");
	/****************************************************************************************/
	panic("Not implemented yet");
}

//=============================
// [10] PRIORITY RR Scheduler:
//=============================
struct Env* fos_scheduler_PRIRR()
{
	/*To protect process Qs (or info of current process) in multi-CPU************************/
	if(!holding_kspinlock(&ProcessQueues.qlock))
		panic("fos_scheduler_PRIRR: q.lock is not held by this CPU while it's expected to be.");
	/****************************************************************************************/
	panic("fos_scheduler_PRIRR() is not implemented yet...!!");
}

//========================================
// [11] Clock Interrupt Handler
//	  (Automatically Called Every Quantum)
//========================================
void clock_interrupt_handler(struct Trapframe* tf)
{
	if (isSchedMethodMLFQ())
	{
		//TODO: [PROJECT'26.IM#3] CPU SCHEDULING - [4] clock_interrupt_handler
		//Your code is here
		//Comment the following line
		panic("clock_interrupt_handler() is not implemented yet...!!");
	}

	/********DON'T CHANGE THESE LINES***********/
	ticks++ ;
	struct Env* p = get_cpu_proc();
	if (p == NULL)
	{
		lapic_eoi();
		fos_scheduler();
	}
	else
	{
		p->nClocks++ ;
		if(isPageReplacmentAlgorithmLRU(PG_REP_LRU_TIME_APPROX))
		{
			update_WS_time_stamps();
		}
		//cprintf("\n***************\nClock Handler\n***************\n") ;
		lapic_eoi();
		yield();
	}
	/*****************************************/
}

//===================================================================
// [9] Update LRU Timestamp of WS Elements
//	  (Automatically Called Every Quantum in case of LRU Time Approx)
//===================================================================
void update_WS_time_stamps()
{
	panic("update_WS_time_stamps is not implemented yet...!!");
}
