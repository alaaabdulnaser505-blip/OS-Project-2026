// Test the Sleep Lock for applying critical section
// Slave program: acquire, release then increment test to declare finishing
#include <inc/lib.h>
extern volatile bool printStats ;

void
_main(void)
{
	int envID = sys_getenvid();

	//Acquire the lock
	char cmd1[64] = "__AcquireSleepLock__";
	sys_utilities(cmd1, 0);
	{
		if (gettst() > 1)
		{
			//Other slaves: wait for a while
			env_sleep(RAND(5000, 10000));
		}
		else
		{
			//this is the first slave inside C.S.! so wait until receiving signal from master
			while (gettst() != 1);
		}

		//Check lock value inside C.S.
		int lockVal = 0;
		char cmd2[64] = "__GetLockValue__";
		sys_utilities(cmd2, (int)(&lockVal));
		if (lockVal != 1)
		{
			panic("%~test sleeplock failed! lock is not held while it's expected to be");
		}

		//Validate the number of blocked processes till now
		int numOfBlockedProcesses = 0;
		char cmd3[64] = "__GetLockQueueSize__";
		sys_utilities(cmd3, (uint32)(&numOfBlockedProcesses));
		int numOfFinishedProcesses = gettst() -1 /*since master already incremented it by 1*/;
		int numOfSlaves = 0;
		char cmd4[64] = "__NumOfSlaves@Get";
		sys_utilities(cmd4, (uint32)(&numOfSlaves));

		if (numOfFinishedProcesses + numOfBlockedProcesses != numOfSlaves - 1)
		{
			panic("%~test SleepLock failed! inconsistent number of blocked & waken-up processes. #wakenup %d + #blocked %d != #slaves %d", numOfFinishedProcesses, numOfBlockedProcesses, numOfSlaves-1);
		}

		//indicates finishing
		inctst();
	}
	//Release the lock
	char cmd5[64] = "__ReleaseSleepLock__";
	sys_utilities(cmd5, 0);

	cprintf_colored(TEXT_light_magenta, ">>> Slave %d is Finished\n", envID);
	printStats = 0;

	return;
}
