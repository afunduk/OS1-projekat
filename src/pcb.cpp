/*
 * pcb.cpp
 *
 *  Created on: May 25, 2016
 *      Author: OS1
 */

#include "thread.h"
#include "pcb.h"
#include "def.h"
#include <dos.h>
#include "schedule.h"
#include "iostream.h"

//inic statickih polja - list svih pcb-ova
PCB::PCBlist *PCB::allpcbHead = 0;
PCB::PCBlist *PCB::allpcbTail = 0;

PCB::PCB(StackSize stackSz, Time timeSlice, Thread *thread) : stackSize(stackSz), timeSlice(timeSlice), myThread(thread) {
	//predlazem da se odlozi stvaranje stack-a u thread->start() umesto u konstruktoru
	if (stackSize > 0) {

		stackSize = stackSize / sizeof(unsigned);
		stack = new unsigned[stackSize];

		stack[stackSize - 1] = 0x200;
		stack[stackSize - 12] = 0; // bp na vrhu stack-a = 0

#ifndef BCC_BLOCK_IGNORE

		stack[stackSize - 2] = FP_SEG(PCB::wrapper);
		stack[stackSize - 3] = FP_OFF(PCB::wrapper);

		ss = FP_SEG(stack + stackSize - 12);
		sp = FP_OFF(stack + stackSize - 12);

#endif

		bp = sp;
	}
	else stack = 0;

	flag = NEW;
	waitingHead = 0;
	waitingTail = 0;

	children = 0;// broj dece
	parent = 0; //pokazivac na oca
	waitChildren = new Semaphore(0);

	//umetanje u listu svih pcb-ova
	PCBlist *newPCBlist = new PCBlist(this);
	if (allpcbHead == 0) allpcbHead = newPCBlist;
	else allpcbTail->next = newPCBlist;
	allpcbTail = newPCBlist;
}

PCB::~PCB() {
	PCBlist *tmp = allpcbHead;
	PCBlist *prev = 0;
	while (tmp != 0) {
		if (id == tmp->pcb->id) break;
		prev = tmp;
		tmp = tmp->next;
	}
	//skidanje sa pocetka
	if (prev == 0) {
		allpcbHead = allpcbHead->next;
	}
	else {
	//skidanje sa kraja i sredine
		prev->next = tmp->next;
	}
	delete tmp;
	if (allpcbHead == 0) allpcbTail = 0;

	delete[] stack;
}

extern volatile PCB* running;

void PCB::wrapper() {
	//run niti
	running->myThread->run();
	//zavrsi nit
	end();
}

void PCB::end() {
#ifndef BCC_BLOCK_IGNORE
	lock
#endif
	PCBlist *tmp, *destroy;
	// nit je zavrsila - ne treba da se stavi u sheduler
	running->flag = FINISHED;

	//signaliziraj oca koj ceka da se deca zavrse
	if (running->parent != 0) {
		running->parent->waitChildren->signal();
	}

	//sve niti koje su cekale ovu nit mogu u scheduler
	if (running->waitingHead != 0) {

		tmp = running->waitingHead;
		while (tmp != 0) {

			tmp->pcb->flag = READY;
			Scheduler::put(tmp->pcb);

			destroy = tmp;
			tmp = tmp->next;
			delete destroy;

		}
		running->waitingHead = 0;
		running->waitingTail = 0;

	}
#ifndef BCC_BLOCK_IGNORE
	unlock
#endif
	//promena konteksta
	dispatch();
}

unsigned offsetParent;
unsigned offsetChild;
unsigned *stackPointerParent;
unsigned *stackPointerChild;

unsigned tmpbp;
unsigned tmpss;

extern volatile PCB *childPCB;

void interrupt PCB::fork() {

	delete[] childPCB->stack;
	size_t size = running->stackSize;
	childPCB->stack = new unsigned[size];
	childPCB->stackSize = running->stackSize;
	childPCB->timeSlice = running->timeSlice;
	childPCB->parent = (PCB*)running;
	size *= sizeof(unsigned);

	//kopiraj stack
	memcpy((void*)childPCB->stack, (void*)running->stack, size);

#ifndef BCC_BLOCK_IGNORE

	asm {
		mov tmpss, ss
		mov tmpbp, bp
	}

	childPCB->ss = FP_SEG(childPCB->stack);

	offsetParent = FP_OFF(running->stack);
	offsetChild = FP_OFF(childPCB->stack);
	childPCB->bp = childPCB->sp = tmpbp - offsetParent + offsetChild;

	offsetParent = tmpbp;
	offsetChild = childPCB->sp;
	
	while (1) {
		stackPointerParent = (unsigned*)MK_FP(tmpss, offsetParent);
		stackPointerChild = (unsigned*)MK_FP(childPCB->ss, offsetChild);
		if (*stackPointerParent == 0) {
			*stackPointerChild = 0;
			break;
		}
		*stackPointerChild = *stackPointerParent - offsetParent + offsetChild;
		offsetParent = *stackPointerParent;
		offsetChild = *stackPointerChild;
	}

#endif

	childPCB->myThread->start();
}
