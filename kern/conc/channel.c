/*
 * channel.c
 *
 *  Created on: Sep 22, 2024
 *      Author: HP
 */
#include "channel.h"
#include <kern/proc/user_environment.h>
#include <kern/cpu/sched.h>
#include <kern/cpu/cpu.h>
#include <inc/string.h>
#include <inc/disk.h>

/************************************************************************************/
//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//
//TODO: [PROJECT'26.GM#4] BLOCKING - [0] GIVEN FUNCTIONS
//===============================
// 1) INITIALIZE THE CHANNEL:
//===============================
// initialize its lock & queue
void init_channel(struct Channel *chan, char *name)
{
	strcpy(chan->name, name);
	init_queue(&(chan->queue));
}
/************************************************************************************/

//=====================================================================================//
//============================== REQUIRED FUNCTIONS ===================================//
//=====================================================================================//
//===============================
// 2) SLEEP ON A GIVEN CHANNEL:
//===============================
// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
// Ref: xv6-x86 OS code
void sleep(struct Channel *chan, struct kspinlock* lk)
{
	//TODO: [PROJECT'26.GM#4] BLOCKING - [13] Channel sleep
	//Your code is here
	struct Env *crr_process;
	int isQLock;

	if (chan == NULL || lk == NULL)
		panic("sleep: NULL channel or NULL lock is given");

	crr_process = get_cpu_proc();
	if (crr_process == NULL)
		panic("sleep: no running process to block");

	isQLock = (lk == &ProcessQueues.qlock);
	if (!isQLock)
	{
		acquire_kspinlock(&ProcessQueues.qlock);
		release_kspinlock(lk);
	}

	crr_process->channel = chan;
	crr_process->env_status = ENV_BLOCKED;
	enqueue(&chan->queue, crr_process);

	sched();

	crr_process->channel = NULL;

	if (!isQLock)
	{
		release_kspinlock(&ProcessQueues.qlock);
		acquire_kspinlock(lk);
	}
	//Comment the following line
	//panic("sleep() is not implemented yet...!!");
}

//==================================================
// 3) WAKEUP ONE BLOCKED PROCESS ON A GIVEN CHANNEL:
//==================================================
// Wake up ONE process sleeping on chan.
// The qlock must be held.
// Ref: xv6-x86 OS code
// chan MUST be of type "struct Env_Queue" to hold the blocked processes
void wakeup_one(struct Channel *chan)
{
	//TODO: [PROJECT'26.GM#4] BLOCKING - [14] Channel wakeup_one
	//Your code is here
	struct Env *targetEnv;
	int mmsook;

	if (chan == NULL)
		panic("mfeesh channel");

	mmsook = holding_kspinlock(&ProcessQueues.qlock);
	if (!mmsook)
		acquire_kspinlock(&ProcessQueues.qlock);

	targetEnv = dequeue(&chan->queue);
	if (targetEnv != NULL)
	{
		sched_insert_ready(targetEnv);
	}

	if (!mmsook)
		release_kspinlock(&ProcessQueues.qlock);
	//Comment the following line
	//panic("wakeup_one() is not implemented yet...!!");
}

//====================================================
// 4) WAKEUP ALL BLOCKED PROCESSES ON A GIVEN CHANNEL:
//====================================================
// Wake up all processes sleeping on chan.
// The queues lock must be held.
// Ref: xv6-x86 OS code
// chan MUST be of type "struct Env_Queue" to hold the blocked processes
void wakeup_all(struct Channel *chan)
{
	//TODO: [PROJECT'26.GM#4] BLOCKING - [15] Channel wakeup_all
	//Your code is here
	struct Env *targetEnv;
		int mmsook;

		if (chan == NULL)
			panic("mfeesh channel");

		mmsook = holding_kspinlock(&ProcessQueues.qlock);
		if (!mmsook)
			acquire_kspinlock(&ProcessQueues.qlock);

		for (targetEnv = dequeue(&chan->queue); targetEnv != NULL; targetEnv = dequeue(&chan->queue))
		{
			sched_insert_ready(targetEnv);
		}

		if (!mmsook)
			release_kspinlock(&ProcessQueues.qlock);
	//Comment the following line
	//panic("wakeup_all() is not implemented yet...!!");
}

