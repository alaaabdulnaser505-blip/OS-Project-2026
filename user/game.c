#include <inc/lib.h>

void
_main(void)
{
	int i=28;
	int txtClr = TEXT_black;
	for(;i<128; i++)
	{
		txtClr = (txtClr + 1) % 16;
		int c=0;
		for(;c<10; c++)
		{
			cprintf_colored(txtClr, "%~%c",i);
		}
		int d=0;
		for(; d< 5000000; d++);
		c=0;
		for(;c<10; c++)
		{
			cprintf("%~\b");
		}
	}

	return;
}
