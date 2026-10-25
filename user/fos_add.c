// hello, world
#include <inc/lib.h>

void
_main(void)
{
	int i1=0;
	int i2=0;

	i1 = strtol("100", NULL, 10);
	i2 = strtol("200", NULL, 10);

	cprintf("%@number 1 + number 2 = %d\n", i1+i2);
	//atomic_cprintf("%@number 1 + number 2 = %d\n",i1+i2);

	int N = 1000000;
	int64 sum = 0;
	for (int i = 0; i < N; ++i) {
		sum+=i ;
	}
	sys_lock_cons();
	{
		cprintf_colored(TEXT_green + TEXTBG_blue, "sum 1->%d = %lld\n", N, sum);
		cprintf_colored(TEXT_magenta + TEXTBG_cyan, "%~sum 1->%d = %lld\n", N, sum);
	}
	sys_unlock_cons();
	return;
}
