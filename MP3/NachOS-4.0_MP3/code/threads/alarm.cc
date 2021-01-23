// alarm.cc
//	Routines to use a hardware timer device to provide a
//	software alarm clock.  For now, we just provide time-slicing.
//
//	Not completely implemented.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "alarm.h"
#include "main.h"

// 12/15 added.
// #include "stats.h"

//----------------------------------------------------------------------
// Alarm::Alarm
//      Initialize a software alarm clock.  Start up a timer device
//
//      "doRandom" -- if true, arrange for the hardware interrupts to 
//		occur at random, instead of fixed, intervals.
//----------------------------------------------------------------------

Alarm::Alarm(bool doRandom)
{
    timer = new Timer(doRandom, this);
}

//----------------------------------------------------------------------
// Alarm::CallBack
//	Software interrupt handler for the timer device. The timer device is
//	set up to interrupt the CPU periodically (once every TimerTicks).
//	This routine is called each time there is a timer interrupt,
//	with interrupts disabled.
//
//	Note that instead of calling Yield() directly (which would
//	suspend the interrupt handler, not the interrupted thread
//	which is what we wanted to context switch), we set a flag
//	so that once the interrupt handler is done, it will appear as 
//	if the interrupted thread called Yield at the point it is 
//	was interrupted.
//
//	For now, just provide time-slicing.  Only need to time slice 
//      if we're currently running something (in other words, not idle).
//----------------------------------------------------------------------

///////////// 12/12 /////////////
/*
    Here, when the 'Timer interrupt' occurs,
    Alarm::CallBack() is called.
    We can update threads' priorities here? (Aging)

    (1) Iterate through scheduler's L1~L3 queue, add threads' priority by 10?
    (2) After priority changes, all threads should be run: Re-arranging their queue.
    (3) For Yield() : A thread should run AddToQueue() instead of ReadyToRun().

*/

void 
Alarm::CallBack() 
{
    Interrupt *interrupt = kernel->interrupt;
    MachineStatus status = interrupt->getStatus();
    

    if (status != IdleMode) {

   	    interrupt->YieldOnReturn();
        /* -------------------------------
        // kernel->counter += TimerTicks;
        // // cout<<"Alarm CallBack\n";
        // //////////// 12/12 ////////////////////
        // if(kernel->counter >= 1500 ){
        //     // cout<<"Aging!\n";
        //     // (1)
        //     kernel->scheduler->AddThreadPriority();
        //     // (2)
        //     kernel->scheduler->ReArrangeThreads();
        //     kernel->counter = 0; //reset counter.
            
        //     // On Aging, Should Re-Scheduling?            
        //     // Only Re-schedule if 
        // }
        ///////////////////////////////////////
        ------------------------------------*/
    }
}
