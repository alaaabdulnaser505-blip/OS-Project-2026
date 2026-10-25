/*
 * arrayOperations.h
 *
 *  Created on: Dec 3, 2025
 *      Author: HP
 */

#ifndef USER_ARRAYOPERATIONS_H_
#define USER_ARRAYOPERATIONS_H_

#define USE_KERN_SEMAPHORE 1

//Functions Declarations
void Swap(int *Elements, int First, int Second);
void PrintElements(int *Elements, int NumOfElements);

void Swap(int *Elements, int First, int Second)
{
	int Tmp = Elements[First] ;
	Elements[First] = Elements[Second] ;
	Elements[Second] = Tmp ;
}

void PrintElements(int *Elements, int NumOfElements)
{
	int i ;
	int NumsPerLine = 20 ;
	for (i = 0 ; i < NumOfElements-1 ; i++)
	{
		if (i%NumsPerLine == 0)
			cprintf("\n");
		cprintf("%d, ",Elements[i]);
	}
	cprintf("%d\n",Elements[i]);

}

#endif /* USER_ARRAYOPERATIONS_H_ */
