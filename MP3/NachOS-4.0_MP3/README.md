# MP3 CPU Scheduling Implementation Notes.


[TOC]

## 12/14

Hello ivy liu.
You can write anything you want here.
We should use this tool earlier!
hehehaha（＾ν＾）

omg how do you type this icon?
Are U an artist?
hehehahatony

### Files I have touched:

`threads/thread.h`:

![](https://i.imgur.com/Oqomowc.png)


`threads/thread.cc`:

```c=
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
}

```

```c=
//////////// 12/10 Added /////////////////
//----------------------------------------------------------------------
// Thread::record_start_ticks
// 	Record starting tick when a thread gets the CPU.
//
// Thread::record_start_ticks
// 	Record starting tick when a thread gets the CPU.
//

//----------------------------------------------------------------------


void 
Thread::record_start_ticks(int cpu_start_ticks){
    this->cpu_start_ticks = cpu_start_ticks;
}

void
Thread::update_burst_time(int cpu_end_ticks){
    this->cpu_end_ticks = cpu_end_ticks;
    int True_burst_ticks = this->cpu_end_ticks - this->cpu_start_ticks;
    
    // Update approximate cpu burst time ti.
    this->approx_bursttime = 0.5*True_burst_ticks + 0.5*this->last_approx_burstiime;

    // update last approx. bursttime. (ti-1)
    this->last_approx_burstiime = this->approx_bursttime;

}

```




`threads/scheduler.h`

```c=
    /////////// 12/10 Added //////////////////
    // TODO(1) : Add a scheduling function when a threads is add to ready-queue.(readyList)
    // i.e. : When someone calls "kernel->scheduler->ReadyToRun(this)".
    // Add to L1List/ L2List / L3List according to its priority (aging).
    void Scheduling(bool finishing);

    // TODO(2) : Add a function similar to "ReadyToRun" but with
    //           arguments: 1. Thread* 2. Its Priority to decide 
    //           which level queue it should be added. (L1, L2, L3) 
    void AddToQueue(Thread* thread, int Priority);
    
    void AddThreadPriority();
    void ReArrangeThreads();

  private:
    List<Thread *> *readyList;  // queue of threads that are ready to run,
				// but not running
    
    /////////// 12/10 Added //////////////////
    // List<Thread *> *L1List; // Preemptive SFJ 
    // List<Thread *> *L2List; // Non-preemptive priority
   
    /// 3-Level queue: sorted with threir priority!
    SortedList<Thread *> *L1List;	
    SortedList<Thread *> *L2List;	
    List<Thread *> *L3List; // Round-Robin TQ:100 ticks 


    Thread *toBeDestroyed;	// finishing thread to be destroyed
    				// by the next thread that runs
};
```


`threads/scheduler.cc`

```c++=
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


```

In `Scheduler::Run()` : 

Add UpdateBurstTime and 
RecordStartTime

```c=
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

```



`threads/kernel.h`

**`int   threadPriority[10];`**

```c=

    int Exec(char* name, int Initial_Priority);
    ...
private:

	Thread* t[10];
	char*   execfile[10];
    int   threadPriority[10];
```


`threads/kernel.cc`

```c=
		} else if (strcmp(argv[i], "-e") == 0) {
        	execfile[++execfileNum]= argv[++i];
            threadPriority[execfileNum] = 0; // default.
			cout << execfile[execfileNum] << "\n";
        } else if (strcmp(argv[i], "-ep") == 0){        //To be modified
            /* code */                                  //add prioity to threads
            execfile[++execfileNum]= argv[++i];         // First get fileName
            threadPriority[execfileNum] = atoi(argv[++i]);    // Then get initial priority
            cout << "launch "<<execfile[execfileNum]<<" with priority "<< threadPriority[execfileNum]<< "\n";

        } else if (strcmp(argv[i], "-ci") == 0) {
```


```c=
void Kernel::ExecAll()
{
	for (int i=1;i<=execfileNum;i++) {
		// int a = Exec(execfile[i]); // a = threadID . Not used here.
    	int a = Exec(execfile[i], threadPriority[i]); 
    }
	currentThread->Finish();
    //Kernel::Exec();	
}
```


```c=
// int Kernel::Exec(char* name)
int Kernel::Exec(char* name, int Initial_Priority)
{
	t[threadNum] = new Thread(name, threadNum, Initial_Priority);
    // t[threadNum] = new Thread(name, threadNum);
    // Thread* orig_thread = kernel->currentThread;
```





`threads/alarm.cc`:

```c=

void 
Alarm::CallBack() 
{
    Interrupt *interrupt = kernel->interrupt;
    MachineStatus status = interrupt->getStatus();
    
    if (status != IdleMode) {

        //////////// 12/12 ////////////////////
        // (1)
        kernel->scheduler->AddThreadPriority();
        // (2)
        kernel->scheduler->ReArrangeThreads();
        ///////////////////////////////////////

	interrupt->YieldOnReturn();
    }
}

```

`threads/kernel.cc`
`in Kernel::Initialize() and Kernel::~Kernel()`


