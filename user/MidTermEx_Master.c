// Scenario that tests the usage of shared variables
#include <inc/lib.h>

void
_main(void)
{
	/*[1] CREATE SHARED VARIABLE & INITIALIZE IT*/
	int *X = smalloc("X", sizeof(int) , 1) ;
	*X = 5 ;

	/*[2] SPECIFY WHETHER TO USE SEMAPHORE OR NOT*/
	char select;
	sys_lock_cons();
	{
		cprintf("%~Which type of concurrency protection do you want to use? \n") ;
		cprintf("%~0) Nothing\n") ;
		cprintf("%~1) Semaphores\n") ;
		cprintf("%~2) SpinLock\n") ;
		cprintf("%~your choice (0, 1, 2): ") ;
		select = getchar() ;
		cputchar(select);
		cputchar('\n');
	}
	sys_unlock_cons();

	/*[3] SHARE THIS SELECTION WITH OTHER PROCESSES*/
	int *protType = smalloc("protType", sizeof(int) , 0) ;
	*protType = 0 ;
	if (select == '1') 		*protType = 1 ;
	else if (select == '2') *protType = 2 ;

	struct semaphore T, finished, finishedCountMutex;
	struct uspinlock *sT, *sfinishedCountMutex;
	int *numOfFinished ;
	if (*protType == 1)
	{
		T = create_semaphore("T", 0);
		finished = create_semaphore("finished", 0);
		finishedCountMutex = create_semaphore("finishedCountMutex", 1);
	}
	else if (*protType == 2)
	{
		sT = smalloc("T", sizeof(struct uspinlock), 1);
		init_uspinlock(sT, "T", 0);
		sfinishedCountMutex = smalloc("finishedCountMutex", sizeof(struct uspinlock), 1);
		init_uspinlock(sfinishedCountMutex, "finishedCountMutex", 1);
	}
	//Create the check-finishing counter
	numOfFinished = smalloc("finishedCount", sizeof(int), 1) ;
	*numOfFinished = 0 ;

	/*[4] CREATE AND RUN ProcessA & ProcessB*/

	//Create the 2 processes
	int32 envIdProcessA = sys_create_env("midterm_a", (myEnv->page_WS_max_size),(myEnv->page_2nd_WS_size), (myEnv->percentage_of_WS_pages_to_be_removed));
	int32 envIdProcessB = sys_create_env("midterm_b", (myEnv->page_WS_max_size), (myEnv->page_2nd_WS_size),(myEnv->percentage_of_WS_pages_to_be_removed));

	//Run the 2 processes
	sys_run_env(envIdProcessA);
	sys_run_env(envIdProcessB);

	/*[5] WAIT TILL FINISHING BOTH PROCESSES*/
	if (*protType == 1)
	{
		wait_semaphore(finished);
		wait_semaphore(finished);
	}
	if (*protType == 2)
	{
		while (*numOfFinished != 2) ;
	}
	else
	{
		while (*numOfFinished != 2) ;
	}

	/*[6] PRINT X*/
	atomic_cprintf("%~Final value of X = %d\n", *X);

	return;
}
