// Test the Sleep and wakeup on a channel
// Master program: create and run slaves, wait them to finish
#include <inc/lib.h>
#define __AUTOGRADE__ 1

void
_main(void)
{
	cprintf_colored(TEXT_yellow,"==============================================\n");
	cprintf_colored(TEXT_yellow,"MAKE SURE to have a FRESH RUN for this test\n(i.e. don't run any program/test before it)\n");
	cprintf_colored(TEXT_yellow,"==============================================\n");

	int envID = sys_getenvid();
	char slavesCnt[10];
#if __AUTOGRADE__
	int numOfSlaves = 7 ;
	cprintf("Enter the number of Slave Programs: %d\n", numOfSlaves);
#else
	readline("Enter the number of Slave Programs: ", slavesCnt);
	int numOfSlaves = strtol(slavesCnt, NULL, 10);
#endif

	//Save number of slaved to be checked later
	char cmd1[64] = "__NumOfSlaves@Set";
	sys_utilities(cmd1, (uint32)(&numOfSlaves));

	//Create and run slave programs that should sleep
	int id;
	for (int i = 0; i < numOfSlaves; ++i)
	{
		id = sys_create_env("tstChanOneSlave", (myEnv->page_WS_max_size),(myEnv->page_2nd_WS_size), (myEnv->percentage_of_WS_pages_to_be_removed));
		if (id== E_ENV_CREATION_ERROR)
		{
			cprintf_colored(TEXT_TESTERR_CLR, "\n%~insufficient number of processes in the system! only %d slave processes are created\n", i);
			numOfSlaves = i;
			break;
		}
		sys_run_env(id);
	}

	//Wait until all slaves are blocked
	int numOfBlockedProcesses = 0;
	char cmd2[64] = "__GetChanQueueSize__";
	sys_utilities(cmd2, (uint32)(&numOfBlockedProcesses));
	int cnt = 0;
	while (numOfBlockedProcesses != numOfSlaves)
	{
		env_sleep(5000);
		if (cnt == numOfSlaves)
		{
			panic("%~test channels failed! unexpected number of blocked slaves. Expected = %d, Current = %d", numOfSlaves, numOfBlockedProcesses);
		}
		char cmd3[64] = "__GetChanQueueSize__";
		sys_utilities(cmd3, (uint32)(&numOfBlockedProcesses));
		cnt++ ;
	}

	rsttst();

	//Wakeup one
	char cmd4[64] = "__WakeupOne__";
	sys_utilities(cmd4, 0);

	//Wait until all slave finished
	cnt = 0;
	while (gettst() != numOfSlaves)
	{
		env_sleep(10000);
		if (cnt == numOfSlaves)
		{
			panic("%~test channels failed! not all slaves finished");
		}
		cnt++ ;
	}

	cprintf_colored(TEXT_light_green, "%~\n\nCongratulations!! Test of Channel (sleep & wakeup ONE) completed successfully!!\n\n");

	return;
}
