/*
 * ivte.cpp
 *
 *  Created on: May 25, 2016
 *      Author: OS1
 */

#include "ivte.h"
#include <dos.h>
#include "kev.h"

IVTEntry *IVTEntry::IVT[256] = { 0 };

IVTEntry::IVTEntry(IVTNo ivtNo, pInterrupt newRoutine) : ivtNo(ivtNo) {
	if (ivtNo >= 0 && ivtNo < 256) {
		if (IVT[ivtNo] == 0) {
			IVT[ivtNo] = this;
#ifndef BCC_BLOCK_IGNORE

			oldRoutine = getvect(ivtNo);
			setvect(ivtNo, newRoutine);

#endif
		}
	}
}

IVTEntry::~IVTEntry() {

#ifndef BCC_BLOCK_IGNORE

	setvect(ivtNo, oldRoutine);

#endif

	IVT[ivtNo] = 0;
}

IVTEntry* IVTEntry::getEntry(IVTNo i) {
	if (i >= 0 && i < 256) return IVT[i];
	else return 0;
}

void IVTEntry::signal() {
	if (KernelEv::events[ivtNo] != 0) KernelEv::events[ivtNo]->signal();
}

void IVTEntry::callold() { oldRoutine(); }


