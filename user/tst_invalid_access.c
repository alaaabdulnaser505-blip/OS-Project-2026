/********************************************************** */
/* MAKE SURE PAGE_WS_MAX_SIZE = 15 - SECOND CHANCE LIST = 2*/
/************************************************************/

#include <inc/lib.h>

void _main(void)
{
	int eval = 0;

	cprintf_colored(TEXT_cyan, "%~PART I: Test the Pointer Validation inside fault_handler(): [70%]\n");
	cprintf_colored(TEXT_cyan, "%~=================================================================\n");
	rsttst();
	int ID1 = sys_create_env("tia_slave1", (myEnv->page_WS_max_size), (myEnv->page_2nd_WS_size),(myEnv->percentage_of_WS_pages_to_be_removed));
	sys_run_env(ID1);

	int ID2 = sys_create_env("tia_slave2", (myEnv->page_WS_max_size), (myEnv->page_2nd_WS_size),(myEnv->percentage_of_WS_pages_to_be_removed));
	sys_run_env(ID2);

	int ID3 = sys_create_env("tia_slave3", (myEnv->page_WS_max_size), (myEnv->page_2nd_WS_size),(myEnv->percentage_of_WS_pages_to_be_removed));
	sys_run_env(ID3);
	env_sleep(15000);

	if (gettst() != 0)
		cprintf_colored(TEXT_TESTERR_CLR, "\n%~PART I... Failed.\n");
	else
	{
		cprintf_colored(TEXT_green, "\n%~PART I... completed successfully\n\n");
		eval += 70;
	}

	cprintf_colored(TEXT_cyan, "%~PART II: PLACEMENT: Test the Invalid Access to a NON-EXIST page in Page File, Stack & Heap: [30%]\n");
	cprintf_colored(TEXT_cyan, "%~=================================================================================================\n");

	rsttst();
	int ID4 = sys_create_env("tia_slave4", (myEnv->page_WS_max_size), (myEnv->page_2nd_WS_size),(myEnv->percentage_of_WS_pages_to_be_removed));
	sys_run_env(ID4);

	env_sleep(15000);

	if (gettst() != 0)
		cprintf_colored(TEXT_TESTERR_CLR, "\n%~PART II... Failed.\n");
	else
	{
		cprintf_colored(TEXT_green, "\n%~PART II... completed successfully\n\n");
		eval += 30;
	}
	cprintf_colored(TEXT_light_green, "%~\ntest invalid access completed. Eval = %d%\n\n", eval);

}

