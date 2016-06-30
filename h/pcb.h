/*
 * pcb.h
 *
 *  Created on: May 11, 2016
 *      Author: OS1
 */

#ifndef PCB_H_
#define PCB_H_

#include "thread.h";
#include "semaphor.h"

class PCB {
public:

	unsigned ss;
	unsigned sp;
	unsigned bp;
	int flag; 		// 0-new, 1-ready, 2-blocked, 3-finished
	int id;
	unsigned *stack;

	Thread *myThread;
	Time timeSlice;
	StackSize stackSize;

	struct PCBlist {
		PCB *pcb;
		PCBlist *next;
		PCBlist(PCB *pcb, PCBlist* next = 0) : pcb(pcb), next(next) {}
	};

	PCBlist *waitingHead, *waitingTail;

	static PCBlist *allpcbHead, *allpcbTail;

	PCB(StackSize stackSize, Time timeSlice, Thread* thread);
	~PCB();

	int children;
	PCB *parent;
	Semaphore *waitChildren;

	static void wrapper();
	static void end();
	static void interrupt fork();
};

#endif /* PCB_H_ */
