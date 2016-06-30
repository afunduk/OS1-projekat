/*
 * ivte.h
 *
 *  Created on: May 25, 2016
 *      Author: OS1
 */

#ifndef IVTE_H_
#define IVTE_H_

typedef unsigned char IVTNo;

typedef void interrupt(*pInterrupt)(...);

class IVTEntry {
public:

	IVTEntry(IVTNo ivtNo, pInterrupt newRoutine);
	~IVTEntry();

	void signal();
	void callold();

	static IVTEntry* IVT[256];
	static IVTEntry* getEntry(IVTNo i);

private:
	pInterrupt oldRoutine;
	IVTNo ivtNo;
};

#define PREPAREENTRY(i,callOld)\
	void interrupt inter##i(...);\
	IVTEntry e##i = IVTEntry(i,inter##i);\
	void interrupt inter##i(...){\
		e##i.signal();\
		if (callOld == 1)\
			e##i.callold();\
	}

#endif /* IVTE_H_ */
