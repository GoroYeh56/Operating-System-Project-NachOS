// filesys.h 
//	Data structures to represent the Nachos file system.
//
//	A file system is a set of files stored on disk, organized
//	into directories.  Operations on the file system have to
//	do with "naming" -- creating, opening, and deleting files,
//	given a textual file name.  Operations on an individual
//	"open" file (read, write, close) are to be found in the OpenFile
//	class (openfile.h).
//
//	We define two separate implementations of the file system. 
//	The "STUB" version just re-defines the Nachos file system 
//	operations as operations on the native UNIX file system on the machine
//	running the Nachos simulation.
//
//	The other version is a "real" file system, built on top of 
//	a disk simulator.  The disk is simulated using the native UNIX 
//	file system (in a file named "DISK"). 
//
//	In the "real" implementation, there are two key data structures used 
//	in the file system.  There is a single "root" directory, listing
//	all of the files in the file system; unlike UNIX, the baseline
//	system does not provide a hierarchical directory structure.  
//	In addition, there is a bitmap for allocating
//	disk sectors.  Both the root directory and the bitmap are themselves
//	stored as files in the Nachos file system -- this causes an interesting
//	bootstrap problem when the simulated disk is initialized. 
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef FS_H
#define FS_H

// #define FILESYS_STUB

#include "copyright.h"
#include "sysdep.h"
#include "openfile.h"
#include "debug.h" 		//just for test!!!

#ifdef FILESYS_STUB 		// Temporarily implement file system calls as 
				// calls to UNIX, until the real file system
				// implementation is available
typedef int OpenFileId;

class FileSystem {
  public:
    // Initialize its OpenFileTable with 20 NULL element!
    FileSystem() { 
	for (int i = 0; i < 20; i++){
        OpenFileTable[i] = NULL; 
        id_to_fd[i] = -1;
        // initial index.
        // file_to_OpenFile_Table[i] = NULL;
    }
    // counter = 0;
    }

    bool Create(char *name) {
	int fileDescriptor = OpenForWrite(name);

	if (fileDescriptor == -1) return FALSE;
	Close(fileDescriptor); 
	return TRUE; 
    }

    // Return an OpenFile object, which can do Open, Read, Write, CLose operation to that file.
    // By using its fileDescriptor.
//The OpenFile function is used for open user program  [userprog/addrspace.cc]
    OpenFile* Open(char *name) {
	int fileDescriptor = OpenForReadWrite(name, FALSE);
	if (fileDescriptor == -1) return NULL;
	return new OpenFile(fileDescriptor);
    }

  

    
    //// TODOD : /////
//  The OpenAFile function is used for kernel open system call

    OpenFileId OpenAFile(char *name) {
        // open file
        for(int i=0; i<20; i++){
            if(OpenFileTable[i]==NULL){
                // can use.

            int fileDescriptor = OpenForReadWrite(name, FALSE);
            cout << "OpenAFile fileDescriptor:" << fileDescriptor << endl;
            if (fileDescriptor == -1){
                cout<<"File not existed, cannot open!"<<endl;
                return -1;
            } 

            id_to_fd[i] = fileDescriptor;
        
            OpenFile* openfile = new OpenFile(fileDescriptor);
            OpenFileTable[i] = openfile; //new OpenFile(fileDescriptor);
            return i;

            }
        }
        return -1; // if all full.


    }

    int Write(char *buffer, int size, OpenFileId id){
         if(OpenFileTable[id] == NULL) return -1;  // Avoid invalid operation
         int num_bytes = OpenFileTable[id]->Write(buffer, size*sizeof(char));
         return num_bytes/sizeof(char);      
    }


    int ReadFile(char *buffer, int size, OpenFileId id){
        if(OpenFileTable[id] == NULL) return -1;   // Avoid invalid operation
        // std::cout<<buffer<<std::endl;
        int num_bytes = OpenFileTable[id]->Read(buffer, size*sizeof(char));
        // std::cout<<buffer<<std::endl;
        return num_bytes/sizeof(char);
    }

    int CloseFile(OpenFileId id){
        // id: the index of OpenFileTable
        // should check whether the id is valid.
        if(OpenFileTable[id] == NULL) return -1; // invalid file
        // else
        // OpenFileTable[id]->~OpenFile();
        // OpenFileTable[id]->~OpenFile(); // destructor would close the file
        OpenFileTable[id] = NULL;
        // Note: Close (int fd) should pass FileDescriptor!!!
        int retval = Close(id_to_fd[id]);
        return retval; //must be >= 0 
    }

    // OpenFileId counter;
    int id_to_fd[20];

    bool Remove(char *name) { return Unlink(name) == 0; }

    // OpenFileId File_to_OpenTableEntry[20]; //index: file_id, output: INDEX of OpenFileTable.

    OpenFile *OpenFileTable[20];
};

#else // FILESYS
class FileSystem {
  public:
    FileSystem(bool format);		// Initialize the file system.
					// Must be called *after* "synchDisk" 
					// has been initialized.
    					// If "format", there is nothing on
					// the disk, so initialize the directory
    					// and the bitmap of free blocks.

    bool Create(char *name, int initialSize);  	
					// Create a file (UNIX creat)

    OpenFile* Open(char *name); 	// Open a file (UNIX open)

    bool Remove(char *name);  		// Delete a file (UNIX unlink)

    void List();			// List all the files in the file system

    void Print();			// List all the files and their contents

  private:
   OpenFile* freeMapFile;		// Bit map of free disk blocks,
					// represented as a file
   OpenFile* directoryFile;		// "Root" directory -- list of 
					// file names, represented as a file
};

#endif // FILESYS

#endif // FS_H
