/*
 * ipc.c
 *
 *  Created on: Sep 28, 2026
 *      Author: HP
 */
#include <kern/ipc/ipc.h>


//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
/******************************************************************************/
//TODO: [PROJECT'26.IM#5] IPC#2 - [0] GIVEN FUNCTIONS
//=============================
// [1] Allocate MailboxHandle:
//=============================
//Allocate MailboxHandle object from the local storage of the given env
//Return:
//	a) if found: new MailboxHandle object
//	b) else: NULL
struct MailboxHandle* allocate_mailbox_handle(struct Env* e)
{
	struct MailboxHandle* handle = LIST_FIRST(&(e->__fmbhl__));
	if (handle == NULL) return NULL;
	LIST_REMOVE(&(e->__fmbhl__), handle);
	return handle;
}
//===========================
// [2] Free MailboxHandle:
//===========================
//Free MailboxHandle object by adding it to the local storage of the given env
void free_mailbox_handle(struct Env* e, struct MailboxHandle* mailboxHandle)
{
	if (mailboxHandle == NULL) return ;
	memset(mailboxHandle, 0, sizeof(struct MailboxHandle));
	LIST_INSERT_HEAD(&(e->__fmbhl__), mailboxHandle);
}
/******************************************************************************/

//=====================================================================================//
//============================== REQUIRED FUNCTIONS ===================================//
//=====================================================================================//

/**********************/
/*Kernel IPC Functions*/
/**********************/
int ipc_create_env_mailbox(struct Env *e, uint32 mailboxSize)
{
	//TODO: [PROJECT'26.IM#4] IPC#1 - [2] ipc_create_env_mailbox
	//Your code is here
	//Comment the following line
	warn("Not Implemented Yet!");

	//Note: this function already called in initialize_environment(...)
	return 0;
}
int ipc_delete_env_mailbox(struct Env *e)
{
	//TODO: [PROJECT'26.IM#4] IPC#1 - [3] ipc_delete_env_mailbox
	//Your code is here
	//Comment the following line
	panic("ipc_delete_env_mailbox() is not implemented yet...!!");
}
int ipc_get_mailbox(uint32 mailboxHandle, struct Mailbox *mb)
{
	//TODO: [PROJECT'26.IM#5] IPC#2 - [5] ipc_get_mailbox
	//Your code is here
	//Comment the following line
	panic("ipc_get_mailbox() is not implemented yet...!!");

	/* Get mailbox object using the info from the given handle
	 * "mb" is a return param (passed by reference)
	 */
}

/*************************/
/*Kernel IPC System Calls*/
/*************************/
int sys_ipc_send(uint32 destMailboxHandle, struct Message *msg)
{
	//TODO: [PROJECT'26.IM#4] IPC#1 - [5] sys_ipc_send
	//Your code is here
	//Comment the following line
	panic("sys_ipc_send() is not implemented yet...!!");

}
int sys_ipc_receive(uint32 srcMailboxHandle, struct Message *msg)
{
	//TODO: [PROJECT'26.IM#4] IPC#1 - [7] sys_ipc_receive
	//Your code is here
	//Comment the following line
	panic("sys_ipc_receive() is not implemented yet...!!");

}
int sys_ipc_create_mailbox(char *mailboxName)
{
	//TODO: [PROJECT'26.IM#5] IPC#2 - [1] sys_ipc_create_mailbox
	//Your code is here
	//Comment the following line
	panic("sys_ipc_create_mailbox() is not implemented yet...!!");
}

int sys_ipc_delete_mailbox(uint32 mailboxHandle)
{
	//TODO: [PROJECT'26.IM#5] IPC#2 - [2] sys_ipc_delete_mailbox
	//Your code is here
	//Comment the following line
	panic("sys_ipc_delete_mailbox() is not implemented yet...!!");
}

int sys_ipc_connect(uint32 ownerID, char *mailboxName, uint8 connectType)
{
	//TODO: [PROJECT'26.IM#5] IPC#2 - [3] sys_ipc_connect
	//Your code is here
	//Comment the following line
	panic("sys_ipc_connect() is not implemented yet...!!");
}

int sys_ipc_disconnect(uint32 mailboxHandle)
{
	//TODO: [PROJECT'26.IM#5] IPC#2 - [4] sys_ipc_disconnect
	//Your code is here
	//Comment the following line
	panic("sys_ipc_disconnect() is not implemented yet...!!");
}

int sys_ipc_broadcast(uint32 destMailboxHandle, struct Message *msg)
{
	//TODO: [PROJECT'26.IM#5] IPC#2 - [7] sys_ipc_broadcast
	//Your code is here
	//Comment the following line
	panic("sys_ipc_broadcast() is not implemented yet...!!");
}
