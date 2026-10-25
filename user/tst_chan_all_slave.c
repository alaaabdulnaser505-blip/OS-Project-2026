// Test the Sleep and wakeup on a channel
// Slave program: sleep, increment test after wakeup
#include <inc/lib.h>
extern volatile bool printStats ;

void
_main(void)
{
	int envID = sys_getenvid();

	//Sleep on the channel
	char cmd[64] = "__Sleep__";
	sys_utilities(cmd, 0);

	//indicates wakenup
	inctst();

	cprintf_colored(TEXT_light_magenta, ">>> Slave %d is Finished\n", envID);
	printStats = 0;

	return;
}
