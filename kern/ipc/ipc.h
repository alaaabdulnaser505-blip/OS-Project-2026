/*
 * ipc.h
 *
 *  Created on: Sep 28, 2026
 *      Author: HP
 */

#ifndef KERN_IPC_IPC_H_
#define KERN_IPC_IPC_H_
#include <inc/ipc.h>
#include <inc/environment_definitions.h>
#include <inc/stdio.h>
#include <kern/conc/channel.h>
#include <kern/conc/kspinlock.h>

/******************************************************************************/
//TODO: [PROJECT'26.IM#4] IPC#1 - [0] GIVEN DATA
//TODO: [PROJECT'26.IM#5] IPC#2 - [0] GIVEN DATA
/*Mail Box Data Structure*/
struct Mailbox
{
	uint32 ownerID;					//Env ID of the owner
	char name[NAMELEN];				//Name of mailbox
	struct MessagesList messages;	//List of messages
	int msgsCnt;					//Actual number of messages
	int msgsMaxCnt;					//Mailbox capacity
	uint32 senders[MB_MAX_SENDERS];	//Env ID of all registered (connected) senders
	uint32 receivers[MB_MAX_RECEIVERS];//Env ID of all registered (connected) receivers
	struct Channel blockedSenders;	//Queue of blocked senders
	struct Channel blockedReceivers;//Queue of blocked receivers
	struct kspinlock lk;			//Lock to protect the mailbox
	/*linked list links*/
	LIST_ENTRY(Mailbox) prev_next_info;
} __attribute__((packed));
LIST_HEAD(MailBoxList, Mailbox);
struct MailBoxList mailbox_list;
/******************************************************************************/

struct MailboxHandle* allocate_mailbox_handle(struct Env* e);
void free_mailbox_handle(struct Env* e, struct MailboxHandle* mailboxHandle);

/*Kernel IPC Functions*/
int ipc_create_env_mailbox(struct Env *e, uint32 size);
int ipc_delete_env_mailbox(struct Env *e);
int ipc_get_mailbox(uint32 mailboxHandle, struct Mailbox *mb);

/*Kernel IPC System Calls*/
int sys_ipc_connect(uint32 ownerID, char *mailboxName, uint8 type);
int sys_ipc_disconnect(uint32 mailboxHandle);
int sys_ipc_create_mailbox(char *mailboxName);
int sys_ipc_delete_mailbox(uint32 mailboxHandle);
int sys_ipc_send(uint32 destMailboxHandle, struct Message *msg);
int sys_ipc_receive(uint32 srcMailboxHandle, struct Message *msg);
int sys_ipc_broadcast(uint32 destMailboxHandle, struct Message *msg);

#endif /* KERN_IPC_IPC_H_ */
