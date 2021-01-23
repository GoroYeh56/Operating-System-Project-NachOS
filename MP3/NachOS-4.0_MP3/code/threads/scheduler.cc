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

// #define LIST_QUEUE_CONTENTS
// #define DEBUG_QUEUES
// #define DEBUG_PREEMPT
// #define DEBUG_AGING

static int
L1SchedulingCompare (Thread *x, Thread *y)
{
    if (x->approx_bursttime < y->approx_bursttime) { return -1; }
    else if (x->approx_bursttime > y->approx_bursttime) { return 1; }
    else { return 0; }
}


static int
L2SchedulingCompare (Thread *x, Thread *y)
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
    
    L1List = new SortedList<Thread*>(L1SchedulingCompare);
    L2List = new SortedList<Thread*>(L2SchedulingCompare);
    // L1List = new List<Thread *>;
    // L2List = new List<Thread *>;
    L3List = new List<Thread *>;  // Just insert, no need to sort!
    

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
// Scheduler::AddThreadPriority => NO USE!
// 	For aging mechanism, add priority by 10 to each thread

// Error here. access Item() on NULL list.

// ----------------------------------------------------------------------
void
Scheduler::AddThreadPriority(){

    // Thread* first = L1List->first;

	ListIterator<Thread*> *iter;
    iter = new ListIterator<Thread*>(L1List);

    if(!L1List->IsEmpty()){
        for (; !iter->IsDone(); iter->Next()) {
            // Operation on iter->Item()

            // TODO : should prevent >149 case. in DEBUG
            if(iter->Item()->priority+10>=149){
            DEBUG(dbgZ,  "[C] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<iter->Item()->ID<<"}​ ] changes its priority from [​ {"<<iter->Item()->priority<<"}​ ] to [​ {149}​ ]");
            }else{
            DEBUG(dbgZ,  "[C] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<iter->Item()->ID<<"}​ ] changes its priority from [​ {"<<iter->Item()->priority<<"}​ ] to [​ {"<<iter->Item()->priority+10 <<"}​ ]");
            }

            if(iter->Item()->priority +10 <= 149)
                iter->Item()->priority += 10;
            else iter->Item()->priority = 149;
            // first = first->next;

        }
    }

	ListIterator<Thread*> *iter2;
    iter2 = new ListIterator<Thread*>(L2List);
    if(!L2List->IsEmpty()){
        for (; !iter2->IsDone(); iter2->Next()) {
            // Operation on iter->Item()
            DEBUG(dbgZ,  "[C] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<iter2->Item()->ID<<"}​ ] changes its priority from [​ {"<<iter2->Item()->priority<<"}​ ] to [​ {"<<iter2->Item()->priority+10 <<"}​ ]");

            if(iter2->Item()->priority +10 <= 149)
                iter2->Item()->priority += 10;
            else iter2->Item()->priority = 149;
            // first = first->next;

        }
    }

	ListIterator<Thread*> *iter3;
    iter3 = new ListIterator<Thread*>(L3List);
    if(!L3List->IsEmpty()){
        for (; !iter3->IsDone(); iter3->Next()) {
        
            // Operation on iter->Item()
            DEBUG(dbgZ,  "[C] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<iter3->Item()->ID<<"}​ ] changes its priority from [​ {"<<iter3->Item()->priority<<"}​ ] to [​ {"<<iter3->Item()->priority+10 <<"}​ ]");
            if(iter3->Item()->priority +10 <= 149)
                iter3->Item()->priority += 10;
            else iter3->Item()->priority = 149;
            // first = first->next;
        }
    }
}

//----------------------------------------------------------------------
// Scheduler::ReArrangeThreads
// 	For aging mechanism, since a thread(process) might migrate from lower
//  level queue to higher level queue due to the increase of its priority,
//  the scheduler should re-arrange the queue each thread belongs to.

//  Check_Preempt here!

// ----------------------------------------------------------------------
void
Scheduler::ReArrangeThreads(){

    Thread* migrate_thread;
    // First, check L3List 
	ListIterator<Thread*> *iter3;
    iter3 = new ListIterator<Thread*>(L3List);

    // Note: We cannot use this method on L3, since L3 is NOT a SortedList!
    // We should just Re-Insert every thread in L3!

	for (; !iter3->IsDone(); iter3->Next()) {
	    // Operation on iter->Item()
        migrate_thread = L3List->RemoveFront();

        // This case is NOT possible!
        if(migrate_thread->priority>=100){
            // Insert into L1
            DEBUG(dbgZ, "[B] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<migrate_thread->ID<<"}​ ] is removed from queue L[​ {3}​ ]");
            L1List->Insert(migrate_thread);
            
            DEBUG(dbgZ, "[A] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<migrate_thread->ID<<"}​ ] is inserted into queue L[​ {1}​ ]");
            this->Check_Preempt(migrate_thread);
        }
        else if(migrate_thread->priority >= 50){
            // Insert into L2
            DEBUG(dbgZ, "[B] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<migrate_thread->ID<<"}​ ] is removed from queue L[​ {3}​ ]");
            L2List->Insert(migrate_thread);
            
            DEBUG(dbgZ, "[A] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<migrate_thread->ID<<"}​ ] is inserted into queue L[​ {2}​ ]");
            this->Check_Preempt(migrate_thread);

        }else{
            // Insert Back.
            DEBUG(dbgZ, "[B] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<migrate_thread->ID<<"}​ ] is removed from queue L[​ {3}​ ]");
            L3List->Append(migrate_thread);
            DEBUG(dbgZ, "[B] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<migrate_thread->ID<<"}​ ] is inserted into queue L[​ {3}​ ]");
            // Don't need to check preempt!
        }
    }

    // Then, check L2List 
	ListIterator<Thread*> *iter2;
    iter2 = new ListIterator<Thread*>(L2List);

	for (; !iter2->IsDone(); iter2->Next()) {
	    // Operation on iter->Item()
        if(iter2->Item()->priority >=100){
            // should move to L1list
            migrate_thread = L2List->RemoveFront();
            DEBUG(dbgZ, "[B] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<migrate_thread->ID<<"}​ ] is removed from queue L[​ {2}​ ]");
            L1List->Insert(migrate_thread);
            DEBUG(dbgZ, "[A] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<migrate_thread->ID<<"}​ ] is inserted into queue L[​ {1}​ ]");
            this->Check_Preempt(migrate_thread);
        }
    }
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

// TODO : should change the last " accumulated total ticks"

// void 
// Scheduler::Scheduling(bool finishing){

// Return the thread to be executed.
Thread*
Scheduler::Scheduling(){

    Thread* nextThread;
    
    #ifdef DEBUG_QUEUES
    cout<<"\nSb. calls scheduling, current Thread in Running state (holds CPU): "<<kernel->currentThread->getName()<<"\n";
    this->List_All_thread();
    // cout<< kernel->synchConsoleOut->waitFor->queue->NumInList();
    // cout<<"WAIT queue: "<<kernel->synchConsoleOut->waitFor->queue->NumInList() <<" threads\n";
    
    #endif


    if(!L1List->IsEmpty()){
        // Pick a thread from L1List.
        // SFJ
        // if(L1List->Front()->priority < kernel->currentThread->priority) return;
        nextThread = L1List->RemoveFront();
        // cout<<"now t"<<kernel->currentThread->getID()<<", to t"<<nextThread->getID()<<"\n";
        nextThread->record_start_ticks(kernel->stats->totalTicks);
        DEBUG(dbgZ, "[B] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<nextThread->ID<<"}​ ] is removed from queue L[​ {1}​ ]");
        DEBUG(dbgZ,  "[E] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<nextThread->ID<<"}​ ] is now selected for execution, thread [​ {"<<kernel->currentThread->ID<<"}​ ] is replaced, and it has executed [​ {"<<kernel->stats->totalTicks - kernel->currentThread->cpu_start_ticks<<"}​ ] ticks ");
        
        #ifdef DEBUG_QUEUES
        cout<<"Pick: "<<nextThread->getName()<<"\n";
        #endif 
        return nextThread;
        // this->Run(nextThread, finishing);
        // nextThread->record_start_ticks(kernel->stats->totalTicks);
    }
    else{
        if(!L2List->IsEmpty()){
            // if(kernel->currentThread->InWhichQueue()==1) return;// L2 Cannot preempt it.
            // Pick a thread from L2List.
            nextThread = L2List->RemoveFront();
            nextThread->record_start_ticks(kernel->stats->totalTicks);
            // cout<<"now t"<<kernel->currentThread->getID()<<", to t"<<nextThread->getID()<<"\n";
            DEBUG(dbgZ, "[B] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<nextThread->ID<<"}​ ] is removed from queue L[​ {2}​ ]");
            DEBUG(dbgZ,  "[E] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<nextThread->ID<<"}​ ] is now selected for execution, thread [​ {"<<kernel->currentThread->ID<<"}​ ] is replaced, and it has executed [​ {"<<kernel->stats->totalTicks - kernel->currentThread->cpu_start_ticks<<"}​ ] ticks ");
            
            #ifdef DEBUG_QUEUES
            cout<<"Pick: "<<nextThread->getName()<<"\n";
            #endif             
            // this->Run(nextThread, finishing);
            return nextThread;
            // nextThread->record_start_ticks(kernel->stats->totalTicks);
        }
        else{
            // Pick a thread from L3List.
            // Round-Robin
            // Run(,FALSE);
            // TODO : should change the last " accumulated total ticks"
            // if(kernel->currentThread->InWhichQueue()==1 || kernel->currentThread->InWhichQueue()==2) return;
            if( !L3List->IsEmpty()){
                nextThread = L3List->RemoveFront();
            // cout<<"now t"<<kernel->currentThread->getID()<<", to t"<<nextThread->getID()<<"\n";
                nextThread->record_start_ticks(kernel->stats->totalTicks);
                DEBUG(dbgZ, "[B] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<nextThread->ID<<"}​ ] is removed from queue L[​ {3}​ ]");
                DEBUG(dbgZ,  "[E] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<nextThread->ID<<"}​ ] is now selected for execution, thread [​ {"<<kernel->currentThread->ID<<"}​ ] is replaced, and it has executed [​ {"<<kernel->stats->totalTicks - kernel->currentThread->cpu_start_ticks<<"}​ ] ticks ");
                
                #ifdef DEBUG_QUEUES
                cout<<"Pick: "<<nextThread->getName()<<"\n";
                #endif         
                return nextThread;
            // this->Run(nextThread, finishing);
            }
            else{
                 // cout<<"Error! NULL threads in ready-queue\n";
                return NULL;
            }
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
Scheduler::Check_Preempt(Thread* thread){
        // cout<<kernel->currentThread->getName()<<"\n";
        // Check for Preemption
        //========== From RUNNING to READY state ===================//
        #ifdef DEBUG_PREEMPT
        cout<<"Check Preempt: Cur t: "<<kernel->currentThread->getName()<<" Add t: "<<thread->getName()<<"\n";
        #endif
        // Case 1: Running L3 but L1 or L2 has threads.
        if( kernel->currentThread->InWhichQueue()==3 && (thread->InWhichQueue()==2||thread->InWhichQueue()==1)){
            #ifdef DEBUG_PREEMPT
            cout<<"\nShould Preempt!\n";
            cout<<"cur  priority: "<< kernel->currentThread->priority <<"in queue: "<<kernel->currentThread->InWhichQueue()<<"\n";
            cout<<"Add  priority: "<< thread->priority<<" belongs to queue "  <<thread->InWhichQueue()<<"\n"; 
            #endif        
            kernel->currentThread->True_ticks += kernel->stats->totalTicks - kernel->currentThread->cpu_start_ticks;

            #ifdef DEBUG_PREEMPT
            cout<<"Current ready-queue contents:\n";
            this->List_All_thread();
            #endif
            Thread* nextThread = this->Scheduling();    // Should retun L1 thread.
            
            #ifdef DEBUG_PREEMPT
            cout<<"Thus, pick "<<nextThread->getName()<<"\n";
            #endif 

            this->AddToQueue(kernel->currentThread, kernel->currentThread->priority);  // from RUNNING to READY state.
            this->Run(nextThread, FALSE);
        }   
        // Case 2: Running L2 but L1 has threads.
        else if (kernel->currentThread->InWhichQueue()==2 && thread->InWhichQueue()==1) {
            #ifdef DEBUG_PREEMPT
            cout<<"\nShould Preempt!\n";
            cout<<"cur  priority: "<< kernel->currentThread->priority <<"in queue: "<<kernel->currentThread->InWhichQueue()<<"\n";
            cout<<"Add  priority: "<< thread->priority<<" belongs to queue "  <<thread->InWhichQueue()<<"\n"; 
            #endif  
            kernel->currentThread->True_ticks += kernel->stats->totalTicks - kernel->currentThread->cpu_start_ticks;
            #ifdef DEBUG_PREEMPT
            cout<<"Current ready-queue contents:\n";
            this->List_All_thread();
            #endif

            Thread* nextThread = this->Scheduling();    // Should retun L1 thread.
            #ifdef DEBUG_PREEMPT
            cout<<"Thus, pick "<<nextThread->getName()<<"\n";
            #endif 

            // cout<<"\n";
            this->AddToQueue(kernel->currentThread, kernel->currentThread->priority);  // from RUNNING to READY state.
            this->Run(nextThread, FALSE);
        } 
        // Case 3: Running L1 but the thread to be added has
        //         higher priority (smaller approx_bursttime)
        else if (kernel->currentThread->InWhichQueue()==1 && (thread->approx_bursttime < kernel->currentThread->approx_bursttime &&thread->InWhichQueue()==1)  ) {
            #ifdef DEBUG_PREEMPT
            cout<<"\nShould Preempt!\n";
            cout<<"cur  bursttime: "<< kernel->currentThread->approx_bursttime <<"in queue: "<<kernel->currentThread->InWhichQueue()<<"\n";
            cout<<"Add  bursstime: "<< thread->approx_bursttime<<" belongs to queue "  <<thread->InWhichQueue()<<"\n"; 
            #endif          
            kernel->currentThread->True_ticks += kernel->stats->totalTicks - kernel->currentThread->cpu_start_ticks;
            
            #ifdef DEBUG_PREEMPT
            cout<<"Current ready-queue contents:\n";
            this->List_All_thread();
            #endif
            
            Thread* nextThread = this->Scheduling();    // Should retun L1 thread.
            #ifdef DEBUG_PREEMPT
            cout<<"Thus, pick "<<nextThread->getName()<<"\n";
            #endif 
            // cout<<"\n";
            this->AddToQueue(kernel->currentThread, kernel->currentThread->priority);  // from RUNNING to READY state.
            this->Run(nextThread, FALSE);
        } 

}

// Don't check preempt.
void 
Scheduler::AddToQueue2(Thread* thread, int Priority){
 
    // DEBUG(dbgZ, "[A] Tick [​ {%d}​ ]: Thread [​ {%d}​ ] is inserted into queueL[​ {L1}​ ]");
    cout<<"AddToQueue2: ";
    if(Priority>=100){
        // Add to L1 list.
        // L1List->Append(thread);
        thread->setStatus(READY);
        DEBUG(dbgZ, "[A] Tick [​ {"<< kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<thread->ID<<"}​ ] is inserted into queueL[​ {1}​ ]" );
        L1List->Insert(thread);
        // DEBUG(dbgZ, "[A] Tick [​ {"<< kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<thread->ID<<"}​ ] is inserted into queueL[​ {1}​ ]" );
    }
    else if(Priority >=50){
        // L2List->Append(thread);
        thread->setStatus(READY);
          DEBUG(dbgZ, "[A] Tick [​ {"<< kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<thread->ID<<"}​ ] is inserted into queueL[​ {2}​ ]" );
        L2List->Insert(thread);
        // DEBUG(dbgZ, "[A] Tick [​ {"<< kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<thread->ID<<"}​ ] is inserted into queueL[​ {2}​ ]" );
    }
    else{
        thread->setStatus(READY);
        DEBUG(dbgZ, "[A] Tick [​ {"<< kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<thread->ID<<"}​ ] is inserted into queueL[​ {3}​ ]" );

        #ifdef DEBUG_QUEUES
        if(!L3List->IsEmpty()){
            cout<<"Before: ";
         this->List_All_thread();
        }
        #endif
        L3List->Append(thread);

        #ifdef DEBUG_QUEUES
        if(!L3List->IsEmpty()){
        cout<<"After: ";
        this->List_All_thread();
        }
        #endif    
    
        // DEBUG(dbgZ, "[A] Tick [​ {"<< kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<thread->ID<<"}​ ] is inserted into queueL[​ {3}​ ]" );
    }


}


void 
Scheduler::AddToQueue(Thread* thread, int Priority){
 
    // DEBUG(dbgZ, "[A] Tick [​ {%d}​ ]: Thread [​ {%d}​ ] is inserted into queueL[​ {L1}​ ]");
    
    // set START WAIT time in the ready-queue
    thread->set_wait_starttime(kernel->stats->totalTicks); // update reference point.


    if(Priority>=100){
        // Add to L1 list.
        // L1List->Append(thread);
        thread->setStatus(READY);
        DEBUG(dbgZ, "[A] Tick [​ {"<< kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<thread->ID<<"}​ ] is inserted into queueL[​ {1}​ ]" );
        L1List->Insert(thread);
        // DEBUG(dbgZ, "[A] Tick [​ {"<< kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<thread->ID<<"}​ ] is inserted into queueL[​ {1}​ ]" );
    }
    else if(Priority >=50){
        // L2List->Append(thread);
        thread->setStatus(READY);
          DEBUG(dbgZ, "[A] Tick [​ {"<< kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<thread->ID<<"}​ ] is inserted into queueL[​ {2}​ ]" );
        L2List->Insert(thread);
        // DEBUG(dbgZ, "[A] Tick [​ {"<< kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<thread->ID<<"}​ ] is inserted into queueL[​ {2}​ ]" );
    }
    else{
        thread->setStatus(READY);
        DEBUG(dbgZ, "[A] Tick [​ {"<< kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<thread->ID<<"}​ ] is inserted into queueL[​ {3}​ ]" );

        #ifdef DEBUG_QUEUES
        if(!L3List->IsEmpty()){
            cout<<"Before: ";
         this->List_All_thread();
        }
        #endif
        L3List->Append(thread);

        #ifdef DEBUG_QUEUES
        if(!L3List->IsEmpty()){
        cout<<"After: ";
        this->List_All_thread();
        }
        #endif    
    
        // DEBUG(dbgZ, "[A] Tick [​ {"<< kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<thread->ID<<"}​ ] is inserted into queueL[​ {3}​ ]" );
    }

}


    // #ifdef DEBUG_QUEUE
    // cout<<"AddToQueue: After Add: "<<thread->getName() <<"\n";
    // this->List_All_thread();
    // #endif

    // if(kernel->currentThread->getID()!=0){
    //     kernel->scheduler->Check_Preempt(thread);
    // } 





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

Thread*
Scheduler::FindNextToRun2(){
    
    ASSERT(kernel->interrupt->getLevel() == IntOff);
    
    if( ! L1List->IsEmpty()){

        return L1List->RemoveFront();
    }
    else{
        if(!L2List->IsEmpty()){
            return L2List->RemoveFront();
        }
        else{
            if(!L3List->IsEmpty()){
                return L3List->RemoveFront();
            }
            else return NULL;
        }
    }

}

/// 12/19 ///

//----------------------------------------------------------------------
// Scheduler::Aging
//  Iterate through all threads in multi-level ready-queues
//  And check whether to add priorities to each READY thread.	
// 
//----------------------------------------------------------------------

void
Scheduler::Aging(){
    Thread* thread;
    int totalTicks = kernel->stats->totalTicks;
        // L1 queue : note MAX 149
        if(!L1List->IsEmpty()){
        ListIterator<Thread*> *iter;
        iter = new ListIterator<Thread*>(L1List);            
            for (; !iter->IsDone(); iter->Next()) {
                thread = iter->Item();
                // thread->accu_wait_ticks += 100;
                thread->accu_wait_ticks += totalTicks - thread->start_wait_ticks;
                thread->start_wait_ticks = totalTicks; // update reference point.
                #ifdef DEBUG_AGING
                cout<<"Ticks: "<<totalTicks<<", thread "<<thread->getID()<<"wait tick: "<<thread->accu_wait_ticks<<"\n";
                #endif                
                if(thread->accu_wait_ticks >= 1500){
                    if(thread->priority+10>=149){
                        DEBUG(dbgZ,  "[C] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<iter->Item()->ID<<"}​ ] changes its priority from [​ {"<<iter->Item()->priority<<"}​ ] to [​ {149}​ ]");
                        thread->priority = 149;
                    }
                    else{
                        DEBUG(dbgZ,  "[C] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<iter->Item()->ID<<"}​ ] changes its priority from [​ {"<<iter->Item()->priority<<"}​ ] to [​ {"<<iter->Item()->priority+10 <<"}​ ]");
                        thread->priority += 10;
                    }    
                    // reset wait ticks.
                    // thread->accu_wait_ticks = 0;
                    thread->accu_wait_ticks -= 1500;  // Should NOT reset!
                    
                }                
            }                
        }

        // L2
        if(!L2List->IsEmpty()){
                ListIterator<Thread*> *iter;
                iter = new ListIterator<Thread*>(L2List);

                for (; !iter->IsDone(); iter->Next()) {
                    thread = iter->Item();
                    // thread->accu_wait_ticks += 100;
                    thread->accu_wait_ticks += totalTicks - thread->start_wait_ticks;
                    thread->start_wait_ticks = totalTicks; // update reference point.
                    #ifdef DEBUG_AGING
                    cout<<"Ticks: "<<totalTicks<<", thread "<<thread->getID()<<"wait tick: "<<thread->accu_wait_ticks<<"\n";
                    #endif
                    // thread->accu_wait_ticks = totalTicks - thread->start_wait_ticks;
                    if(thread->accu_wait_ticks >= 1500){
                        
                        // Print DEBUG message.
                        DEBUG(dbgZ,  "[C] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<iter->Item()->ID<<"}​ ] changes its priority from [​ {"<<iter->Item()->priority<<"}​ ] to [​ {"<<iter->Item()->priority+10 <<"}​ ]");
                        thread->priority += 10;
                        // reset wait ticks.
                        // thread->set_wait_starttime(totalTicks);
                        // thread->accu_wait_ticks = 0;
                        thread->accu_wait_ticks -= 1500;  // Should NOT reset!
                    } 
                }                
        }

        // L3 queue
        if(!L3List->IsEmpty()){   
                ListIterator<Thread*> *iter;
                iter = new ListIterator<Thread*>(L3List);

                for (; !iter->IsDone(); iter->Next()) {
                    thread = iter->Item();
                    // thread->accu_wait_ticks += 100;
                    thread->accu_wait_ticks += totalTicks - thread->start_wait_ticks;
                    thread->start_wait_ticks = totalTicks; // update reference point.
                    #ifdef DEBUG_AGING
                    cout<<"Ticks: "<<totalTicks<<", thread "<<thread->getID()<<"wait tick: "<<thread->accu_wait_ticks<<"\n";
                    #endif                   
                   
                    // thread->accu_wait_ticks = totalTicks - thread->start_wait_ticks;
                    if(thread->accu_wait_ticks >= 1500){
                        DEBUG(dbgZ,  "[C] Tick [​ {"<<kernel->stats->totalTicks<<"}​ ]: Thread [​ {"<<iter->Item()->ID<<"}​ ] changes its priority from [​ {"<<iter->Item()->priority<<"}​ ] to [​ {"<<iter->Item()->priority+10 <<"}​ ]");
                        thread->priority += 10;
                        // thread->set_wait_starttime(totalTicks);
                        // thread->accu_wait_ticks = 0;
                        thread->accu_wait_ticks -= 1500;  // Should NOT reset!
                    } 
                }                
        }

}



////////////////////////// 12/15 ///////////////////

// Display all thread in ready queues.
void
Scheduler::List_All_thread(){

        cout<<"L1 queue: ";
        if(!L1List->IsEmpty()){
        ListIterator<Thread*> *iter;
        iter = new ListIterator<Thread*>(L1List);            
            for (; !iter->IsDone(); iter->Next()) {
                
                        // cout<< iter->Item()->getName()<<" id "<<iter->Item()->ID <<" /";
                cout<< iter->Item()->getName()<<" ";
            }                
            cout<<"  \n";
        }
        // else cout<< "empty\n";
        else cout<< "\n";

        cout<<"L2 queue: ";
        if(!L2List->IsEmpty()){
                ListIterator<Thread*> *iter;
                iter = new ListIterator<Thread*>(L2List);

                for (; !iter->IsDone(); iter->Next()) {
                    // cout<< iter->Item()->getName()<<" id "<<iter->Item()->ID <<" /";
                    cout<< iter->Item()->getName()<<" ";
                }                
                cout<<"  \n";
        }
        // else cout<< "empty\n";
        else cout<< "\n";


        cout<<"L3 queue: ";
        if(!L3List->IsEmpty()){   
                ListIterator<Thread*> *iter;
                iter = new ListIterator<Thread*>(L3List);

                for (; !iter->IsDone(); iter->Next()) {
                    // cout<< iter->Item()->getName()<<" id "<<iter->Item()->ID <<" /";
                    cout<< iter->Item()->getName()<<" ";
                }                
                cout<<"  \n";
        }
        // else cout<< "empty\n";
        else cout<< "\n";
}

//----------------------------------------------------------------------
// Scheduler::StillHasThreadToRun
// 	Return the next thread to be scheduled onto the CPU.
//	If there are no ready threads, return NULL.
// Side effect:
//	Thread is removed from the ready list.
//----------------------------------------------------------------------

bool
Scheduler::StillHasThreadToRun ()
{
    ASSERT(kernel->interrupt->getLevel() == IntOff);

    if( ! L1List->IsEmpty()){

                #ifdef LIST_QUEUE_CONTENTS
                cout<<"L1 queue: ";
                ListIterator<Thread*> *iter;
                iter = new ListIterator<Thread*>(L1List);

                for (; !iter->IsDone(); iter->Next()) {
                    // cout<< iter->Item()->getName()<<" id "<<iter->Item()->ID <<" /";
                    cout<< iter->Item()->getName()<<" /";
                }                
                cout<<"  \n";
                #endif



        return TRUE;
    }
    else{
        if(!L2List->IsEmpty()){

                #ifdef LIST_QUEUE_CONTENTS
                cout<<"L2 queue: ";
                ListIterator<Thread*> *iter;
                iter = new ListIterator<Thread*>(L2List);

                for (; !iter->IsDone(); iter->Next()) {
                    // cout<< iter->Item()->getName()<<" id "<<iter->Item()->ID <<" /";
                    cout<< iter->Item()->getName()<<" /";
                }                
                cout<<"  \n";
                #endif

            return TRUE;
        }
        else{
            if(!L3List->IsEmpty()){

                #ifdef LIST_QUEUE_CONTENTS
                cout<<"L3 queue: ";
                ListIterator<Thread*> *iter;
                iter = new ListIterator<Thread*>(L3List);

                for (; !iter->IsDone(); iter->Next()) {
                    // cout<< iter->Item()->getName()<<" id "<<iter->Item()->ID <<" /";
                    cout<< iter->Item()->getName()<<" /";
                }                
                cout<<"  \n";
                #endif
                
                return TRUE;
            }
            else return FALSE;
        }
    }

    // if (readyList->IsEmpty()) {
	// 	return NULL;
    // } else {
    // 	return readyList->RemoveFront();
    // }
}

// // Return kernel->currentThread and remove it from
// Thread*
// Scheduler::ReturnCurrentThread(){


// }


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

    ///////// 12/15 /////////
    // Should UPDATE BURST TIME when call thread->Sleep(FALSE)!!!
    // Update approx. burst time for OLD thread.
    // Get current ticks. (Update this thread's approx. cpu burst ticks)
    // oldThread->update_burst_time(kernel->stats->totalTicks);

    kernel->currentThread = nextThread;  // switch to the next thread
    nextThread->setStatus(RUNNING);      // nextThread is now running
    
    DEBUG(dbgThread, "Switching from: " << oldThread->getName() << " to: " << nextThread->getName());
    
    // This is a machine-dependent assembly language routine defined 
    // in switch.s.  You may have to think
    // a bit to figure out what happens after this, both from the point
    // of view of the thread and from the perspective of the "outside world".



    SWITCH(oldThread, nextThread);

    // we're back, running oldThread

    ///////// 12/10 /////////
    // Resume accumulating CPU burst ticks.
    // kernel->currentThread->record_start_ticks(kernel->stats->totalTicks);
    // cout<<"thread "<<kernel->currentThread->getID()<<" update start ticks: "<<  kernel->currentThread->cpu_start_ticks<<std::endl;
    // // interrupts are off when we return from switch!
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

