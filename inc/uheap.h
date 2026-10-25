#ifndef FOS_INC_UHEAP_H
#define FOS_INC_UHEAP_H 1

//Values for user heap placement strategy
#define UHP_PLACE_FIRSTFIT 	0x1
#define UHP_PLACE_BESTFIT 	0x2
#define UHP_PLACE_NEXTFIT 	0x3
#define UHP_PLACE_WORSTFIT 	0x4
#define UHP_PLACE_CUSTOMFIT 0x5

/*******************************************************************/
//TODO: [PROJECT'26.IM#2] USER HEAP - [0] GIVEN DATA
uint32 uheapPageAllocStart ;
uint32 uheapPageAllocBreak ;
uint32 uheapPlaceStrategy ;
/*******************************************************************/

void *malloc(uint32 size);
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable);
void* sget(int32 ownerEnvID, char *sharedVarName);
void free(void* virtual_address);
void sfree(void* virtual_address);

#endif
