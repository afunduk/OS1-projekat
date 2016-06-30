/*
 * sem.cpp
 *
 *  Created on: May 25, 2016
 *      Author: OS1
 */

#include "def.h"
#include "ksem.h"
#include "semaphor.h"

Semaphore::Semaphore(int i) {
#ifndef BCC_BLOCK_IGNORE
		lock
#endif

	myImpl = new KernelSem(i);

#ifndef BCC_BLOCK_IGNORE
		unlock
#endif
}

Semaphore::~Semaphore() {
#ifndef BCC_BLOCK_IGNORE
		lock
#endif

	delete myImpl;

#ifndef BCC_BLOCK_IGNORE
		unlock
#endif
}

int Semaphore::val() const { return myImpl->val; }

extern volatile PCB *running;

int Semaphore::wait(Time maxTimeToWait) {
#ifndef BCC_BLOCK_IGNORE
		lock
#endif

	if (--myImpl->val < 0) myImpl->block(maxTimeToWait,(PCB*) running);

#ifndef BCC_BLOCK_IGNORE
		unlock
#endif

	return myImpl->sig;
}

void Semaphore::signal() {
#ifndef BCC_BLOCK_IGNORE
		lock
#endif

	if (myImpl->val++ < 0) myImpl->deblock();

	myImpl->sig = 1;		//wait vraca 1

#ifndef BCC_BLOCK_IGNORE
		unlock
#endif
}

