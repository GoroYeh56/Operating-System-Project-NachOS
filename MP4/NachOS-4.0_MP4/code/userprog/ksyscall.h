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

void SysHalt()
{
	kernel->interrupt->Halt();
}

int SysAdd(int op1, int op2)
{
	return op1 + op2;
}


/* ------------ MP4 ----------*/

int SysCreate(char *filename, int initialSize)
{
	// return value
	// 1: success
	// 0: failed
	return kernel->fileSystem->Create(filename, initialSize); //must create a file from user program.
}

/* Open the Nachos file "name", and return an "OpenFileId" that can 
 * be used to read and write to the file.
 */
OpenFileId SysOpen(char *name){
	OpenFileId ofid = (OpenFileId) kernel->fileSystem->Open(name);
	if(ofid>0) return ofid;
	else return -1; // fail.
	// return kernel->fileSystem->Open(name);
}

/* Write "size" bytes from "buffer" to the open file. 
 * Return the number of bytes actually read on success.
 * On failure, a negative error code is returned.
 */
int SysWrite(char *buffer, int size, OpenFileId id){

	return kernel->fileSystem->Write(buffer, size, id);
}

/* Read "size" bytes from the open file into "buffer".  
 * Return the number of bytes actually read -- if the open file isn't
 * long enough, or if it is an I/O device, and there aren't enough 
 * characters to read, return whatever is available (for I/O devices, 
 * you should always wait until you can return at least one character).
 */
int SysRead(char *buffer, int size, OpenFileId id){
	return kernel->fileSystem->Read(buffer, size, id);

}

/* Close the file, we're done reading and writing to it.
 * Return 1 on success, negative error code on failure
 */
int SysClose(OpenFileId id){
	return kernel->fileSystem->Close(id);

}

/* ------------------------------------------------------*/

#ifdef FILESYS_STUB
int SysCreate(char *filename)
{
	// return value
	// 1: success
	// 0: failed
	return kernel->interrupt->CreateFile(filename);
}
#endif

#endif /* ! __USERPROG_KSYSCALL_H__ */
