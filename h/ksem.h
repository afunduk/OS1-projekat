/*
 * ksem.h
 *
 *  Created on: May 25, 2016
 *      Author: OS1
 */

#ifndef KSEM_H_
#define KSEM_H_

#include "semaphor.h"

typedef unsigned int Time;

class PCB;

class KernelSem {
public:
	struct PCBsleeping {
		int sleep;
		PCB *pcb;
		PCBsleeping *next;
		PCBsleeping(PCB* p, Time t, PCBsleeping* n = 0) : pcb(p), next(n), sleep(t) {}
	};

	struct SemList {
		KernelSem *sem;
		SemList *next;
		SemList(KernelSem *sem, SemList *next = 0) : sem(sem), next(next) {}
	};

	static SemList *allSemHead, *allSemTail;

	KernelSem(int val);
	~KernelSem();

	void block(Time t, PCB *running);
	void deblock();

private:
	int val;

	int sig;

	PCBsleeping *blocked;

	friend class Semaphore;
	friend void interrupt timer(...);
};

#endif /* KSEM_H_ */
