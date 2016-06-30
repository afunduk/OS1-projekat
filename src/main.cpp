/*
 * main.cpp
 *
 *  Created on: May 25, 2016
 *      Author: OS1
 */

#include "thread.h"
#include "pcb.h"
#include "def.h"
#include "schedule.h"
#include "ksem.h"
#include <dos.h>
#include <iostream.h>

volatile PCB* running = 0;

volatile int counter = 0;
volatile int zahtevana_promena_konteksta = 0;

extern void tick();

typedef void interrupt(*pInterrupt)(...);
pInterrupt oldTimer;

//pomocne promenljive za prekid tajmera
unsigned tsp;
unsigned tss;
unsigned tbp;

class Idle : public Thread {
public:
	Idle() : Thread(256, 1) { pcb = myPCB; }

	void run() { while (1); }

	PCB* pcb;
};

volatile Idle* idle = 0;


void interrupt timer(...) {

	//ako je ovo pozvao CPU, izmeri vreme
	if (zahtevana_promena_konteksta == 0) {
		counter--;

		if (KernelSem::allSemHead != 0) {
			KernelSem::SemList *tmp = KernelSem::allSemHead;
			while (tmp != 0) {
				if (tmp->sem->blocked != 0) {

					KernelSem::PCBsleeping *tmpsleeping = tmp->sem->blocked;
					KernelSem::PCBsleeping *prevsleeping = 0;

					if (tmpsleeping->sleep == 0) {
						while (tmpsleeping != 0 && tmpsleeping->sleep == 0) {
							prevsleeping = tmpsleeping;
							tmpsleeping = tmpsleeping->next;
						}
					}
					if (tmpsleeping != 0) {

						tmpsleeping->sleep--;

						if (tmpsleeping->sleep == 0) {
							KernelSem::PCBsleeping *destroy = 0;

							while (tmpsleeping->sleep == 0) {

								PCB *ready = tmpsleeping->pcb;
								ready->flag = READY;
								Scheduler::put(ready);

								tmp->sem->sig = 0;    // wait vraca 0

								if (prevsleeping == 0) tmp->sem->blocked = tmp->sem->blocked->next;
								else prevsleeping->next = tmpsleeping->next;

								destroy = tmpsleeping;
								tmpsleeping = tmpsleeping->next;
								delete destroy;
							}
						}
					}
				}
				tmp = tmp->next;
			}
		}

		oldTimer();
		tick();
	}
	//ako je ovo pozvano iz dispatch() ili ako je vreme isteklo, promeni kontekst
	if (zahtevana_promena_konteksta == 1 || (running->timeSlice > 0 && counter == 0)) {

		zahtevana_promena_konteksta = 0;

#ifndef BCC_BLOCK_IGNORE

		asm{
			mov tsp, sp
			mov tss, ss
			mov tbp, bp
		}

#endif

		running->sp = tsp;
		running->ss = tss;
		running->bp = tbp;

		if (running->flag == 1 && running != idle->pcb) Scheduler::put((PCB*)running); //ako je ready

		running = Scheduler::get();

		if (running == 0) running = idle->pcb; // ako scheduler prazan idle nit

		tsp = running->sp;
		tss = running->ss;
		tbp = running->bp;

#ifndef BCC_BLOCK_IGNORE

		asm{
			mov sp, tsp
			mov ss, tss
			mov bp, tbp
		}

#endif

		counter = running->timeSlice;
	}
}

void inic() {
#ifndef BCC_BLOCK_IGNORE

	oldTimer = getvect(8);
	setvect(8, timer);

#endif
}

void restore() {
#ifndef BCC_BLOCK_IGNORE

	setvect(8, oldTimer);

#endif
}


extern int userMain(int argc, char* argv[]);

class Main : public Thread {
	int argc;
	char **argv;
public:

	Main(int argc, char **argv) : Thread(defaultStackSize, defaultTimeSlice), argc(argc), argv(argv) {}

	int value;
	void run() { value = userMain(argc, argv); }

	~Main() { waitToComplete(); }
};

int main(int argc, char *argv[]) {
	inic();

	idle = new Idle();
	((Thread*)idle)->start();

	running = new PCB(0, 0, 0);

	Main *mainThread = new Main(argc, argv);
	mainThread->start();

	delete mainThread;
	delete idle;

	restore();
	return 0;
}

