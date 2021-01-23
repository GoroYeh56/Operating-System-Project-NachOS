// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "ksyscall.h"
//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// If you are handling a system call, don't forget to increment the pc
// before returning. (Or else you'll loop making the same system call forever!)
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	is in machine.h.
//----------------------------------------------------------------------
void
ExceptionHandler(ExceptionType which)
{
    char ch;
    int val, filename_addr, buf_addr, size_addr, id_addr;
	OpenFileId id;
	// int id;
	int size;  // Read / Write char size.

    int type = kernel->machine->ReadRegister(2);
    int status, exit, threadID, programID, fileID, numChar;
    DEBUG(dbgSys, "Received Exception " << which << " type: " << type << "\n");
    DEBUG(dbgTraCode, "In ExceptionHandler(), Received Exception " << which << " type: " << type << ", " << kernel->stats->totalTicks);
    switch (which) {
    case SyscallException:
	switch(type) {
		// Todo: ADD SC_Open, SC_Write, SC_Read, SC_Close here. (is SyscallException?)
		case SC_Open: // 6
			DEBUG(dbgSys, "SC_Open, open a file and return OpenFileid.\n");
			filename_addr = kernel->machine->ReadRegister(4);
			{
			char *filename = &(kernel->machine->mainMemory[filename_addr]);
			cout << "Open filename: "<<filename << endl;
			id = SysOpen(filename); // return OpenFileId id
			// cout<<"id: "<<id<<endl;
			// write result into r2.
			kernel->machine->WriteRegister(2, id);
			// kernel->machine->WriteRegister(2, (OpenFileid) status);
			}
			//// Write Program Counter
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();
		break;

		case SC_Read: // 7
			DEBUG(dbgSys, "SC_Read, read a file(id) into buf, size of element(each sizeof(char)).\n");
			buf_addr = kernel->machine->ReadRegister(4);
			size = kernel->machine->ReadRegister(5);
			id = kernel->machine->ReadRegister(6);
			{
			char *buf = &(kernel->machine->mainMemory[buf_addr]);
			cout<<"Read "<<size<<" char from file with id: "<<id<<endl;

			numChar = SysRead(buf, size, id); // return OpenFileId id
			// cout<<"id: "<<id<<endl;
			// write result into r2.
			cout<<"return number of char: "<< numChar <<endl;
			kernel->machine->WriteRegister(2, (int) numChar);
			}
			//// Write Program Counter
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();
		break;

		case SC_Write: // 8
			DEBUG(dbgSys, "SC_Write, write buf into a file(id), with size of element(each sizeof(char)).\n");
			buf_addr= kernel->machine->ReadRegister(4);
			size = kernel->machine->ReadRegister(5);
			id = kernel->machine->ReadRegister(6);
			{
			char *buf = &(kernel->machine->mainMemory[buf_addr]);
			// cout<< "buf[0]: "<<buf[0]<<endl;
			cout<<"write "<<size<<" char into file with id: "<<id<<endl;
			// cout<<"buffer content: "<<buf<<endl; // write buffer into.
			status = SysWrite(buf, size, id); // return number of char written into
			cout<<"return number of char: "<<status<<endl;
			// write result into r2.
			kernel->machine->WriteRegister(2, (int) status);
			}
			//// Write Program Counter
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();

		break;
			
		case SC_Close: // 10
			DEBUG(dbgSys, "SC_Close, close a file with OpenFileid.\n");
			id = kernel->machine->ReadRegister(4);
			{
			// id = kernel->machine->mainMemory[id_addr];
			
			//cout << filename << endl;
			status = SysClose(id); // return status
			// write result into r2.
			if(status >=0) status = 1;
			else status = -1;
			kernel->machine->WriteRegister(2, (int) status);
			}
			//// Write Program Counter
			kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
			kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
			return;
			ASSERTNOTREACHED();

		break;				

	    case SC_Halt:
		DEBUG(dbgSys, "Shutdown, initiated by user program.\n");
		SysHalt();
		cout<<"in exception\n";
		ASSERTNOTREACHED();
	    break;
	    case SC_PrintInt:
		DEBUG(dbgSys, "Print Int\n");
		val=kernel->machine->ReadRegister(4);
		DEBUG(dbgTraCode, "In ExceptionHandler(), into SysPrintInt, " << kernel->stats->totalTicks);    
		SysPrintInt(val); 	
		// q : debug Syshalt?
		DEBUG(dbgTraCode, "In ExceptionHandler(), return from SysPrintInt, " << kernel->stats->totalTicks);
		// Set Program Counter
		kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
		kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
		kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
		return;
		ASSERTNOTREACHED();
	    break;

	    case SC_MSG:
		DEBUG(dbgSys, "Message received.\n");
		val = kernel->machine->ReadRegister(4);
		{
		char *msg = &(kernel->machine->mainMemory[val]);
		cout << msg << endl;
		}
		SysHalt();
		ASSERTNOTREACHED();
	    break;

	    case SC_Create:
		val = kernel->machine->ReadRegister(4); // read filename.
		{
		char *filename = &(kernel->machine->mainMemory[val]);
		//cout << filename << endl;
		// return OpenFileId id.
		status = SysCreate(filename);
		kernel->machine->WriteRegister(2, (int) status);
		}
		kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
		kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
		kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
		return;
		ASSERTNOTREACHED();
	    break;
		
      	    case SC_Add:
		DEBUG(dbgSys, "Add " << kernel->machine->ReadRegister(4) << " + " << kernel->machine->ReadRegister(5) << "\n");
		/* Process SysAdd Systemcall*/
		int result;
		result = SysAdd(/* int op1 */(int)kernel->machine->ReadRegister(4),
		/* int op2 */(int)kernel->machine->ReadRegister(5));
		DEBUG(dbgSys, "Add returning with " << result << "\n");
		/* Prepare Result */
		kernel->machine->WriteRegister(2, (int)result);	
		/* Modify return point */
		{
		/* set previous programm counter (debugging only)*/
		kernel->machine->WriteRegister(PrevPCReg, kernel->machine->ReadRegister(PCReg));
			
		/* set programm counter to next instruction (all Instructions are 4 byte wide)*/
		kernel->machine->WriteRegister(PCReg, kernel->machine->ReadRegister(PCReg) + 4);
	  
		/* set next programm counter for brach execution */
		kernel->machine->WriteRegister(NextPCReg, kernel->machine->ReadRegister(PCReg)+4);
		}
		cout << "result is " << result << "\n";	
		return;	
		ASSERTNOTREACHED();
	    break;
	    case SC_Exit:
			DEBUG(dbgAddr, "Program exit\n");
            		val=kernel->machine->ReadRegister(4);
            		cout << "return value:" << val << endl;
			kernel->currentThread->Finish();
            break;
      	    default:
		cerr << "Unexpected system call " << type << "\n";
	    break;
	}
	break;
	default:
		cerr << "Unexpected user mode exception " << (int)which << "\n";
		break;
    }
    ASSERTNOTREACHED();
}

