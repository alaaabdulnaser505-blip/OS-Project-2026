#include <inc/lib.h>
extern volatile bool printStats;

void _main(void)
{
	printStats = 0;

	int32 parentenvID = sys_getparentenvid();
	int delay;

	/*[1] GET SHARED VARIABLE, SEMAPHORE SEL, check-finishing counter*/
	int *X = sget(parentenvID, "X") ;
	int *protType = sget(parentenvID, "protType") ;
	int * finishedCount = sget(parentenvID, "finishedCount") ;
	struct semaphore T, finished, finishedCountMutex ;
	struct uspinlock *sT, *sfinishedCountMutex;

	if (*protType == 1)
	{
		T = get_semaphore(parentenvID, "T");
		finished = get_semaphore(parentenvID, "finished");
		finishedCountMutex = get_semaphore(parentenvID, "finishedCountMutex");
	}
	else if (*protType == 2)
	{
		sT = sget(parentenvID, "T");
		sfinishedCountMutex = sget(parentenvID, "finishedCountMutex");
	}

	/*[2] DO THE JOB*/
	int Z ;
	if (*protType == 1)
	{
		wait_semaphore(T);
	}
	else if (*protType == 2)
	{
		acquire_uspinlock(sT);
	}

	//random delay
	delay = RAND(2000, 10000);
	env_sleep(delay);
	//	cprintf("delay = %d\n", delay);

	Z = (*X) + 1 ;

	//random delay
	delay = RAND(2000, 10000);
	env_sleep(delay);
	//	cprintf("delay = %d\n", delay);

	(*X) = Z ;

	//random delay
	delay = RAND(2000, 10000);
	env_sleep(delay);
	//	cprintf("delay = %d\n", delay);

	/*[3] DECLARE FINISHING*/
	if (*protType == 1)
	{
		signal_semaphore(finished);

		wait_semaphore(finishedCountMutex);
		{
			(*finishedCount)++ ;
		}
		signal_semaphore(finishedCountMutex);
	}
	else if (*protType == 2)
	{
		acquire_uspinlock(sfinishedCountMutex);
		{
			(*finishedCount)++ ;
		}
		release_uspinlock(sfinishedCountMutex);
	}else
	{
		sys_lock_cons();
		{
			(*finishedCount)++ ;
		}
		sys_unlock_cons();
	}
}
