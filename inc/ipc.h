/*
 * ipc.h
 *
 *  Created on: Sep 28, 2026
 *      Author: HP
 */

#ifndef INC_IPC_H_
#define INC_IPC_H_

#include <inc/types.h>
#include <inc/queue.h>
/******************************************************************************/
//TODO: [PROJECT'26.IM#4] IPC#1 - [0] GIVEN DATA
//TODO: [PROJECT'26.IM#5] IPC#2 - [0] GIVEN DATA
/*Message Types*/
#define MSG_DATA  		1
#define MSG_NULL 		2
#define MSG_SEND_INV_MB 3
#define MSG_RCV_INV_MB 	4
#define MSG_SHR_MEM		5
/*Connection Types*/
#define MB_OWNER  		1
#define MB_SENDER  		2
#define MB_RECEIVER 	3
/*Sizes*/
#define MSG_MAX_SIZE 		128 //max msg length
#define MB_MAX_MESSAGES		128	//max num of msg's in mailbox
#define MB_MAX_HANDLES 		32	//max num of handles for each env
#define MB_MAX_LIST_SIZE 	256	//max total num of mailboxes in OS
#define MB_MAX_SENDERS		64	//max num of connected senders per mailbox
#define MB_MAX_RECEIVERS 	64	//max num of connected receivers per mailbox
/*Errors*/
#define E_MB_NOT_EXIST 		-1
#define E_MB_EXIST 			-2
#define E_MB_LIST_FULL 		-3
#define E_MB_HANDLES_FULL	-4
#define E_MB_INV_OWNER		-5
#define E_MB_ENV_CONNECTED	-6
#define E_MB_INV_REQUEST	-7
#define E_MB_OPER_FAILED	-8
/*Message Data Structure*/
struct Message
{
	/*HEADER*/
	uint32 senderID;		//source Env ID
	uint32 receiverID;		//destination Env ID
	uint8 type;				//MSG_DATA, MSG_SEND_INV_MB, MSG_RCV_INV_MB
	/*BODY OF DATA MSG*/
	uint8 data[MSG_MAX_SIZE]; //data as stream of bytes
	uint16 length;			//length of data
	/*linked list links*/
	LIST_ENTRY(Message) prev_next_info;
} __attribute__((packed));

LIST_HEAD(MessagesList, Message);
/******************************************************************************/

/*User IPC Functions*/
int ipc_connect(uint32 ownerID, char *mailboxName, uint8 type);
int ipc_disconnect(uint32 mailboxHandle);
int ipc_create_mailbox(char *mailboxName);
int ipc_delete_mailbox(uint32 mailboxHandle);
int ipc_create_data_message(uint8 *data, uint16 length, struct Message* msg);
int ipc_send(uint32 destMailboxHandle, struct Message *msg);
int ipc_receive(uint32 srcMailboxHandle, struct Message *msg);
int ipc_broadcast(uint32 destMailboxHandle, struct Message *msg);

/*User IPC System Calls*/
int sys_ipc_connect(uint32 ownerID, char *mailboxName, uint8 type);
int sys_ipc_disconnect(uint32 mailboxHandle);
int sys_ipc_create_mailbox(char *mailboxName);
int sys_ipc_delete_mailbox(uint32 mailboxHandle);
int sys_ipc_send(uint32 destMailboxHandle, struct Message *msg);
int sys_ipc_receive(uint32 srcMailboxHandle, struct Message *msg);
int sys_ipc_broadcast(uint32 destMailboxHandle, struct Message *msg);
#endif /* INC_IPC_H_ */
