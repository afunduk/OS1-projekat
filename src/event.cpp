/*
 * event.cpp
 *
 *  Created on: May 25, 2016
 *      Author: OS1
 */

#include "kev.h"
#include "event.h"
#include "def.h"
#include "iostream.h"

Event::Event(IVTNo ivtNo) {
#ifndef BCC_BLOCK_IGNORE
	lock
#endif
	myImpl = new KernelEv(ivtNo, this);
#ifndef BCC_BLOCK_IGNORE
	unlock
#endif
}

Event::~Event() {
#ifndef BCC_BLOCK_IGNORE
	lock
#endif
	delete myImpl;
#ifndef BCC_BLOCK_IGNORE
	unlock
#endif
}

void Event::wait() { myImpl->wait(); }

void Event::signal() { if (myImpl != 0) myImpl->signal(); }


