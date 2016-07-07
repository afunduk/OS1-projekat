/*
 * thread.cpp
 *
 *  Created on: May 25, 2016
 *      Author: OS1
 */

#include "thread.h"
#include "pcb.h"
#include "def.h"
#include "schedule.h"
#include "iostream.h"
#include <dos.h>

volatile int IDgenerator = 0;

Thread::Thread(StackSize stackSize, Time timeSlice) {
#ifndef BCC_BLOCK_IGNORE
	lock
#endif

	myPCB = new PCB(stackSize, timeSlice, this);
	myPCB->id = IDgenerator++;

#ifndef BCC_BLOCK_IGNORE
	unlock
#endif
}

Thread::~Thread() {
#ifndef BCC_BLOCK_IGNORE
	lock
#endif

	delete myPCB;

#ifndef BCC_BLOCK_IGNORE
	unlock
#endif
}

void Thread::start() {
	if (myPCB->flag == NEW) {

#ifndef BCC_BLOCK_IGNORE
		lock
#endif

		myPCB->flag = READY;
		Scheduler::put(myPCB);

#ifndef BCC_BLOCK_IGNORE
		unlock
#endif
	}
}

extern volatile PCB *running;

void Thread::waitToComplete() {
#ifndef BCC_BLOCK_IGNORE
		lock
#endif

		if (myPCB->flag != FINISHED && myPCB != (PCB*)running) {

			PCB::PCBlist *newPCBlist = new PCB::PCBlist((PCB*)running);
			running->flag = BLOCKED;

			if (myPCB->waitingHead == 0) myPCB->waitingHead = newPCBlist;
			else myPCB->waitingTail->next = newPCBlist;
			myPCB->waitingTail = newPCBlist;

			dispatch();
		}

#ifndef BCC_BLOCK_IGNORE
		unlock
#endif
}

ID Thread::getId() { return myPCB->id; }

ID Thread::getRunningId() {	return running->id; }

Thread* Thread::getThreadById(ID id) {
#ifndef BCC_BLOCK_IGNORE
		lock
#endif

		PCB::PCBlist *tmp = PCB::allpcbHead;
		Thread *thread = 0;

		while (tmp != 0) {
			if (tmp->pcb->id == id) {
				thread = tmp->pcb->myThread;
				break;
			}
			tmp = tmp->next;
		}

#ifndef BCC_BLOCK_IGNORE
		unlock
#endif
		return thread;
}

extern void interrupt timer(...);
extern int zahtevana_promena_konteksta;

void dispatch() {
#ifndef BCC_BLOCK_IGNORE
		lock
#endif

		zahtevana_promena_konteksta = 1;
		timer();

#ifndef BCC_BLOCK_IGNORE
		unlock
#endif
}

void Thread::exit() {

		//cout << "Gasim nit id: " << running->id << endl;
		if (running->flag != FINISHED) PCB::end();

}

void Thread::waitForForkChildren() {

	while (running->children > 0) {
		running->waitChildren->wait(0);
		running->children--;
	}

}

Thread* Thread::clone() const {	return new Thread(); }

volatile ID childId;
volatile PCB *childPCB;

ID Thread::fork() {
#ifndef BCC_BLOCK_IGNORE
		lock
#endif

		Thread *child = 0;
		child = running->myThread->clone();

		if (child == 0) {
#ifndef BCC_BLOCK_IGNORE
			unlock
#endif
			return -1;
		}

		//cout << "kreirao dete id: " << child->myPCB->id << endl;

		running->children++;
		childId = child->myPCB->id;

		childPCB = child->myPCB;
		PCB::fork();

		//cout << "izasao iz pcb::forka id: " << running->id << endl;

#ifndef BCC_BLOCK_IGNORE
		unlock
#endif
		if (running->id == childId) return 0;
		else return childId;
}

