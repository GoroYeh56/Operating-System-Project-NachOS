// filehdr.cc
//	Routines for managing the disk file header (in UNIX, this
//	would be called the i-node).
//
//	The file header is used to locate where on disk the
//	file's data is stored.  We implement this as a fixed size
//	table of pointers -- each entry in the table points to the
//	disk sector containing that portion of the file data
//	(in other words, there are no indirect or doubly indirect
//	blocks). The table size is chosen so that the file header
//	will be just big enough to fit in one disk sector,
//
//      Unlike in a real system, we do not keep track of file permissions,
//	ownership, last modification date, etc., in the file header.
//
//	A file header can be initialized in two ways:
//	   for a new file, by modifying the in-memory data structure
//	     to point to the newly allocated data blocks
//	   for a file already on disk, by reading the file header from disk
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"

#include "filehdr.h"
#include "debug.h"
#include "synchdisk.h"
#include "main.h"







//----------------------------------------------------------------------
// MP4 mod tag
// FileHeader::FileHeader
//	There is no need to initialize a fileheader,
//	since all the information should be initialized by Allocate or FetchFrom.
//	The purpose of this function is to keep valgrind happy.
//----------------------------------------------------------------------
FileHeader::FileHeader()
{
	numBytes = -1;
	numSectors = -1;
	memset(dataSectors, -1, sizeof(dataSectors));
}

//----------------------------------------------------------------------
// MP4 mod tag
// FileHeader::~FileHeader
//	Currently, there is not need to do anything in destructor function.
//	However, if you decide to add some "in-core" data in header
//	Always remember to deallocate their space or you will leak memory
//----------------------------------------------------------------------
FileHeader::~FileHeader()
{
	// nothing to do now
}

//----------------------------------------------------------------------
// FileHeader::Allocate
// 	Initialize a fresh file header for a newly created file.
//	Allocate data blocks for the file out of the map of free disk blocks.
//	Return FALSE if there are not enough free blocks to accomodate
//	the new file.
//
//	"freeMap" is the bit map of free disk sectors
//	"fileSize" is the size of this file you want to create
//----------------------------------------------------------------------

// numBytes, numSectors are private members of 'FileHeader' class.

bool FileHeader::Allocate(PersistentBitmap *freeMap, int fileSize)
{
	numBytes = fileSize;
	// KEY !!!　Below: use fileSize instead of private member. numByes!! Since recursive
	
	numSectors = divRoundUp(fileSize, SectorSize);  // need numSectors to support total 'fileSize' bytes
	// 1. Check whether still have free space.
	DEBUG(dbgFileCreate, "This file needs "<<numSectors<<" sectors");
	if (freeMap->NumClear() < numSectors)
		return FALSE; // not enough space

	if(fileSize > num_of_bytes_4level ){ // Four-level indirect (~100 MB)// single indirect (orignal nachos)
		DEBUG(dbgFileCreate, ">4level: "<<num_of_bytes_4level<<" bytes, need "<<fileSize<<" bytes");
		int i=0; // index for dataSectors.
		while(fileSize>0){	// still needs to allocate.
			dataSectors[i] = freeMap->FindAndSet();
			FileHeader* sub_hdr = new FileHeader(); // sub header
			// This if-else is used for: when the last entry(dataSector) is assign, and its remain size(fileSize)<num_of_bytes_4level
			// We just allocate space on Disk for the rest(remaining) bytes. (go else{})
			if(fileSize >  num_of_bytes_4level){
				sub_hdr->Allocate(freeMap, num_of_bytes_4level); // using the same bitmap to allocate
			}
			else{
				sub_hdr->Allocate(freeMap, fileSize);
			}
			fileSize -= num_of_bytes_4level;	// Every while-loop we allocate some size, so remember to minus num_of_bytes_4level
			sub_hdr->WriteBack(dataSectors[i]); // Write Back to disk.
			i++; // move to the next dataSector.
		}
		
	}
	else if(fileSize > num_of_bytes_3level){ // tripple indirect (~3MB)
		int i=0; // index for dataSectors.
		DEBUG(dbgFileCreate, ">3level: "<<num_of_bytes_3level<<" bytes, need "<<fileSize<<" bytes");
		while(fileSize>0){	// still needs to allocate.
			dataSectors[i] = freeMap->FindAndSet();
			FileHeader* subdir = new FileHeader();
			if(fileSize >  num_of_bytes_3level){
				subdir->Allocate(freeMap, num_of_bytes_3level);
			}
			else{
				subdir->Allocate(freeMap, fileSize);
			}
			fileSize -= num_of_bytes_3level;
			subdir->WriteBack(dataSectors[i]);
			i++; // move to the next dataSector.
		}
	}
	else if(fileSize > num_of_bytes_2level){ // double indirect (~128KB)
		int i=0; // index for dataSectors.
		DEBUG(dbgFileCreate, ">2level: "<<num_of_bytes_2level<<" bytes, need "<<fileSize<<" bytes");
		while(fileSize>0){	// still needs to allocate.
			dataSectors[i] = freeMap->FindAndSet();
			FileHeader* subdir = new FileHeader();
			if(fileSize >  num_of_bytes_2level){
				subdir->Allocate(freeMap, num_of_bytes_2level);
			}
			else{
				subdir->Allocate(freeMap, fileSize);
			}
			fileSize -= num_of_bytes_2level;
			subdir->WriteBack(dataSectors[i]);
			i++; // move to the next dataSector.
		}
	}	
	else if(fileSize > num_of_bytes_1level){ // double indirect (~4KB)
		int i=0; // index for dataSectors.
		DEBUG(dbgFileCreate, ">1level: "<<num_of_bytes_1level<<" bytes, need "<<fileSize<<" bytes");
		while(fileSize>0){	// still needs to allocate.
			// std::cout<<"\n i=: "<<i<< "in level 1, remain "<<fileSize<<" bytes\n";
			dataSectors[i] = freeMap->FindAndSet();
			FileHeader* subdir = new FileHeader();
			if(fileSize >  num_of_bytes_1level){
				subdir->Allocate(freeMap, num_of_bytes_1level);
			}
			else{
				subdir->Allocate(freeMap, fileSize);
			}
			fileSize -= num_of_bytes_1level;
			subdir->WriteBack(dataSectors[i]);
			i++; // move to the next dataSector.
		}
	}
	else{	// single indirect (orignal nachos)
		DEBUG(dbgFileCreate, "<= 1level: "<<num_of_bytes_1level<<" bytes, need "<<fileSize<<" bytes");
		#ifdef DEBUG_FHDR_ALLOCATE
		std::cout<<"Original nachOS single indirect\n";
		#endif

		for (int i = 0; i < numSectors; i++)
		{
			// Allocate datablock here! One sector at a time
			dataSectors[i] = freeMap->FindAndSet();
			// since we checked that there was enough free space,
			// we expect this to succee

			DEBUG(dbgFileCreate, "dataSector["<<i<<"] takes sector "<<dataSectors[i]);
			ASSERT(dataSectors[i] >= 0); // sector number should be >=0

			///////// MP4 KEY !!!!! CLEAN DISK HERE!!! ////////////
			char* clean_data = new char[SectorSize]();
			// memset(clean_data, 0, SectorSize);
			kernel->synchDisk->WriteSector(dataSectors[i], clean_data);
			delete clean_data;

		}
		DEBUG(dbgFileCreate, "Done allocating this level of fileheader.");
		
	}


	return TRUE;
}


/* - Original 
bool FileHeader::Allocate(PersistentBitmap *freeMap, int fileSize)
{
	numBytes = fileSize;
	numSectors = divRoundUp(fileSize, SectorSize);  // need numSectors to support total 'fileSize' bytes
	if (freeMap->NumClear() < numSectors)
		return FALSE; // not enough space

	for (int i = 0; i < numSectors; i++)
	{
		// Allocate datablock here! One sector at a time
		dataSectors[i] = freeMap->FindAndSet();
		// since we checked that there was enough free space,
		// we expect this to succeed
		ASSERT(dataSectors[i] >= 0); // sector number should be >=0
	}
	return TRUE;
}
*/
//----------------------------------------------------------------------
// FileHeader::Deallocate
// 	De-allocate all the space allocated for data blocks for this file.
//
//	"freeMap" is the bit map of free disk sectors
//----------------------------------------------------------------------



void FileHeader::Deallocate(PersistentBitmap *freeMap)
{
	if(numBytes > num_of_bytes_4level ){ // Four-level indirect (~100 MB)// single indirect (orignal nachos)
		for (int i = 0; i < divRoundUp(numSectors, NumDirect) ; i++) // 要 deallocate 幾次 (幾個level)
		{	//  在此：呼叫幾次遞迴！ 假設最高level每個entry都被填滿（最多就是30個）,表示要跑30次的遞迴，所以這層for-loop要跑30次！
						// subdir is sub-header! also has 30 datablocks (map to on-disk sector numbers)
			DEBUG(dbgFileRemove, "FileHDR: Deallocate(): > 4level");
			FileHeader* subdir = new FileHeader();
			// subdir->FetchFrom(dataSectors[dataSectors[i]]);			
			subdir->FetchFrom(dataSectors[i]);	
			// after fetchfrom [sector number of this sub-header], subhdr would have the
			// 'numBytes' and 'numSectors' of contents below this level.
			subdir->Deallocate(freeMap); // recursive.
		}
	}
	else if(numBytes > num_of_bytes_3level){ // tripple indirect (~3MB)
		for (int i = 0; i < divRoundUp(numSectors, NumDirect) ; i++)
		{
			DEBUG(dbgFileRemove, "FileHDR: Deallocate(): > 3level");
			FileHeader* subdir = new FileHeader();
			subdir->FetchFrom(dataSectors[i]);			
			subdir->Deallocate(freeMap); // recursive.
		}
	}
	else if(numBytes > num_of_bytes_2level){ // double indirect (~128KB)
		for (int i = 0; i < divRoundUp(numSectors, NumDirect) ; i++)
		{
			DEBUG(dbgFileRemove, "FileHDR: Deallocate(): > 2level");
			FileHeader* subdir = new FileHeader();
			subdir->FetchFrom(dataSectors[i]);			
			subdir->Deallocate(freeMap); // recursive.
		}
	}	
	else if(numBytes > num_of_bytes_1level){ // double indirect (~128KB)
		for (int i = 0; i < divRoundUp(numSectors, NumDirect) ; i++)
		{
			DEBUG(dbgFileRemove, "FileHDR: Deallocate(): > 1level");
			FileHeader* subdir = new FileHeader();
			subdir->FetchFrom(dataSectors[i]);			
			subdir->Deallocate(freeMap); // recursive.
		}
	}
	else{	// single indirect (orignal nachos)
			DEBUG(dbgFileRemove, "numBytes: "<<this->numBytes);
		for (int i = 0; i < numSectors; i++)
		{	
			DEBUG(dbgFileRemove,"Ready to test dataSectors["<<i<<"] : on-disk sector " <<dataSectors[i]);
			// std::cout<<"Ready to test dataSectors["<<i<<"] : on-disk sector " <<dataSectors[i]<<"\n";
			ASSERT(freeMap->Test((int)dataSectors[i])); // ought to be marked!
			DEBUG(dbgFileRemove,"Test dataSectors["<<i<<"] true!(marked)");
			// std::cout<<"Test dataSectors["<<i<<"] true!(marked)\n";
			freeMap->Clear((int)dataSectors[i]);
		}
	}
}

/* --- Original ---
void FileHeader::Deallocate(PersistentBitmap *freeMap)
{
	for (int i = 0; i < numSectors; i++)
	{
		ASSERT(freeMap->Test((int)dataSectors[i])); // ought to be marked!
		freeMap->Clear((int)dataSectors[i]);
	}
}
*/
//----------------------------------------------------------------------
// FileHeader::FetchFrom
// 	Fetch contents of file header from disk.
//
//	"sector" is the disk sector containing the file header
//----------------------------------------------------------------------

void FileHeader::FetchFrom(int sector)
{
	//
	// std::cout<<"FCB fetch from disk: sector "<<sector<<"\n";
	kernel->synchDisk->ReadSector(sector, (char *)this);
	// std::cout<<"After fecth from disk: hdr->FileLength: "<<this->numBytes<<"\n";
	/*
		MP4 Hint:
		After you add some in-core informations, you will need to rebuild the header's structure
	*/
}

//----------------------------------------------------------------------
// FileHeader::WriteBack
// 	Write the modified contents of the file header back to disk.
//
//	"sector" is the disk sector to contain the file header
//----------------------------------------------------------------------

void FileHeader::WriteBack(int sector)
{
	kernel->synchDisk->WriteSector(sector, (char *)this);

	/*
		MP4 Hint:
		After you add some in-core informations, you may not want to write all fields into disk.
		Use this instead:
		char buf[SectorSize];
		memcpy(buf + offset, &dataToBeWritten, sizeof(dataToBeWritten));
		...
	*/
}

//----------------------------------------------------------------------
// FileHeader::ByteToSector
// 	Return which disk sector is storing a particular byte within the file.
//      This is essentially a translation from a virtual address (the
//	offset in the file) to a physical address (the sector where the
//	data at the offset is stored).
//
//  divided by SectorSize since one sector per block, 
//  To find out "which dataBlock" this offset byte locates,
//	Just divided by #bytes/ per dataBlock.  = (#bytes * #sectors) / per Block
//	 =>  (128 bytes * 1 sector ) / per dataBlock
//	"offset" is the location within the file of the byte in question
//----------------------------------------------------------------------

// N-Level: N 層（個）FCB (Fileheaders )，第 n+1 層 才是真的 datablock

int FileHeader::ByteToSector(int offset)
{
	if(numBytes > num_of_bytes_4level){		// Level 1
		FileHeader* subhdr = new FileHeader;
		// subhdr->FetchFrom()
		int entry_number = divRoundDown(offset,num_of_bytes_4level); // RoundDown since 
		//dataSectors index start from 0! e.g. offset/num... = 1.7, 要去第二個entry繼續找，但因為第二個entry是datasectors[1],所以用roundown
		subhdr->FetchFrom(dataSectors[entry_number]);
		subhdr->ByteToSector(offset-num_of_bytes_4level*entry_number);

	}
	else if(numBytes > num_of_bytes_3level){ // Level 2
		FileHeader* subhdr = new FileHeader;
		// subhdr->FetchFrom()
		int entry_number = divRoundDown(offset,num_of_bytes_3level); // RoundDown since 
		//dataSectors index start from 0! e.g. offset/num... = 1.7, 要去第二個entry繼續找，但因為第二個entry是datasectors[1],所以用roundown
		subhdr->FetchFrom(dataSectors[entry_number]);
		subhdr->ByteToSector(offset-num_of_bytes_3level*entry_number);

	}
	else if(numBytes > num_of_bytes_2level){ // Level 3
		FileHeader* subhdr = new FileHeader;
		// subhdr->FetchFrom()
		int entry_number = divRoundDown(offset,num_of_bytes_2level); // RoundDown since 
		//dataSectors index start from 0! e.g. offset/num... = 1.7, 要去第二個entry繼續找，但因為第二個entry是datasectors[1],所以用roundown
		subhdr->FetchFrom(dataSectors[entry_number]);
		subhdr->ByteToSector(offset-num_of_bytes_2level*entry_number);
		
	}
	else if(numBytes > num_of_bytes_1level){ // Level 4
		FileHeader* subhdr = new FileHeader;
		// subhdr->FetchFrom()
		int entry_number = divRoundDown(offset,num_of_bytes_1level); // RoundDown since 
		//dataSectors index start from 0! e.g. offset/num... = 1.7, 要去第二個entry繼續找，但因為第二個entry是datasectors[1],所以用roundown
		subhdr->FetchFrom(dataSectors[entry_number]);
		subhdr->ByteToSector(offset-num_of_bytes_1level*entry_number);
		
	}
	else{	// Level 5: True DataBlock
		return (dataSectors[offset / SectorSize]);
	}

}

/*
int FileHeader::ByteToSector(int offset)
{
	return (dataSectors[offset / SectorSize]);
}
*/


//----------------------------------------------------------------------
// FileHeader::FileLength
// 	Return the number of bytes in the file.
//----------------------------------------------------------------------

int FileHeader::FileLength()
{
	return numBytes;
}

//----------------------------------------------------------------------
// FileHeader::Print
// 	Print the contents of the file header, and the contents of all
//	the data blocks pointed to by the file header.
//----------------------------------------------------------------------

void FileHeader::Print()
{
	int i, j, k;
	char *data = new char[SectorSize];

	printf("FileHeader contents.  File size: %d.  File blocks:\n", numBytes);
	for (i = 0; i < numSectors; i++)
		printf("%d ", dataSectors[i]);
	printf("\nFile contents:\n");


	if(numBytes > num_of_bytes_4level){		// Level 1
		for(int i=0; i< numSectors/NumDirect; i++){	// If FULL: run 30 recursive calls
			OpenFile* opfile = new OpenFile(dataSectors[i]);
			FileHeader* subhdr = opfile->getHdr();
			subhdr->Print();
		}
	}
	else if(numBytes > num_of_bytes_3level){ // Level 2
		for(int i=0; i< numSectors/NumDirect; i++){
			OpenFile* opfile = new OpenFile(dataSectors[i]);
			FileHeader* subhdr = opfile->getHdr();
			subhdr->Print();
		}
	}
	else if(numBytes > num_of_bytes_2level){ // Level 3
		for(int i=0; i< numSectors/NumDirect; i++){
			OpenFile* opfile = new OpenFile(dataSectors[i]);
			FileHeader* subhdr = opfile->getHdr();
			subhdr->Print();
		}
	}
	else if(numBytes > num_of_bytes_1level){ // Level 4
		for(int i=0; i< numSectors/NumDirect; i++){
			OpenFile* opfile = new OpenFile(dataSectors[i]);
			FileHeader* subhdr = opfile->getHdr();
			subhdr->Print();
		}
	}
	else{	// Level 5: True DataBlock
		for (i = k = 0; i < numSectors; i++)
		{
			kernel->synchDisk->ReadSector(dataSectors[i], data);
			for (j = 0; (j < SectorSize) && (k < numBytes); j++, k++)
			{
				if ('\040' <= data[j] && data[j] <= '\176') // isprint(data[j])
					printf("%c", data[j]);
				else
					printf("\\%x", (unsigned char)data[j]);
			}
			printf("\n");
		}
	}
	delete[] data;
	
}
