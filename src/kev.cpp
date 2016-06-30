/*
 * kev.cpp
 *
 *  Created on: May 25, 2016
 *      Author: OS1
 */

#include "kev.h"
#include "pcb.h"
#include "def.h"
#include "schedule.h"

KernelEv *KernelEv::events[256] = { 0 };

extern volatile PCB* running;

KernelEv::KernelEv(IVTNo i, Event *e) : ivtNo(i), myEvent(e), val(0), creator((PCB*)running), isBlocked(0) {
	if (i >= 0 && i < 256) events[i] = this;
}

KernelEv::~KernelEv() {	events[ivtNo] = 0; }

void KernelEv::wait() {
	if ((PCB*)running == creator) {
		if (val == 1) val = 0;
		else {
			isBlocked = 1;
			creator->flag = BLOCKED;
			dispatch();
		}
	}
}

void KernelEv::signal() {
	if (isBlocked == 0) val = 1;
	else {
		creator->flag = READY;
		Scheduler::put(creator);
		isBlocked = 0;
	}
}
