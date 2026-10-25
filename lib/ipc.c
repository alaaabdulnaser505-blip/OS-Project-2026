/*
 * ipc.c
 *
 *  Created on: Sep 28, 2026
 *      Author: HP
 */
#include <inc/syscall.h>
#include <inc/lib.h>


/********************/
/*User IPC Functions*/
/********************/
int ipc_create_data_message(uint8 *data, uint16 length, struct Message* msg)
{
	//TODO: [PROJECT'26.IM#4] IPC#1 - [1] ipc_create_data_message
	//Your code is here
	//Comment the following line
	panic("ipc_create_data_message() is not implemented yet...!!");
	//Note: msg is a return param (passed by reference)

	return 0;
}
int ipc_send(uint32 destMailboxHandle, struct Message *msg)
{
	//TODO: [PROJECT'26.IM#4] IPC#1 - [4] ipc_send
	//Your code is here
	//Comment the following line
	panic("ipc_send() is not implemented yet...!!");

}
int ipc_receive(uint32 srcMailboxHandle, struct Message *msg)
{
	//TODO: [PROJECT'26.IM#4] IPC#1 - [6] ipc_receive
	//Your code is here
	//Comment the following line
	panic("ipc_receive() is not implemented yet...!!");

}
int ipc_broadcast(uint32 destMailboxHandle, struct Message *msg)
{
	//TODO: [PROJECT'26.IM#5] IPC#2 - [6] ipc_broadcast
	//Your code is here
	//Comment the following line
	panic("ipc_broadcast() is not implemented yet...!!");

}

/******************************************************************************/
//TODO: [PROJECT'26.IM#5] IPC#2 - [0] GIVEN FUNCTIONS
int ipc_create_mailbox(char *mailboxName)
{
	//panic("not implemented yet!");
	//1. If env list of handles is full, print error message and return E_MB_OPER_FAILED
	if (LIST_SIZE(&(myEnv->mailboxHandles.mailbox_handles_list)) == MB_MAX_HANDLES)
	{
		cprintf_colored(TEXT_ERROR_CLR, "[%d] run out of mailbox handles! can't create more\n", myEnv->env_id);
		return E_MB_OPER_FAILED;
	}
	//2. Call sys_ipc_create_mailbox()
	int ret = sys_ipc_create_mailbox(mailboxName);
	//3. If it returns error, return E_MB_OPER_FAILED. Else, return the value (i.e. mailboxHandle)
	if (ret < 0) return E_MB_OPER_FAILED;
	return ret;
}
int ipc_delete_mailbox(uint32 mailboxHandle)
{
	//panic("not implemented yet!");
	//1. Call sys_ipc_delete_mailbox()
	int ret = sys_ipc_delete_mailbox(mailboxHandle);
	//2. If it returns error, return E_MB_OPER_FAILED. Else, return 0
	if (ret < 0) return E_MB_OPER_FAILED;
	return 0;
}
int ipc_connect(uint32 ownerID, char *mailboxName, uint8 connectType)
{
	//panic("not implemented yet!");
	//1. If type = MB_OWNER, print error message and return E_MB_OPER_FAILED
	if (connectType == MB_OWNER)
	{
		cprintf_colored(TEXT_ERROR_CLR, "[%d] is already the owner of %x mailbox! can't connect to it\n", myEnv->env_id, mailboxName);
		return E_MB_OPER_FAILED;
	}
	//2. If env list of handles is full, print error message and return E_MB_OPER_FAILED
	if (LIST_SIZE(&(myEnv->mailboxHandles.mailbox_handles_list)) == MB_MAX_HANDLES)
	{
		cprintf_colored(TEXT_ERROR_CLR, "[%d] run out of mailbox handles! can't connect more\n", myEnv->env_id);
		return E_MB_OPER_FAILED;
	}
	//3. Call sys_ipc_connect()
	int ret = sys_ipc_connect(ownerID, mailboxName, connectType);
	//4. If it returns error, return E_MB_OPER_FAILED. Else, return the value (i.e. mailboxHandle)
	if (ret < 0) return E_MB_OPER_FAILED;
		return ret;
}

int ipc_disconnect(uint32 mailboxHandle)
{
	//panic("not implemented yet!");
	//1. Call sys_ipc_disconnect()
	int ret = sys_ipc_disconnect(mailboxHandle);
	//2. If it returns error, return E_MB_OPER_FAILED. Else, return 0
	if (ret < 0) return E_MB_OPER_FAILED;
	return 0;
}
/******************************************************************************/

/***********************/
/*User IPC System Calls*/
/***********************/
int sys_ipc_connect(uint32 ownerID, char *mailboxName, uint8 type)
{
	return syscall(SYS_ipc_connect, ownerID, (uint32)mailboxName, (uint32)type, 0, 0);
}
int sys_ipc_disconnect(uint32 mailboxHandle)
{
	return syscall(SYS_ipc_disconnect, mailboxHandle, 0, 0, 0, 0);
}
int sys_ipc_create_mailbox(char *mailboxName)
{
	return syscall(SYS_ipc_create_mailbox, (uint32)mailboxName, 0, 0, 0, 0);
}
int sys_ipc_delete_mailbox(uint32 mailboxHandle)
{
	return syscall(SYS_ipc_delete_mailbox, mailboxHandle, 0, 0, 0, 0);
}
int sys_ipc_send(uint32 destMailboxHandle, struct Message *msg)
{
	return syscall(SYS_ipc_send, destMailboxHandle, (uint32)msg, 0, 0, 0);
}
int sys_ipc_receive(uint32 srcMailboxHandle, struct Message *msg)
{
	return syscall(SYS_ipc_receive, srcMailboxHandle, (uint32)msg, 0, 0, 0);
}
int sys_ipc_broadcast(uint32 destMailboxHandle, struct Message *msg)
{
	return syscall(SYS_ipc_broadcast, destMailboxHandle, (uint32)msg, 0, 0, 0);
}
