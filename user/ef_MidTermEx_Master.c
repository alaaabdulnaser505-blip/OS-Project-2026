// Scenario that tests the usage of shared variables
#include <inc/lib.h>

void
_main(void)
{
	/*[1] CREATE SHARED VARIABLE & INITIALIZE IT*/
	int *X = smalloc("X", sizeof(int) , 1) ;
	*X = 5 ;

	/*[2] SPECIFY WHETHER TO USE SEMAPHORE OR NOT*/
	//cprintf("Do you want to use semaphore (y/n)? ") ;
	//char select = getchar() ;
	char select = 'y';
	//cputchar(select);
	//cputchar('\n');

	/*[3] SHARE THIS SELECTION WITH OTHER PROCESSES*/
	int *useSem = smalloc("useSem", sizeof(int) , 0) ;
	*useSem = 0 ;
	if (select == 'Y' || select == 'y')
		*useSem = 1 ;

	struct semaphore T, finished, finishedCountMutex ;
	int *numOfFinished ;
	//Create the check-finishing counter
	numOfFinished = smalloc("finishedCount", sizeof(int), 1) ;
	*numOfFinished = 0 ;

	if (*useSem == 1)
	{
		T = create_semaphore("T", 0);
		finished = create_semaphore("finished", 0);
		finishedCountMutex = create_semaphore("finishedCountMutex", 1);
	}

	/*[4] CREATE AND RUN ProcessA & ProcessB*/

	//Create the 2 processes
	int32 envIdProcessA = sys_create_env("midterm_a", (myEnv->page_WS_max_size),(myEnv->page_2nd_WS_size), 50);
	int32 envIdProcessB = sys_create_env("midterm_b", (myEnv->page_WS_max_size),(myEnv->page_2nd_WS_size), 50);
	if (envIdProcessA == E_ENV_CREATION_ERROR || envIdProcessB == E_ENV_CREATION_ERROR)
		panic("NO AVAILABLE ENVs...");

	//Run the 2 processes
	sys_run_env(envIdProcessA);
	//env_sleep(10000);
	sys_run_env(envIdProcessB);

	/*[5] BUSY-WAIT TILL FINISHING BOTH PROCESSES*/
	if (*useSem == 1)
	{
		wait_semaphore(finished);
		wait_semaphore(finished);
	}
	else
	{
		while (*numOfFinished != 2) ;
	}

	/*[6] PRINT X*/
	atomic_cprintf("Final value of X = %d\n", *X);

	//ensure that X has the expected value (=11)
	if (*X != 11)
		panic("Final value of X is not correct. Semaphore and/or shared variables are not working correctly\n");

	int32 parentenvID = sys_getparentenvid();
	if(parentenvID > 0)
	{
		//Get the check-finishing counter
		int *AllFinish = NULL;
		AllFinish = sget(parentenvID, "finishedCount") ;

		//DISABLE the interrupt to ensure the env_free is done as a whole without preemption
		//to avoid context switch (due to clock interrupt) while freeing the env to prevent:
		//	1. context switching to a wrong process specially in the part of temporarily switching the CPU process for freeing shared variables
		//	2. changing the # free frames
		char changeIntCmd[100] = "__changeInterruptStatus__";
		sys_utilities(changeIntCmd, 0);
		{
			sys_destroy_env(envIdProcessA);
			sys_destroy_env(envIdProcessB);
		}
		sys_utilities(changeIntCmd, 1);

		sys_lock_cons();
		{
			(*AllFinish)++ ;
		}
		sys_unlock_cons();
	}

	return;
}
