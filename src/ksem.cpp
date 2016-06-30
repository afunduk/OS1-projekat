/*
 * ksem.cpp
 *
 *  Created on: May 25, 2016
 *      Author: OS1
 */

#include "ksem.h"
#include "def.h"
#include "pcb.h"
#include "schedule.h"
#include <iostream.h>

typedef unsigned int Time;

extern volatile PCB *running;

void KernelSem::block(Time t, PCB *running) {
	int sum, sub;
	PCBsleeping *curr, *prev;

	running->flag = BLOCKED; // status = blocked;
	PCBsleeping *newPCBsleeping = new PCBsleeping(running, t);

	if (blocked == 0) blocked = newPCBsleeping;
	else {
		curr = blocked; sum = curr->sleep; prev = 0;
		while (sum <= t && curr != 0) {
			prev = curr; curr = curr->next; if (curr != 0) sum += curr->sleep;
		}
		if (prev == 0) { //umetanje na pocetak
			blocked = newPCBsleeping;
			newPCBsleeping->next = curr;
			curr->sleep -= t;
		}
		else {
			if (curr == 0) { //umetanje na kraj
				prev->next = newPCBsleeping;
				newPCBsleeping->sleep -= sum;
			}				//umetanje na sredinu
			else {
				prev->next = newPCBsleeping;
				newPCBsleeping->next = curr;
				sub = t + curr->sleep - sum;
				newPCBsleeping->sleep = sub;
				curr->sleep -= sub;
			}
		}
	}
	/*
	cout << "Blokirani: " << endl;
	curr = blocked;
	while (curr != 0) {
		cout << curr->pcb->id << "(" << curr->sleep << ") ";
		curr = curr->next;
	}
	cout << endl;
	*/
	dispatch();
}

void KernelSem::deblock() {
	PCBsleeping *destroy = blocked;
	if (blocked != 0) {

		blocked->pcb->flag = READY; //status = ready
		Scheduler::put(blocked->pcb);

		if (blocked->next != 0) blocked->next->sleep += blocked->sleep;
		blocked = blocked->next;
		delete destroy;
	}
}

KernelSem::SemList *KernelSem::allSemHead = 0;
KernelSem::SemList *KernelSem::allSemTail = 0;

KernelSem::KernelSem(int val) : val(val), blocked(0), sig(0) {
	//dodaj u listu svih semafora
	SemList *newSemList = new SemList(this);
	if (allSemHead == 0) allSemHead = newSemList;
	else allSemTail->next = newSemList;
	allSemTail = newSemList;
}

KernelSem::~KernelSem() {
	//skini iz liste svih semafora
	SemList *tmp = allSemHead;
	SemList *prev = 0;
	while (tmp != 0) {
		if (this == tmp->sem) break;
		prev = tmp;
		tmp = tmp->next;
	}
	//skidanje sa pocetka
	if (prev == 0) {
		allSemHead = allSemHead->next;
	}
	else {
		//skidanje sa kraja i sredine
		prev->next = tmp->next;
	}
	delete tmp;
	if (allSemHead == 0) allSemTail = 0;

	//ubaciti preostale blokirane niti u scheduler
	while (blocked != 0) deblock();
}


