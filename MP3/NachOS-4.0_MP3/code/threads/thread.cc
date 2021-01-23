// thread.cc 
//	Routines to manage threads.  These are the main operations:
//
//	Fork -- create a thread to run a procedure concurrently
//		with the caller (this is done in two steps -- first
//		allocate the Thread object, then call Fork on it)
//	Begin -- called when the forked procedure starts up, to turn
//		interrupts on and clean up after last thread
//	Finish -- called when the forked procedure finishes, to clean up
//	Yield -- relinquish control over the CPU to another ready thread
//	Sleep -- relinquish control over the CPU, but thread is now blocked.
//		In other words, it will not run again, until explicitly 
//		put back on the ready queue.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "thread.h"
#include "switch.h"
#include "synch.h"
#include "sysdep.h"

// this is put at the top of the execution stack, for detecting stack overflows
const int STACK_FENCEPOST = 0xdedbeef;

// #define DEBUG_QUEUE
// #define DEBUG_THREAD_FINISH

//----------------------------------------------------------------------
// Thread::Thread
// 	Initialize a thread control block, so that we can then call
//	Thread::Fork.
//
//	"threadName" is an arbitrary string, useful for debugging.
//----------------------------------------------------------------------

Thread::Thread(char* threadName, int threadID)
{
	ID = threadID;
    name = threadName;
    stackTop = NULL;
    stack = NULL;
    status = JUST_CREATED;
    for (int i = 0; i < MachineStateSize; i++) {
	machineState[i] = NULL;		// not strictly necessary, since
					// new thread ignores contents 
					// of machine registers
    }
    space = NULL;   // user space. NOT kernel space
    priority = 0;  // default
    approx_bursttime = 0; //t0 = 0.
    cpu_start_ticks = cpu_end_ticks = 0;

    accu_wait_ticks = 0; //t0 = 0.
    start_wait_ticks = 0;

}

////////// 12/10 Added /////////////////
Thread::Thread(char* threadName, int threadID, int _priority)
{
	ID = threadID;
    name = threadName;
    stackTop = NULL;
    stack = NULL;
    status = JUST_CREATED;
    for (int i = 0; i < MachineStateSize; i++) {
	machineState[i] = NULL;		// not strictly necessary, since
					// new thread ignores contents 
					// of machine registers
    }
    space = NULL;   // user space. NOT kernel space
    priority = _priority;
    approx_bursttime = 0; //t0 = 0.
    cpu_start_ticks = cpu_end_ticks = 0;
    accu_wait_ticks = 0; //t0 = 0.
    start_wait_ticks = 0;    
}


//----------------------------------------------------------------------
// Thread::~Thread
// 	De-allocate a thread.
//
// 	NOTE: the current thread *cannot* delete itself directly,
//	since it is still running on the stack that we need to delete.
//
//      NOTE: if this is the main thread, we can't delete the stack
//      because we didn't allocate it -- we got it automatically
//      as part of starting up Nachos.
//----------------------------------------------------------------------

Thread::~Thread()
{
    DEBUG(dbgThread, "Deleting thread: " << name);
    ASSERT(this != kernel->currentThread);
    if (stack != NULL)
	DeallocBoundedArray((char *) stack, StackSize * sizeof(int));
}

//----------------------------------------------------------------------
// Thread::Fork
// 	Invoke (*func)(arg), allowing caller and callee to execute 
//	concurrently.
//
//	NOTE: although our definition allows only a single argument
//	to be passed to the procedure, it is possible to pass multiple
//	arguments by making them fields of a structure, and passing a pointer
//	to the structure as "arg".
//
// 	Implemented as the following steps:
//		1. Allocate a stack
//		2. Initialize the stack so that a call to SWITCH will
//		cause it to run the procedure
//		3. Put the thread on the ready queue
// 	
//	"func" is the procedure to run concurrently.
//	"arg" is a single argument to be passed to the procedure.
//----------------------------------------------------------------------

void 
Thread::Fork(VoidFunctionPtr func, void *arg)
{
    Interrupt *interrupt = kernel->interrupt;
    Scheduler *scheduler = kernel->scheduler;
    IntStatus oldLevel;
    
    DEBUG(dbgThread, "Forking thread: " << name << " f(a): " << (int) func << " " << arg);
    StackAllocate(func, arg);

    oldLevel = interrupt->SetLevel(IntOff);

    /// 12/15 //
    // scheduler->ReadyToRun(this);	// ReadyToRun assumes that interrupts 
    DEBUG(dbgTraCode, "Fork(): AddToQueue: "<<this->name);
    // cout<<"Fork(): AddToQueue: "<<this->name<<"\n";
    scheduler->AddToQueue(this, this->priority);
    // DEBUG(dbgZ, "Add thread: " << this->name << " with priority: " << this->priority);
    //////// 12/10 Added /////////
    // scheduler->AddToQueue(this, this->priority);

					// are disabled!
    (void) interrupt->SetLevel(oldLevel); // 把舊的interrupt status (IntON / IntOff)設回來
}    


//////////// 12/10 Added /////////////////
//----------------------------------------------------------------------
// Thread::record_start_ticks
// 	Record starting tick when a thread gets the CPU.
//
// Thread::record_start_ticks
// 	Record starting tick when a thread gets the CPU.
//

//----------------------------------------------------------------------

int 
Thread::InWhichQueue(){
    if(this->priority>=100) return 1;
    else if(this->priority>=50) return 2;
    else return 3;
}

void 
Thread::record_start_ticks(int cpu_start_ticks){
    this->cpu_start_ticks = cpu_start_ticks;
}

void
Thread::update_burst_time(int cpu_end_ticks){
    this->cpu_end_ticks = cpu_end_ticks;
    this->True_ticks += this->cpu_end_ticks - this->cpu_start_ticks;
    // int True_burst_ticks = this->cpu_end_ticks - this->cpu_start_ticks;
    // Update approximate cpu burst time ti.
    this->approx_bursttime = 0.5*this->True_ticks + 0.5*this->last_approx_bursttime;

    DEBUG(dbgZ, "[D] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<this->ID<<"}​ ] update approximate burst time, from: [​ {"<<this->last_approx_bursttime  <<"}], add [​ {"<<this->True_ticks<<"}​ ], to [​ {"<<this->approx_bursttime<<"}]");
    
    // update last approx. bursttime. (ti-1)
    this->last_approx_bursttime = this->approx_bursttime;
    this->True_ticks = 0; //reset True_ticks.
}




//----------------------------------------------------------------------
// Thread::CheckOverflow
// 	Check a thread's stack to see if it has overrun the space
//	that has been allocated for it.  If we had a smarter compiler,
//	we wouldn't need to worry about this, but we don't.
//
// 	NOTE: Nachos will not catch all stack overflow conditions.
//	In other words, your program may still crash because of an overflow.
//
// 	If you get bizarre results (such as seg faults where there is no code)
// 	then you *may* need to increase the stack size.  You can avoid stack
// 	overflows by not putting large data structures on the stack.
// 	Don't do this: void foo() { int bigArray[10000]; ... }
//----------------------------------------------------------------------

void
Thread::CheckOverflow()
{
    if (stack != NULL) {
#ifdef HPUX			// Stacks grow upward on the Snakes
	ASSERT(stack[StackSize - 1] == STACK_FENCEPOST);
#else
	ASSERT(*stack == STACK_FENCEPOST);
#endif
   }
}

//----------------------------------------------------------------------
// Thread::Begin
// 	Called by ThreadRoot when a thread is about to begin
//	executing the forked procedure.
//
// 	It's main responsibilities are:
//	1. deallocate the previously running thread if it finished 
//		(see Thread::Finish())
//	2. enable interrupts (so we can get time-sliced)
//----------------------------------------------------------------------

void
Thread::Begin ()
{
    ASSERT(this == kernel->currentThread);
    DEBUG(dbgThread, "Beginning thread: " << name);
    
    kernel->scheduler->CheckToBeDestroyed();
    kernel->interrupt->Enable();
}

//----------------------------------------------------------------------
// Thread::Finish
// 	Called by ThreadRoot when a thread is done executing the 
//	forked procedure.
//
// 	NOTE: we can't immediately de-allocate the thread data structure 
//	or the execution stack, because we're still running in the thread 
//	and we're still on the stack!  Instead, we tell the scheduler
//	to call the destructor, once it is running in the context of a different thread.
//  (CheckToBeDestroyed())
//
// 	NOTE: we disable interrupts, because Sleep() assumes interrupts
//	are disabled.
//----------------------------------------------------------------------

//
void
Thread::Finish ()
{
    (void) kernel->interrupt->SetLevel(IntOff);		
    ASSERT(this == kernel->currentThread);
    
    DEBUG(dbgThread, "Finishing thread: " << name);
    Sleep(TRUE);				// invokes SWITCH
    // not reached
}


//----------------------------------------------------------------------
// Thread::Yield
// 	Relinquish the CPU if any other thread is ready to run.
//	If so, put the thread on the end of the ready list, so that
//	it will eventually be re-scheduled.
//
//	NOTE: returns immediately if no other thread on the ready queue.
//	Otherwise returns when the thread eventually works its way
//	to the front of the ready list and gets re-scheduled.
//
//	NOTE: we disable interrupts, so that looking at the thread
//	on the front of the ready list, and switching to it, can be done
//	atomically.  On return, we re-set the interrupt level to its
//	original state, in case we are called with interrupts disabled. 
//
// 	Similar to Thread::Sleep(), but a little different.
//----------------------------------------------------------------------

void
Thread::Yield ()
{
    Thread *nextThread;
    IntStatus oldLevel = kernel->interrupt->SetLevel(IntOff);
    
    ASSERT(this == kernel->currentThread);
    
    DEBUG(dbgThread, "Yielding thread: " << name);

   
    //// DO Aging here.
    kernel->scheduler->Aging();
    kernel->scheduler->ReArrangeThreads();
    // Only call Scheduling if cpu is running in L3 queue(round-robin)
    if(kernel->currentThread->InWhichQueue()==3){
        nextThread = kernel->scheduler->Scheduling();
        if (nextThread != NULL) {
        kernel->scheduler->AddToQueue(this, this->priority);
        kernel->scheduler->Run(nextThread, FALSE);
        }
    }
    (void) kernel->interrupt->SetLevel(oldLevel);
}

    // else{
    //     //cout<<"NOT running in L3 queue, don't need to yield thread.";
    // }

    // If currentThread is L1 or L2, cannot preempt!


    /* ------------- Original -------------------------
    nextThread = kernel->scheduler->FindNextToRun();
    if (nextThread != NULL) {
	// kernel->scheduler->ReadyToRun(this);
	// kernel->scheduler->Run(nextThread, FALSE);
    }
    ------------------------------------------------- */




//----------------------------------------------------------------------
// Thread::Sleep
// 	Relinquish the CPU, because the current thread has either
//	finished or is blocked waiting on a synchronization 
//	variable (Semaphore, Lock, or Condition).  In the latter case,
//	eventually some thread will wake this thread up, and put it
//	back on the ready queue, so that it can be re-scheduled.
//
//	NOTE: if there are no threads on the ready queue, that means
//	we have no thread to run.  "Interrupt::Idle" is called
//	to signify that we should idle the CPU until the next I/O interrupt
//	occurs (the only thing that could cause a thread to become
//	ready to run).
//
//	NOTE: we assume interrupts are already disabled, because it
//	is called from the synchronization routines which must
//	disable interrupts for atomicity.   We need interrupts off 
//	so that there can't be a time slice between pulling the first thread
//	off the ready list, and switching to it.
//----------------------------------------------------------------------
void
Thread::Sleep (bool finishing)
{
    Thread *nextThread;
    
    ASSERT(this == kernel->currentThread);
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    
    // #ifdef DEBUG_QUEUES
    DEBUG(dbgThread, "Sleeping thread: " << name);
    DEBUG(dbgTraCode, "In Thread::Sleep, Sleeping thread: " << name << ", " << kernel->stats->totalTicks);
    // #endif

    status = BLOCKED;

    ///////// 12/15 /////////
    // Should UPDATE BURST TIME when call thread->Sleep(FALSE)!!!
    
    // Reset accu_waitticks.
    // this->accu_wait_ticks = 0; //reset.

    if(finishing==FALSE){
    #ifdef DEBUG_THREAD_FINISH
        cout<<"\nTicks: "<< kernel->stats->totalTicks<<", thread " << name << " wait for I/O so Sleep.\n";
    #endif
        this->update_burst_time(kernel->stats->totalTicks);
        
        // this->accu_wait_ticks = 0; //reset.
    }
     #ifdef DEBUG_THREAD_FINISH
    else cout<<"Ticks: "<< kernel->stats->totalTicks<<", thread " << name << " finish!\n";
    #endif
    // cout<<"Ticks: "<< kernel->stats->totalTicks<<", debug Thread::Sleep " << name << " wait for Idle";
	// DEBUG(dbgZ, "Ticks: "<< kernel->stats->totalTicks<<", debug Thread::Sleep " << name << " wait for Idle");

    // while(kernel->scheduler->StillHasThreadToRun()==false){
    //     kernel->interrupt->Idle();
    // }
    // kernel->scheduler->Scheduling(finishing);


    while ((nextThread = kernel->scheduler->Scheduling()) == NULL) {
		kernel->interrupt->Idle();	// no one to run, wait for an interrupt
        // DEBUG(dbgThread, "NO Thread to be run. Kernel->interrupt->Idle !");
	}     
    kernel->scheduler->Run(nextThread, finishing); // CALL Context Switch SWITCH()


    /* ----------------------------- Original -------------------------------------
    while ((nextThread = kernel->scheduler->FindNextToRun()) == NULL) {
		kernel->interrupt->Idle();	// no one to run, wait for an interrupt
        DEBUG(dbgThread, "NO Thread to be run. Kernel->interrupt->Idle !");
	}     
    kernel->scheduler->Run(nextThread, finishing); // CALL Context Switch SWITCH()
    ----------------------------------------------------------------------------- */   
}

    ////////// 12/15 TODO: First time when main thread is done, SLEEP(TRUE)
    /* 
        We should find another thread to Run, Do NOT use FindNextToRun
        Since it uses readyList instead of L1, L2, L3 queue we want!
    */
    //////// KEY: Should BLOCKED here if NO any thread in ALL ready-queue
    // Blocked here until some thread get resource and call Semaphore::V() !
    // Wake a thread up and put it back to ready-queue (AddToQueue in threads/synch.cc)
    // And this thread is dispacted to CPU and get computing resources!

    // cout<<"thread "<<this->getID()<<" sleep, scheduling!\n";
    // Should be blocked above.
    // returns when it's time for us to run (only for waiting for I/O (Semaphore::P() ))
    // cout<<"\ncur thread: "<<kernel->currentThread->getName()<<"\n";
    // For someone go to sleep, yield CPU here! 
    // Next time when it return from here, what?

    ////////// 12/16 Problem here: when we preempt t2 by t1(since aging, priority->100 > t2's 90)
    //         Immediately switch from t1 to t2 back (t1 executes 0 ticks)
    //         Still finding the reason! Debug QQ 
 
    // nextThread = kernel->scheduler->FindNextToRun2();
    // if(nextThread==NULL){
    //     // wait?
    // }
    // else{

    //     kernel->scheduler->Run(nextThread,finishing);
    //     // kernel->scheduler->Scheduling(finishing);
    // }



    // if( kernel->currentThread->InWhichQueue()==3 || (kernel->currentThread->InWhichQueue()==2 && nextThread->InWhichQueue()==1) ||  ( kernel->currentThread->InWhichQueue()==3 && (nextThread->InWhichQueue()==2||nextThread->InWhichQueue()==1)) ){
    //     kernel->scheduler->Scheduling(finishing);
    // }
     // else do not scheduling!



//----------------------------------------------------------------------
// ThreadBegin, ThreadFinish,  ThreadPrint
//	Dummy functions because C++ does not (easily) allow pointers to member
//	functions.  So we create a dummy C function
//	(which we can pass a pointer to), that then simply calls the 
//	member function.
//----------------------------------------------------------------------

static void ThreadFinish()    { kernel->currentThread->Finish(); }
static void ThreadBegin() { kernel->currentThread->Begin(); }
void ThreadPrint(Thread *t) { t->Print(); }

#ifdef PARISC

//----------------------------------------------------------------------
// PLabelToAddr
//	On HPUX, function pointers don't always directly point to code,
//	so we need to do the conversion.
//----------------------------------------------------------------------

static void *
PLabelToAddr(void *plabel)
{
    int funcPtr = (int) plabel;

    if (funcPtr & 0x02) {
        // L-Field is set.  This is a PLT pointer
        funcPtr -= 2;	// Get rid of the L bit
        return (*(void **)funcPtr);
    } else {
        // L-field not set.
        return plabel;
    }
}
#endif

//----------------------------------------------------------------------
// Thread::StackAllocate
//	Allocate and initialize an execution stack.  The stack is
//	initialized with an initial stack frame for ThreadRoot, which:
//		enables interrupts
//		calls (*func)(arg)
//		calls Thread::Finish
//
//	"func" is the procedure to be forked (ForkExecute in kernel.cc)
//	"arg" is the parameter to be passed to the procedure (*thread (pointer) ) 
//----------------------------------------------------------------------

void
Thread::StackAllocate (VoidFunctionPtr func, void *arg)
{
    stack = (int *) AllocBoundedArray(StackSize * sizeof(int));

#ifdef PARISC
    // HP stack works from low addresses to high addresses
    // everyone else works the other way: from high addresses to low addresses
    stackTop = stack + 16;	// HP requires 64-byte frame marker
    stack[StackSize - 1] = STACK_FENCEPOST;
#endif

#ifdef SPARC
    stackTop = stack + StackSize - 96; 	// SPARC stack must contains at 
					// least 1 activation record 
					// to start with.
    *stack = STACK_FENCEPOST;
#endif 

#ifdef PowerPC // RS6000
    stackTop = stack + StackSize - 16; 	// RS6000 requires 64-byte frame marker
    *stack = STACK_FENCEPOST;
#endif 

#ifdef DECMIPS
    stackTop = stack + StackSize - 4;	// -4 to be on the safe side!
    *stack = STACK_FENCEPOST;
#endif

#ifdef ALPHA
    stackTop = stack + StackSize - 8;	// -8 to be on the safe side!
    *stack = STACK_FENCEPOST;
#endif


#ifdef x86
    // the x86 passes the return address on the stack.  In order for SWITCH() 
    // to go to ThreadRoot when we switch to this thread, the return addres 
    // used in SWITCH() must be the starting address of ThreadRoot.
    stackTop = stack + StackSize - 4;	// -4 to be on the safe side!
    *(--stackTop) = (int) ThreadRoot;
    *stack = STACK_FENCEPOST;
#endif
    
#ifdef PARISC
    machineState[PCState] = PLabelToAddr(ThreadRoot);
    machineState[StartupPCState] = PLabelToAddr(ThreadBegin);
    machineState[InitialPCState] = PLabelToAddr(func);
    machineState[InitialArgState] = arg;
    machineState[WhenDonePCState] = PLabelToAddr(ThreadFinish);
#else
    machineState[PCState] = (void*)ThreadRoot;
    machineState[StartupPCState] = (void*)ThreadBegin;
    machineState[InitialPCState] = (void*)func;
    machineState[InitialArgState] = (void*)arg;
    machineState[WhenDonePCState] = (void*)ThreadFinish;
#endif
}

#include "machine.h"

//----------------------------------------------------------------------
// Thread::SaveUserState
//	Save the CPU state of a user program on a context switch.
//
//	Note that a user program thread has *two* sets of CPU registers -- 
//	one for its state while executing user code, one for its state 
//	while executing kernel code.  This routine saves the former.
//----------------------------------------------------------------------

void
Thread::SaveUserState()
{
    for (int i = 0; i < NumTotalRegs; i++)
	userRegisters[i] = kernel->machine->ReadRegister(i);
}

//----------------------------------------------------------------------
// Thread::RestoreUserState
//	Restore the CPU state of a user program on a context switch.
//
//	Note that a user program thread has *two* sets of CPU registers -- 
//	one for its state while executing user code, one for its state 
//	while executing kernel code.  This routine restores the former.
//----------------------------------------------------------------------

void
Thread::RestoreUserState()
{
    for (int i = 0; i < NumTotalRegs; i++)
	kernel->machine->WriteRegister(i, userRegisters[i]);
}


//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

static void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	cout << "*** thread " << which << " looped " << num << " times\n";
        kernel->currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// Thread::SelfTest
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
Thread::SelfTest()
{
    DEBUG(dbgThread, "Entering Thread::SelfTest");

    Thread *t = new Thread("forked thread", 1);

    t->Fork((VoidFunctionPtr) SimpleThread, (void *) 1);
    kernel->currentThread->Yield();
    SimpleThread(0);
}
