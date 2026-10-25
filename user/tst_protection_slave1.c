#include <inc/lib.h>

void
_main(void)
{

	/*=================================================*/
	//Initial test to ensure it works on "PLACEMENT" not "REPLACEMENT"
#if USE_KHEAP
	{
		if (LIST_SIZE(&(myEnv->page_WS_active_list)) >= myEnv->page_WS_max_size)
			panic("Please increase the WS size");
	}
#else
	panic("make sure to enable the kernel heap: USE_KHEAP=1");
#endif
	/*=================================================*/

	{
		char initname[10] = "x";
		char name[10] ;
#define NUM_OF_OBJS 5000
		uint32* vars[NUM_OF_OBJS];
		for (int s = 0; s < NUM_OF_OBJS; ++s)
		{
			char index[10];
			ltostr(s, index);
			strcconcat(initname, index, name);
			vars[s] = smalloc(name, PAGE_SIZE, 1);
			*vars[s] = s;
			if (s % 1000 == 0)
				cprintf_colored(TEXT_light_cyan, "%d creates %d shared objects\n", myEnv->env_id, s);
		}
		for (int s = 0; s < NUM_OF_OBJS; ++s)
		{
			assert(*vars[s] == s);
		}
	}

	inctst();
}
