/*
 * kev.h
 *
 *  Created on: May 25, 2016
 *      Author: OS1
 */

#ifndef KEV_H_
#define KEV_H_

#include "event.h"

class PCB;

class KernelEv {
public:

	static KernelEv *events[256];

	KernelEv(IVTNo i, Event *e);
	~KernelEv();

	void wait();
	void signal();

private:
	int val;
	int isBlocked;

	IVTNo ivtNo;

	PCB *creator;

	Event *myEvent;

	friend class Event;
};

#endif /* KEV_H_ */
