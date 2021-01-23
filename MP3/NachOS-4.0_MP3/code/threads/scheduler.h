// scheduler.h 
//	Data structures for the thread dispatcher and scheduler.
//	Primarily, the list of threads that are ready to run.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "copyright.h"
#include "list.h"
#include "thread.h"

// The following class defines the scheduler/dispatcher abstraction -- 
// the data structures and operations needed to keep track of which 
// thread is running, and which threads are ready but not running.

class Scheduler {
  public:
    Scheduler();		// Initialize list of ready threads 
    ~Scheduler();		// De-allocate ready list

    void ReadyToRun(Thread* thread);	
    				// Thread can be dispatched.
    Thread* FindNextToRun();	// Dequeue first thread on the ready 
				// list, if any, and return thread.
    void Run(Thread* nextThread, bool finishing);
    				// Cause nextThread to start running
    void CheckToBeDestroyed();// Check if thread that had been
    				// running needs to be deleted
    void Print();		// Print contents of ready list
    
    // SelfTest for scheduler is implemented in class Thread

    /* ---------------- MP3 ----------------- */
    void Aging();
    void List_All_thread();
    Thread* FindNextToRun2();
    bool StillHasThreadToRun();
    Thread* Scheduling();
    void Check_Preempt(Thread* thread);
    void AddToQueue(Thread* thread, int Priority);
    void AddToQueue2(Thread* thread, int Priority);
    void AddThreadPriority();
    void ReArrangeThreads();
    /* --------------------------------------- */

    // TODO(1) : Add a scheduling function when a threads is add to ready-queue.(readyList)
    // i.e. : When someone calls "kernel->scheduler->ReadyToRun(this)".
    // Add to L1List/ L2List / L3List according to its priority (aging).
    // void Scheduling(bool finishing);

    // TODO(2) : Add a function similar to "ReadyToRun" but with
    //           arguments: 1. Thread* 2. Its Priority to decide 
    //           which level queue it should be added. (L1, L2, L3) 

  private:
    List<Thread *> *readyList;  // queue of threads that are ready to run,
				// but not running
    
    /////////// 12/10 Added //////////////////
    // List<Thread *> *L1List; // Preemptive SFJ 
    // List<Thread *> *L2List; // Non-preemptive priority
    
    /* ----------------- MP3 ------------------ */
    /// 3-Level queue: sorted with threir priority!
    SortedList<Thread *> *L1List;	  // Preemptive SFJ 
    SortedList<Thread *> *L2List;	  // Non-preemptive priority
    List<Thread *> *L3List;         // Round-Robin TQ:100 ticks 
    /* --------------------------------------- */

    Thread *toBeDestroyed;	// finishing thread to be destroyed
    				// by the next thread that runs
};

#endif // SCHEDULER_H
