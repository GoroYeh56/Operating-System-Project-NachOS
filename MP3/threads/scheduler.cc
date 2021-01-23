// scheduler.cc 
//	Routines to choose the next thread to run, and to dispatch to
//	that thread.
//
// 	These routines assume that interrupts are already disabled.
//	If interrupts are disabled, we can assume mutual exclusion
//	(since we are on a uniprocessor).
//
// 	NOTE: We can't use Locks to provide mutual exclusion here, since
// 	if we needed to wait for a lock, and the lock was busy, we would 
//	end up calling FindNextToRun(), and that would put us in an 
//	infinite loop.
//
// 	Very simple implementation -- no priorities, straight FIFO.
//	Might need to be improved in later assignments.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "debug.h"
#include "scheduler.h"
#include "main.h"

//----------------------------------------------------------------------
// Scheduler::Scheduler
// 	Initialize the list of ready but not running threads.
//	Initially, no ready threads.
//----------------------------------------------------------------------

/////////////// 12/12 Added ////////////////////
//----------------------------------------------------------------------
// SchedulingCompare
//	Compare to thread's priority and put them in decreasing order.
//  First thread should have the highest priority (149)
//----------------------------------------------------------------------

static int
SchedulingCompare (Thread *x, Thread *y)
{
    if (x->priority < y->priority) { return 1; }
    else if (x->priority > y->priority) { return -1; }
    else { return 0; }
}


Scheduler::Scheduler()
{ 
    readyList = new List<Thread *>; 
    toBeDestroyed = NULL;

    //////// 12/10 Added ////////
    
    L1List = new SortedList<Thread*>(SchedulingCompare);
    L2List = new SortedList<Thread*>(SchedulingCompare);
    // L1List = new List<Thread *>;
    // L2List = new List<Thread *>;
    L3List = new List<Thread *>;
    

} 

//----------------------------------------------------------------------
// Scheduler::~Scheduler
// 	De-allocate the list of ready threads.
//----------------------------------------------------------------------

Scheduler::~Scheduler()
{ 
    delete readyList; 

    delete L1List; 
    delete L2List; 
    delete L3List; 
} 

//----------------------------------------------------------------------
// Scheduler::ReadyToRun
// 	Mark a thread as ready, but not running.
//	Put it on the ready list, for later scheduling onto the CPU.
//
//	"thread" is the thread to be put on the ready list.
//----------------------------------------------------------------------

void
Scheduler::ReadyToRun (Thread *thread)
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    DEBUG(dbgThread, "Putting thread on ready list: " << thread->getName());
	//cout << "Putting thread on ready list: " << thread->getName() << endl ;
    thread->setStatus(READY);
    readyList->Append(thread);
}

//////////////////      12/12    ////////////////////////

//----------------------------------------------------------------------
// Scheduler::AddThreadPriority
// 	For aging mechanism, add priority by 10 to each thread

// ----------------------------------------------------------------------
void
Scheduler::AddThreadPriority(){

    // Thread* first = L1List->first;

	ListIterator<Thread*> *iter;
    iter = new ListIterator<Thread*>(L1List);

	for (; !iter->IsDone(); iter->Next()) {
	    // Operation on iter->Item()
        if(iter->Item()->priority +10 <= 149)
            iter->Item()->priority += 10;
        else iter->Item()->priority = 149;
        // first = first->next;

    }


	ListIterator<Thread*> *iter2;
    iter2 = new ListIterator<Thread*>(L2List);

	for (; !iter2->IsDone(); iter2->Next()) {
	    // Operation on iter->Item()
        if(iter2->Item()->priority +10 <= 149)
            iter2->Item()->priority += 10;
        else iter2->Item()->priority = 149;
        // first = first->next;

    }


	ListIterator<Thread*> *iter3;
    iter3 = new ListIterator<Thread*>(L3List);

	for (; !iter3->IsDone(); iter3->Next()) {
	    // Operation on iter->Item()
        if(iter3->Item()->priority +10 <= 149)
            iter3->Item()->priority += 10;
        else iter3->Item()->priority = 149;
        // first = first->next;
    }


    // ListElement<Thread*> *first= this->L1List->first;
    // while(first != L1List->last) {
    //     if(first->item->priority +10 <= 149)
    //         first->item->priority += 10;
    //     else first->item->priority = 149;
    //     first = first->next;
    // }
    // Last element in L1List.
    // if(first->item->priority +10 <= 149)
    //     first->item->priority += 10;
    // else first->item->priority = 149;

    /////// L2 List //////////
    // first = L2List->first;
    // while(first != L2List->last) {
    //     // if(first->item->priority +10 < = 149)
    //     first->item->priority += 10;
    //     // else first->item->priority = 149;
    //     first = first->next;
    // }
    // // Last element in L2List.
    // // if(first->item->priority +10 < = 149)
    // first->item->priority += 10;
    // // else first->item->priority = 149;

    // ///////// L3 List ///////////
    // first = L3List->first;
    //   while(first != L3List->last) {
    //     first->item->priority += 10;
    //     first = first->next;
    // }
    // // Last element in L3List.
    // first->item->priority += 10;  
 
 
    // thread->priority += 10;
}

//----------------------------------------------------------------------
// Scheduler::ReArrangeThreads
// 	For aging mechanism, since a thread(process) might migrate from lower
//  level queue to higher level queue due to the increase of its priority,
//  the scheduler should re-arrange the queue each thread belongs to.

// ----------------------------------------------------------------------
void
Scheduler::ReArrangeThreads(){
    // First just clear all contents in the queue.
    // Thread* dummy_thread;
    // while(!L1List->IsEmpty()) dummy_thread = L1List->RemoveFront();
    // while(!L2List->IsEmpty()) dummy_thread = L2List->RemoveFront();
    // while(!L3List->IsEmpty()) dummy_thread = L3List->RemoveFront();
    Thread* migrate_thread;

    // First, check L3List 
	ListIterator<Thread*> *iter3;
    iter3 = new ListIterator<Thread*>(L3List);

	for (; !iter3->IsDone(); iter3->Next()) {
	    // Operation on iter->Item()
        if(iter3->Item()->priority >=50){
            // should move to L1list
            migrate_thread = L3List->RemoveFront();
            L2List->Insert(migrate_thread);
        }
        // first = first->next;
    }

    // Then, check L2List 
	ListIterator<Thread*> *iter2;
    iter2 = new ListIterator<Thread*>(L2List);

	for (; !iter2->IsDone(); iter2->Next()) {
	    // Operation on iter->Item()
        if(iter2->Item()->priority >=100){
            // should move to L1list
            migrate_thread = L2List->RemoveFront();
            L1List->Insert(migrate_thread);
        }
    }


    // AddToQueue(thread, thread->priority);
}


///////////////////////// 12/12 Updated ///////////////////////////////

//----------------------------------------------------------------------
// Scheduler::Scheduling
// 	When a thread is added to multi-level feedback queue,
//	(Come back from WAITING state or from NEW state)
//  The scheduler should schedule which thread can take
//	over CPU and be executed.
//
//  Three scenario where Scheduling() would be called:
//  (1) Thread::Fork() (NEW -> READY) 
//  (2) Semaphore::V() (WAIT -> READY)
//  (3) Thread::Yield() (RUNNING -> READY) // This one I am not sure!
//
//

//  12/13 TODO :
//  Originally, we will call ReadyToRun to add threads(proc) into ready-queue (readyList)
//  Now, we change them to AddToQueue(thread)
// 
//  For scheduler->Run(nextThread, finishing)
//  We change to scheduler->Scheduling(finishing)
//  
//

//  For 'aging' mechanism, in every 'yieldOnReturn' from timer interrupt, 
//  we 'update' the priority of all threads?
//
//  For each thread, when they call Sleep(FALSE): update their burst time. 
//  When they are switching back to CPU : resume accumulating T. (ticks)
//

//----------------------------------------------------------------------

// TODO(1) : Add a scheduling function when a threads is add to ready-queue.(readyList)
// i.e. : When someone calls "kernel->scheduler->ReadyToRun(this)".
// Add to L1List/ L2List / L3List according to its priority (aging).
void 
Scheduler::Scheduling(bool finishing){

    if(!L1List->IsEmpty()){
        // Pick a thread from L1List.
        // SFJ
        this->Run(L1List->RemoveFront(), finishing);
    }
    else{
        if(!L2List->IsEmpty()){
            // Pick a thread from L2List.
            this->Run(L2List->RemoveFront(), finishing);
        }
        else{
            // Pick a thread from L3List.
            // Round-Robin
            // Run(,FALSE);
            this->Run(L3List->RemoveFront(), finishing);
        }

    }

}

//----------------------------------------------------------------------
// Scheduler::AddToQueue
// 	Add a thread into L1/L2/L3 queue according to its priority.
//----------------------------------------------------------------------


// TODO(2) : Add a function similar to "ReadyToRun" but with
//           arguments: 1. Thread* 2. Its Priority to decide 
//           which level queue it should be added. (L1, L2, L3) 

void 
Scheduler::AddToQueue(Thread* thread, int Priority){
    if(Priority>=100){
        // Add to L1 list.
        // L1List->Append(thread);
        L1List->Insert(thread);
    }
    else if(Priority >=50){
        // L2List->Append(thread);
        L2List->Insert(thread);
    }
    else{
        L3List->Append(thread);
    }
}






//----------------------------------------------------------------------
// Scheduler::FindNextToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

Thread *
Scheduler::FindNextToRun ()
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (readyList->IsEmpty()) {
		return NULL;
    } else {
    	return readyList->RemoveFront();
    }
}

//----------------------------------------------------------------------
// Scheduler::Run
// 	Dispatch the CPU to nextThread.  Save the state of the old thread,
//	and load the state of the new thread, by calling the machine
//	dependent context switch routine, SWITCH.
//
//      Note: we assume the state of the previously running thread has
//	already been changed from running to blocked or ready (depending).
// Side effect:
//	The global variable kernel->currentThread becomes nextThread.
//
//	"nextThread" is the thread to be put into the CPU.
//	"finishing" is set if the current thread is to be deleted
//		once we're no longer running on its stack
//		(when the next thread starts running)
//----------------------------------------------------------------------

void
Scheduler::Run (Thread *nextThread, bool finishing)
{
    Thread *oldThread = kernel->currentThread;
    
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if (finishing) {	// mark that we need to delete current thread
         ASSERT(toBeDestroyed == NULL);
	 toBeDestroyed = oldThread;
    }
    
    if (oldThread->space != NULL) {	// if this thread is a user program,
        oldThread->SaveUserState(); 	// save the user's CPU registers
	oldThread->space->SaveState();
    }
    
    oldThread->CheckOverflow();		    // check if the old thread
					    // had an undetected stack overflow

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running
    
    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".

    ///////// 12/10 /////////
    // Update approx. burst time for this thread.

    // Get current ticks. (Update this thread's approx. cpu burst ticks)
    kernel->currentThread->update_burst_time(kernel->stats->totalTicks);

    SWITCH(oldThread, nextThread);

    // we're back, running oldThread

    ///////// 12/10 /////////
    // Resume accumulating CPU burst ticks.
    kernel->currentThread->record_start_ticks(kernel->stats->totalTicks);
      
    // interrupts are off when we return from switch!
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    DEBUG(dbgThread, "Now in thread: " << oldThread->getName());

    CheckToBeDestroyed();		// check if thread we were running
					// before this one has finished
					// and needs to be cleaned up
    
    if (oldThread->space != NULL) {	    // if there is an address space
        oldThread->RestoreUserState();     // to restore, do it.
	oldThread->space->RestoreState();
    }
}

//----------------------------------------------------------------------
// Scheduler::CheckToBeDestroyed
// 	If the old thread gave up the processor because it was finishing,
// 	we need to delete its carcass.  Note we cannot delete the thread
// 	before now (for example, in Thread::Finish()), because up to this
// 	point, we were still running on the old thread's stack!
//----------------------------------------------------------------------

void
Scheduler::CheckToBeDestroyed()
{
    if (toBeDestroyed != NULL) {
        delete toBeDestroyed;
	toBeDestroyed = NULL;
    }
}
 
//----------------------------------------------------------------------
// Scheduler::Print
// 	Print the scheduler state -- in other words, the contents of
//	the ready list.  For debugging.
//----------------------------------------------------------------------
void
Scheduler::Print()
{
    cout << "Ready list contents:\n";
    readyList->Apply(ThreadPrint);
}

