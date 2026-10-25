// hello, world
#include <inc/lib.h>

void
_main(void)
{
	int64 s = 0;
	while (1==1)
	{
		s++;
	}
	extern unsigned char * etext;
	//cprintf("HELLO WORLD , FOS IS SAYING HI :D:D:D %d\n",4);
	atomic_cprintf("HELLO WORLD , FOS IS SAYING HI :D:D:D \n");
	atomic_cprintf("end of code = %x\n",etext);
}
