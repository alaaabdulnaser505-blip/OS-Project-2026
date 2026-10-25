// Test the Sleep and wakeup on a channel
// Slave program: sleep, increment test after wakeup
#include <inc/lib.h>
extern volatile bool printStats ;

void
_main(void)
{
	int envID = sys_getenvid();

	//Sleep on the channel
	char cmd0[64] = "__Sleep__";
	sys_utilities(cmd0, 0);

	//wait for a while
	env_sleep(RAND(1000, 5000));

	//Validate the number of blocked processes till now
	int numOfBlockedProcesses = 0;
	char cmd1[64] = "__GetChanQueueSize__";
	sys_utilities(cmd1, (uint32)(&numOfBlockedProcesses));
	int numOfWakenupProcesses = gettst() ;
	int numOfSlaves = 0;
	char cmd2[64] = "__NumOfSlaves@Get";
	sys_utilities(cmd2, (uint32)(&numOfSlaves));

	if (numOfWakenupProcesses + numOfBlockedProcesses != numOfSlaves - 1 /*Except this process since it not indicating wakeup yet*/)
	{
		panic("%~test channels failed! inconsistent number of blocked & waken-up processes.");
	}

	//indicates wakenup
	inctst();

	//wakeup another one
	char cmd3[64] = "__WakeupOne__";
	sys_utilities(cmd3, 0);

	cprintf_colored(TEXT_light_magenta, ">>> Slave %d is Finished\n", envID);
	printStats = 0;
	return;
}
