/**************************************************************
 *
 * userprog/ksyscall.h
 *
 * Kernel interface for systemcalls 
 *
 * by Marcus Voelp  (c) Universitaet Karlsruhe
 *
 **************************************************************/

#ifndef __USERPROG_KSYSCALL_H__ 
#define __USERPROG_KSYSCALL_H__ 

#include "kernel.h"

#include "synchconsole.h"


// TODO: add SysOpen(char *name), SysWrite(buffer, size, id ), SysRead(buffer, size, id), SysClose(OpenFileid id)
// Q : void or int ? (Later,since return -1 if failed.)
// void SysOpen(char *filename)
// {
//   kernel->fileSystem->Open(filename);
// }

// void SysWrite(char *buffer, int size, OpenFileId id)
// {
//   kernel->fileSystem->Write(buffer, size, id);
// }

// void SysRead(char *buffer, int size, OpenFileId id)
// {
//   kernel->fileSystem->Read(buffer, size, id);
// }

// void SysClose(OpenFileId id)
// {
//   kernel->fileSystem->Close(filename);
// }




void SysHalt()
{
  kernel->interrupt->Halt();
}

void SysPrintInt(int val)
{ 
  DEBUG(dbgTraCode, "In ksyscall.h:SysPrintInt, into synchConsoleOut->PutInt, " << kernel->stats->totalTicks);
  kernel->synchConsoleOut->PutInt(val);
  DEBUG(dbgTraCode, "In ksyscall.h:SysPrintInt, return from synchConsoleOut->PutInt, " << kernel->stats->totalTicks);
}

int SysAdd(int op1, int op2)
{
  return op1 + op2;
}

int SysCreate(char *filename)
{
	// return value
	// 1: success
	// 0: failed
	return kernel->fileSystem->Create(filename);
}

//When you finish the function "OpenAFile", you can remove the comment below.

OpenFileId SysOpen(char *name)
{
  return kernel->fileSystem->OpenAFile(name);
}


int SysWrite(char *buffer, int size, OpenFileId id)
{
  return kernel->fileSystem->Write(buffer, size, id);
}

int SysRead(char *buffer, int size, OpenFileId id)
{
  return kernel->fileSystem->ReadFile(buffer, size, id);
}

int SysClose(OpenFileId id)
{
  return kernel->fileSystem->CloseFile(id);
}

#endif /* ! __USERPROG_KSYSCALL_H__ */