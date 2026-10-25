/* See COPYRIGHT for copyright information. */

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/assert.h>
#include <inc/memlayout.h>
#include <inc/timerreg.h>
#include <kern/cons/console.h>
#include <kern/proc/user_environment.h>
#include <kern/trap/trap.h>
#include <kern/trap/fault_handler.h>
#include <inc/dynamic_allocator.h>
#include "kern/cmd/command_prompt.h"
#include "kern/cmd/commands.h"
#include <kern/cpu/kclock.h>
#include <kern/cpu/cpu.h>
#include <kern/cpu/sched.h>
#include <kern/cpu/picirq.h>
#include <kern/cpu/lapic.h>
#include <kern/mem/boot_memory_manager.h>
#include <kern/mem/kheap.h>
#include <kern/mem/memory_manager.h>
#include <kern/mem/shared_memory_manager.h>
#include <kern/tests/utilities.h>
#include <kern/tests/test_kheap.h>
#include <kern/tests/test_dynamic_allocator.h>
#include <kern/tests/test_commands.h>
#include <kern/disk/pagefile_manager.h>
#include <kern/conc/bigkernellock.h>

extern int sys_calculate_free_frames();

//Functions Declaration
//======================
void print_welcome_message();
//=======================================

//First ever function called in FOS kernel
extern int __autograde__ ;
void FOS_initialize()
{
	//get actual addresses after code linking
	extern char start_of_uninitialized_data_section[], end_of_kernel[];

	//cprintf("*	1) Global data (BSS) section...");
	{
		// Before doing anything else,
		// clear the uninitialized global data (BSS) section of our program, from start_of_uninitialized_data_section to end_of_kernel
		// This ensures that all static/global variables start with zero value.
		memset(start_of_uninitialized_data_section, 0, end_of_kernel - start_of_uninitialized_data_section);
	}
	//cprintf("[DONE]\n");

	{
		// Initialize the console.
		// Can't call cprintf until after we do this!
		cons_init();
		//print welcome message
		print_welcome_message();
	}

	cprintf("\n********************************************************************\n");
	cprintf("* INITIALIZATIONS:\n");
	cprintf("*=================\n");

	cprintf("* 1) CPU...");
	{
		//Initialize the Main CPU
		cpu_init(0);
	}
	cprintf("[DONE]\n");

	cprintf("* 2) MEMORY:\n");
	{
		detect_memory();
		initialize_kernel_VM();
		initialize_paging();
#if USE_KHEAP
		kheap_init();
		sharing_init();
#endif
		fault_handler_init();
		set_uheap_strategy(UHP_PLACE_CUSTOMFIT);
	}
	//cprintf("* [DONE]\n");

	cprintf("* 3) DISK...");
	{
		ide_init();
	}
	cprintf("[DONE]\n");

	cprintf("* 4) USER ENVs...");
	{
		// Lab 3 user environment initialization functions
		env_init();
		idt_init();
	}
	cprintf("[DONE]\n");

	cprintf("* 5) SCHEDULER & MULTI-TASKING:\n");
	{
		// Lab 4 multitasking initialization functions
		kclock_init();
		sched_init() ;
	}
	//cprintf("* [DONE]\n");

	cprintf("* 6) MULTI-CPUs...");
	{
		// Lab 4 multiprocessor initialization functions
		mp_init();
		lapic_init();

		lock_kernel();

		boot_aps();
	}

	cprintf("* 7) PROGRAMMABLE INTERRUPT CONTROLLER:\n");
	{
		// Lab 4 multitasking initialization functions
		pic_init();
		cprintf("*	PIC is initialized\n");
		cprintf("*	IRQ1 (Keyboard): is ");
		if (KB_IO_METHOD != PROGRAMMED_IO)
		{
			//Enable KB Interrupt
			irq_clear_mask(IRQ_KBD);
			cprintf_colored(TEXT_light_green, "Enabled\n");
		}
		else
		{
			cprintf_colored(TEXT_light_red, "Disabled [Programmed I/O]\n");
		}
		cprintf("*	IRQ4 (COM1): is ");
		if (KB_IO_METHOD != PROGRAMMED_IO)
		{
			//Enable COM1 Interrupt
			irq_clear_mask(IRQ_SERIAL);
			cprintf_colored(TEXT_light_green, "Enabled\n");
		}
		else
		{
			cprintf_colored(TEXT_light_red, "Disabled [Programmed I/O]\n");
		}

		cprintf("*	IRQ14 (Primary ATA Hard Disk): is ");
		if (DISK_IO_METHOD != PROGRAMMED_IO)
		{
			//Enable Primary ATA Hard Disk Interrupt
			irq_clear_mask(IRQ_IDE);
			cprintf_colored(TEXT_light_green, "Enabled\n");
		}
		else
		{
			cprintf_colored(TEXT_light_red, "Disabled [Programmed I/O]\n");
		}
	}


	cprintf("* 7) ESP to SCHED KERN STACK:\n");
	{
		//Relocate SP to its corresponding location in the specific stack area below KERN_BASE (SCHD_KERN_STACK_TOP)
		uint32 old_sp = read_esp();
		uint32 sp_offset = (uint32)ptr_stack_top - old_sp ;
		uint32 new_sp = KERN_STACK_TOP - sp_offset;
		write_esp(new_sp);
		cprintf("*	old SP = %x - updated SP = %x\n", old_sp, read_esp());
	}
	//cprintf("* [DONE]\n");
	cprintf("********************************************************************\n");

	// start the kernel command prompt.
	while (1==1)
	{
		cprintf("\nWelcome to the FOS kernel command prompt!\n");
		cprintf("Type 'help' for a list of commands.\n");
		cprintf("\nAUTOGRADE IS %s\n",__autograde__ == 1? "ENABLED" : "DISABLED") ;

		if (__autograde__)
		{
			/*CHECK THE FOLLOWING:
			 * 1) time of each test
			 * 2) "unhandled trap in" message
			 */

			cprintf("\nPROJECT Automatic testing is STARTED...\n") ;

			//TEST#1: DYNAMIC ALLOCATOR [PARTIAL GRADING]
			{
				char cmd1[BUFLEN] = "tst dynalloc init"; //[0/1 GRADING] [10 sec]
				char cmd2[BUFLEN] = "tst dynalloc alloc";//[PARTIAL GRADING] [10 sec]
				char cmd3[BUFLEN] = "tst dynalloc free"; //[PARTIAL GRADING] [20 sec]
				//char cmd4[BUFLEN] = "tst dynalloc realloc";
				//execute_command(cmd1);
			}
			//TEST#2: KERNEL HEAP [PARTIAL GRADING]
			{
				char cmd1[BUFLEN] = "tst kheap CF kmalloc blk";	//15 sec
				char cmd2[BUFLEN] = "tst kheap CF kmalloc page";//40 sec
				char cmd4[BUFLEN] = "tst kheap CF kfree blk";	//15 sec
				char cmd5[BUFLEN] = "tst kheap CF kfree page";	//50 sec
				char cmd7[BUFLEN] = "tst kheap cf kvirtaddr";	//20 sec
				char cmd8[BUFLEN] = "tst kheap cf kphysaddr";	//20 sec
				//char cmd3[BUFLEN] = "tst kheap CF kmalloc both";
				//char cmd6[BUFLEN] = "tst kheap CF kfree both";
				//execute_command(cmd8);
			}
			//TEST#3: FAULT HANDLER I [PARTIAL GRADING]
			{
				char cmd1[BUFLEN] = "run tia 15";	//20 sec
				char cmd2[BUFLEN] = "run tpp 22";	//10 sec
				//execute_command(cmd2);
			}
			//TEST: FAULT HANDLER II [OPTIMAL] [0/1 GRADING]	[10 sec]
			{
				char cmd0[BUFLEN] = "optimal";
				char cmd1[BUFLEN] = "run toptimal1 11";
				char cmd2[BUFLEN] = "run toptimal2 3000";
				char cmd3[BUFLEN] = "run toptimal3 11";
				//execute_command(cmd0);
				//execute_command(cmd3);
			}
			//TEST: FAULT HANDLER II [CLOCK] [0/1 GRADING]	[10 sec]
			{
				char cmd0[BUFLEN] = "clock";
				char cmd1[BUFLEN] = "run tpr1 11";
				char cmd2[BUFLEN] = "run tpr2 6";
				char cmd3[BUFLEN] = "run tclock1 11";
				char cmd4[BUFLEN] = "run tclock2 11";
				//execute_command(cmd0);
				//execute_command(cmd4);
			}
			//TEST: FAULT HANDLER II [MODIFIED CLOCK] [0/1 GRADING]	[10 sec]
			{
				char cmd0[BUFLEN] = "modclock";
				char cmd1[BUFLEN] = "run tpr1 11";
				char cmd2[BUFLEN] = "run tpr2 6";
				char cmd3[BUFLEN] = "run tmodclk1 11";
				char cmd4[BUFLEN] = "run tmodclk2 11";
				//execute_command(cmd0);
				//execute_command(cmd4);
			}
			//TEST: FAULT HANDLER II [LRU] [0/1 GRADING]	[10 sec]
			{
				char cmd0[BUFLEN] = "lru 1";
				char cmd1[BUFLEN] = "run tpr1 11";
				char cmd2[BUFLEN] = "run tpr2 6";
				char cmd3[BUFLEN] = "run tlru 11";
				//execute_command(cmd0);
				//execute_command(cmd3);
			}
			//TEST: USER HEAP	[PARTIAL GRADING]
			{
				char cmd0[BUFLEN] = "uhcustomfit";
				char cmd1[BUFLEN] = "run tm1 3000";	//15 sec
				char cmd3[BUFLEN] = "run tf1 3000";	//30 sec
				char cmd5[BUFLEN] = "run tcf1 3000";//15 sec
				//char cmd2[BUFLEN] = "run tm2 3000";
				//char cmd4[BUFLEN] = "run tf2 3000";
				//char cmd6[BUFLEN] = "run tcf2 10000";
				//execute_command(cmd0);
				//execute_command(cmd5);
			}
			//TEST: SHARED MEMORY [PARTIAL GRADING] [20 sec]
			{
				char cmd0[BUFLEN] = "uhcustomfit";
				char cmd1[BUFLEN] = "run tshr1 3000";
				char cmd2[BUFLEN] = "run tshr2 3000";
				char cmd3[BUFLEN] = "run tshr3 3000";
				char cmd4[BUFLEN] = "run tcf3 3000";	//depend on USER HEAP (malloc, free)
				char cmd5[BUFLEN] = "run tst_protection 5000"; //[0/1 GRADING] [time limit: 6 mins]
				//execute_command(cmd0);
				//execute_command(cmd4);
			}
			//TEST: PRIORITY RR SCHEDULER [PARTIAL GRADING]
			{
				char cmd01[BUFLEN] = "schedPRIRR 10 40 2000";
				char cmd02[BUFLEN] = "schedPRIRR 10 40 2000";
				char cmd03[BUFLEN] = "schedPRIRR 10 10 50";
				//execute_command(cmd03);
				char cmdU1_1[BUFLEN] = "tst priorityRR 0";	//100 sec
				char cmdU2_1[BUFLEN] = "tst priorityRR 1";	//100 sec
				char cmdU3_1[BUFLEN] = "tst priorityRR 2";	//150 sec
				//execute_command(cmdU3_1);
			}

			//TEST: PROTECTION [0/1 GRADING]
			{
				char cmd1[BUFLEN] = "run tst_chan_one 500";	//20 sec
				char cmd2[BUFLEN] = "run tst_chan_all 500";	//15 sec
				char cmd3[BUFLEN] = "run tst_sleeplock 500";//40 sec
				char cmd4[BUFLEN] = "run tst_ksem1 500";	//15 sec
				char cmd5[BUFLEN] = "run tst_ksem2 500";	//40 sec
				//execute_command(cmd4);
			}

			//TEST#5: OVERALL SCENARIOS
			{
				char cmdInv[BUFLEN] = "run tia 15";
				//execute_command(cmdInv);


				char cmd1[BUFLEN] = "run sc_MultipleApps 10000";		//60s
				char cmd2[BUFLEN] = "run sc_arrops 10000";				//20s
				char cmd3_1[BUFLEN] = "run sc_repl_policies_1 50";		//30s (ALL POLICIES)
				char cmd3_2[BUFLEN] = "run sc_repl_policies_2 50";		//15s (OPT vs CLOCK)
				//execute_command(cmd3);

				char cmd0_1[BUFLEN] = "clock";
				char cmd0_2[BUFLEN] = "modclock";
				//execute_command(cmd0_2);
				char cmd0_3[BUFLEN] = "schedPRIRR 30 50 200";
				//execute_command(cmd0_3);
				char cmd4_1[BUFLEN] = "run sc_priRR_1 100 29";	//modified clock 	//100sec
				char cmd4_2[BUFLEN] = "run sc_priRR_1 100 29";	//clock 			//100sec
				char cmd5_1[BUFLEN] = "run sc_priRR_2 100 29";	//modified clock 	//180sec
				char cmd5_2[BUFLEN] = "run sc_priRR_2 100 29";	//clock 			//180sec
				//execute_command(cmd5_1);
			}
			//TEST#X: BONUS - DA: BLOCK IF NO BLOCK
			{
				char cmd1[BUFLEN] = "run tst_da_block 100";
				char cmd2[BUFLEN] = "run tst_da_block2 100";
				//execute_command(cmd2);
			}
			//TEST#X: BONUS - KREALLOC	[PARTIAL GRADING] [15 sec]
			{
				char cmd0[BUFLEN] = "khcustomfit";
				char cmd1[BUFLEN] = "tst kheap CF krealloc page";
				char cmd2[BUFLEN] = "tst kheap CF krealloc blk";
				char cmd3[BUFLEN] = "tst kheap CF krealloc both";
				//execute_command(cmd0);
				//execute_command(cmd3);
			}
			//TEST#X: BONUS - FAST PAGE ALLOCATOR
			{
				char cmd0[BUFLEN] = "khcustomfit";
				char cmd1[BUFLEN] = "tst kheap CF fast"; //64KB blocks [0/1 GRADING] [time limit: 5 secs]
				char cmd2[BUFLEN] = "tst kheap CF fast"; //16KB blocks [0/1 GRADING] [time limit: 15 secs]
				//execute_command(cmd0);
				//execute_command(cmd1);
			}
			//TEST#X: BONUS - EXIT
			{
				char cmd0[BUFLEN] = "clock";
				//Scenario 1: without using dynamic allocation/de-allocation [PLACEMENT]
				char cmd1[BUFLEN] = "run tef1 100";		//15sec
				//Scenario 2: using dynamic allocation and free [REPLACEMENT]
				char cmd2[BUFLEN] = "run tef2 20";		//50sec
				//Scenario3: using dynamic allocation and free [process kill itself] [REPLACEMENT]
				char cmd3[BUFLEN] = "run tef3 20";		//60sec
				//Scenario 4: using create & get of shared variables [REPLACEMENT]
				char cmd4[BUFLEN] = "run tef4 10";		//15sec
				//Scenario 5: using create, get and free shared variables by the created environment itself before calling env_free [PLACEMENT]
				char cmd5[BUFLEN] = "run tef5_2 3000";	//15sec
				//				//Scenario 6: using shared variables and semaphores together [PLACEMENT]
				//				char cmd6[BUFLEN] = "run tef6 3000";

				//execute_command(cmd0);
				//execute_command(cmd3);

			}

			cprintf("PROJECT Automatic testing is ENDED\n") ;
			__autograde__ = 0;
		}
		get_into_prompt();
	}
}


void print_welcome_message()
{
	cprintf("\n\n\n");
	cprintf("\t\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	cprintf("\t\t!!                                                             !!\n");
	cprintf("\t\t!!                   !! FCIS says HELLO !!                     !!\n");
	cprintf("\t\t!!                                                             !!\n");
	cprintf("\t\t!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
	cprintf("\n\n\n\n");
}


/*
 * Variable panicstr contains argument to first call to panic; used as flag
 * to indicate that the kernel has already called panic.
 */
static const char *panicstr;

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", exit the curenv and schedule the next environment.
 */
void _panic(const char *file, int line, const char *fmt,...)
{
	struct Env* cur_env = get_cpu_proc();

	va_list ap;

	//if (panicstr)
	//	goto dead;
	panicstr = fmt;

	va_start(ap, fmt);
	cprintf_colored(TEXT_PANIC_CLR, "\nkernel [EVAL_FINAL]panic at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);

	dead:
	/* break into the fos scheduler */
	//2013: Check if the panic occur when running an environment
	if (cur_env != NULL && cur_env->env_status == ENV_RUNNING)
	{
		//cprintf("\n>>>>>>>>>>> exiting the cur env<<<<<<<<<<<<\n");
		//Place the running env into the exit queue then switch to the scheduler
		env_exit(); //env_exit --> sched_exit_env --> sched --> context_switch into fos_scheduler
	}
	//else //2024: panic from Kernel and no current running env
	{
		char* esp = (char*)read_esp();
		cprintf("esp = %x\n", esp);
		//			//2024: make sure that the SP points to the kernel stack (either the one above KERN_BASE or below it)
		//			assert((esp < ptr_stack_top && esp >= ptr_stack_bottom) ||
		//					(esp < (char*)SCHD_KERN_STACK_TOP && esp >= (char*)SCHD_KERN_STACK_TOP - KERNEL_STACK_SIZE)) ;

		get_into_prompt();
	}

}

/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", exit all env's and then enters the kernel command prompt.
 */
void _panic_all(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	//if (panicstr)
	//	goto dead;
	panicstr = fmt;

	va_start(ap, fmt);
	cprintf_colored(TEXT_PANIC_CLR, "\nkernel panic at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);

	dead:
	/* break into the command prompt */
	pushcli();
	struct CpuInfo *c = mycpu();
	int sched_stat = c->scheduler_status;
	popcli();
	/*2022*///Check if the scheduler is successfully initialized or not
	if (sched_stat != SCH_UNINITIALIZED)
	{
		//exit all ready env's
		sched_exit_all_ready_envs();
		struct Env* cur_env = get_cpu_proc();
		if (cur_env != NULL && cur_env->env_status == ENV_RUNNING)
		{
			//cprintf("exit curenv...........\n");
			//Place the running env into the exit queue then switch to the scheduler
			env_exit(); //env_exit --> sched_exit_env --> sched --> context_switch into fos_scheduler
		}
//		cprintf("scheduler_status=%d\n", scheduler_status);
//		fos_scheduler();
	}
	//else //2024: panic from Kernel and no current running env
	{
		//2024: make sure that the SP points to the kernel stack (either the one above KERN_BASE or below it)
		char* esp = (char*)read_esp();
//		assert((esp < ptr_stack_top && esp >= ptr_stack_bottom) ||
//				(esp < (char*)SCHD_KERN_STACK_TOP && esp >= (char*)SCHD_KERN_STACK_TOP - KERNEL_STACK_SIZE)) ;

		get_into_prompt();

	}
}


/*
 * Panic is called on unresolvable fatal errors.
 * It prints "panic: mesg", exit the curenv (if any) and break into the command prompt.
 */
void _panic_into_prompt(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	//if (panicstr)
	//	goto dead;
	panicstr = fmt;

	va_start(ap, fmt);
	cprintf_colored(TEXT_PANIC_CLR,"\nkernel panic at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);

//	dead:
	/* break into the fos scheduler */
	//2013: Check if the panic occur when running an environment
	struct Env* cur_env = get_cpu_proc();
	if (cur_env != NULL && cur_env->env_status == ENV_RUNNING)
	{
		//Place the running env into the exit queue then switch to the scheduler
		env_exit(); //env_exit --> sched_exit_env --> sched --> context_switch into fos_scheduler
	}

	get_into_prompt();

}

/* like panic, but don't enters the kernel command prompt*/
void _warn(const char *file, int line, const char *fmt,...)
{
	va_list ap;

	va_start(ap, fmt);
	cprintf_colored(TEXT_WARN_CLR, "\nkernel warning at %s:%d: ", file, line);
	vcprintf(fmt, ap);
	cprintf("\n");
	va_end(ap);
}


