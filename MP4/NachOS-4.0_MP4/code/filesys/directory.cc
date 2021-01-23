// directory.cc
//	Routines to manage a directory of file names.
//
//	The directory is a table of fixed length entries; each
//	entry represents a single file, and contains the file name,
//	and the location of the file header on disk.  The fixed size
//	of each directory entry means that we have the restriction
//	of a fixed maximum size for file names.
//
//	The constructor initializes an empty directory of a certain size;
//	we use ReadFrom/WriteBack to fetch the contents of the directory
//	from disk, and to write back any modifications back to disk.
//
//	Also, this implementation has the restriction that the size
//	of the directory cannot expand.  In other words, once all the
//	entries in the directory are used, no more files can be created.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "filehdr.h"
#include "directory.h"
#include "filesys.h"
#include "kernel.h"
#include "main.h"

//----------------------------------------------------------------------
// Directory::Directory
// 	Initialize a directory; initially, the directory is completely
//	empty.  If the disk is being formatted, an empty directory
//	is all we need, but otherwise, we need to call FetchFrom in order
//	to initialize it from disk.
//
//	"size" is the number of entries in the directory
//----------------------------------------------------------------------

Directory::Directory(int size)
{
    table = new DirectoryEntry[size];

    // MP4 mod tag
    memset(table, 0, sizeof(DirectoryEntry) * size); // dummy operation to keep valgrind happy

    tableSize = size;
    for (int i = 0; i < tableSize; i++){
        table[i].inUse = FALSE;
        table[i].isDir = FALSE; // default: a file.
    }
}

//----------------------------------------------------------------------
// Directory::~Directory
// 	De-allocate directory data structure.
//----------------------------------------------------------------------

Directory::~Directory()
{
    delete[] table;
}

//----------------------------------------------------------------------
// Directory::FetchFrom
// 	Read the contents of the directory from disk.
//
//	"file" -- file containing the directory contents
//----------------------------------------------------------------------

void Directory::FetchFrom(OpenFile *file)
{
    (void)file->ReadAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::WriteBack
// 	Write any modifications to the directory back to disk
//
//	"file" -- file to contain the new directory contents
//----------------------------------------------------------------------

void Directory::WriteBack(OpenFile *file)
{
    (void)file->WriteAt((char *)table, tableSize * sizeof(DirectoryEntry), 0);
}

//----------------------------------------------------------------------
// Directory::FindIndex
//  Find File's Index in directory->table!
//  
// 	Look up file name in directory, and return its location in the table of
//	directory entries.  Return -1 if the name isn't in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int Directory::FindIndex(char *name)
{
    for (int i = 0; i < tableSize; i++)
        if (table[i].inUse && !strncmp(table[i].name, name, FileNameMaxLen))
            return i;
    return -1; // name not in directory
}

//----------------------------------------------------------------------
// Directory::Find
// 	Look up file name in directory, and return the disk sector number
//	where the file's header is stored. Return -1 if the name isn't
//	in the directory.
//
//	"name" -- the file name to look up
//----------------------------------------------------------------------

int Directory::Find(char *name)
{
    int i = FindIndex(name); // First find the entry of this file("name") in this Directory

    if (i != -1)
        return table[i].sector; // Then, return its sector (contain this file's header)
    return -1;
}

//----------------------------------------------------------------------
// Directory::Add
// 	Add a file into the directory.  Return TRUE if successful;
//	return FALSE if the file name is already in the directory, or if
//	the directory is completely full, and has no more space for
//	additional file names.
//
//	"name" -- the name of the file being added
//	"newSector" -- the disk sector containing the added file's header

// 
// Data Structure: DirectoryEntry: 
//    bool inUse;                    // Is this directory entry in use?
//    int sector;                    // Location on disk to find the
                                     //   FileHeader for this file
//    char name[FileNameMaxLen + 1]; // Text name for file, with +1 for
         
//----------------------------------------------------------------------

/*  MP4 */
bool Directory::Add(char *name, int newSector, bool isDir)
{
    if (FindIndex(name) != -1)
        return FALSE;

    // Find free entry in the directory table, set inUse to TRUE and string copy!

    for (int i = 0; i < tableSize; i++)
        if (!table[i].inUse)
        {
            table[i].inUse = TRUE;
            strncpy(table[i].name, name, FileNameMaxLen); // string copy: copy 'name' to table[i].name, no more than 'FileNameMaxLen' characters
            table[i].sector = newSector;
            table[i].isDir = isDir; /* ---------- MP4 --------- */
            return TRUE;
        }
    return FALSE; // no space.  Fix when we have extensible files.
}



bool Directory::Add(char *name, int newSector)
{
    if (FindIndex(name) != -1)  // Already have this file; We CANNOT have two file with the same name in one(the same) directory.
        return FALSE;

    // Find free entry in the directory table, set inUse to TRUE and string copy!

    for (int i = 0; i < tableSize; i++)
        if (!table[i].inUse)
        {
            table[i].inUse = TRUE;
            strncpy(table[i].name, name, FileNameMaxLen); // string copy: copy 'name' to table[i].name, no more than 'FileNameMaxLen' characters
            table[i].sector = newSector;
            return TRUE;
        }
    return FALSE; // no space.  Fix when we have extensible files.
}

//----------------------------------------------------------------------
// Directory::Remove
// 	Remove a file name from the directory.  Return TRUE if successful;
//	return FALSE if the file isn't in the directory.
//
//	"name" -- the file name to be removed
//----------------------------------------------------------------------

bool Directory::Remove(char *name)
{
    int i = FindIndex(name);

    if (i == -1)
        return FALSE; // name not in directory // No such file in the directory
    table[i].inUse = FALSE;
    return TRUE;
}

void Directory::RecursiveRemove(char *name)
{
    bool empty = TRUE;
    // First enter current dir (e.g /t0/bb)
    std::cout<<"Current in : "<<name<<"\n";
    for(int i=0; i<tableSize; i++){
        if(table[i].inUse){
            empty = FALSE;
            /* KEY ! If have next dir: should record current dir, when return, use
                cur->dir->Remove(table[i].name);
            */

            if(table[i].isDir){
                // kernel->fileSystem->Remove(path_under_this_dir, TRUE);
                std::cout<<"entry "<<i<<" is Dir. name "<<table[i].name<<", sector "<<table[i].sector<<""<<"\n";
                Directory* next_dir = new Directory(NumDirEntries);
                OpenFile* next_dir_file = new OpenFile(table[i].sector);    // (FCB of the next_directory file. ) =>Open from FCB.
                next_dir->FetchFrom(next_dir_file);

                PersistentBitmap* freeMap = new PersistentBitmap(kernel->fileSystem->getFreeMapFile() , NumSectors);
                FileHeader* next_dirfile_tobeRemove = new FileHeader;
                next_dirfile_tobeRemove->FetchFrom(table[i].sector);

                // DEBUG(dbgFileRemove, "Clear bitmap of dataSectors & HeaderSector of dir file "<<table[i].name);
                // DEBUG(dbgFileRemove, "next_dir: "<<table[i].name<<", on-disk FCB sector: "<<table[i].sector);              
              
                // First Remove dataSectors.
                DEBUG(dbgFileRemove, "First Remove dataSectors of dir file "<<table[i].name);
                            
                next_dirfile_tobeRemove->Deallocate(freeMap); // remove data blocks
                // std::cout<<"Now, want to clear the header-bit (sector number ) on bitmap\n";
                DEBUG(dbgFileRemove, "Second Remove the FileHeader. "<<table[i].name<<", on-disk FCB sector: "<<table[i].sector);  
                DEBUG(dbgFileRemove, "freeMap->Clear("<<table[i].sector<<")");
                // Second Remove the FileHeader.
                freeMap->Clear(table[i].sector); 
                

                // Third, remove its directory entries. 
                DEBUG(dbgFileRemove,  "Third, remove its directory entries of dir /"<<table[i].name<<", recursive call.");                  
                
                next_dir->RecursiveRemove(table[i].name);
                
                /*  Write back the result of next_dir to its FCB. */
                DEBUG(dbgFileRemove,  "Write back the result of next_dir /"<<table[i].name<<" to its FCB.");       
                next_dir->WriteBack(next_dir_file);
                /* 
                    Remove next_dir 's FCB & dataSectors.
                */                
                DEBUG(dbgFileRemove,  "From this dir, remove next_dir /"<<table[i].name);     
                this->Remove(table[i].name); /* KEY!!!! */
                freeMap->WriteBack( kernel->fileSystem->getFreeMapFile() );
    
                delete next_dir_file;
                delete next_dir;
                delete next_dirfile_tobeRemove;
                delete freeMap;

            }
            else{ // is a fil.e
                std::cout<<"entry "<<i<<" is a File. name "<<table[i].name<<", sector "<<table[i].sector<<""<<"\n";

                /* Order:  Deallocate dataSectors -> Deallocate Hdr -> Remove directory Entry !!!　*/
                PersistentBitmap* freeMap = new PersistentBitmap(kernel->fileSystem->getFreeMapFile() , NumSectors);
                FileHeader* fileHdr_of_file_tobeRemove = new FileHeader;
                fileHdr_of_file_tobeRemove->FetchFrom(table[i].sector);
                DEBUG(dbgFile, "First deallocate dataSectors of file "<<table[i].name);
                fileHdr_of_file_tobeRemove->Deallocate(freeMap); // remove data blocks

                DEBUG(dbgFile, "Second, Clear bitmap of File Header (FCB)Sector of file "<<table[i].name);
                // std::cout<<"Now, want to clear the header-bit (sector number ) on bitmap\n";
                freeMap->Clear(table[i].sector); 

                DEBUG(dbgFile, "Third, Remove this file: "<<table[i].name<<" from its directory entry.");
                if(!Remove(table[i].name)){
                    std::cout<<"Err in removing file: "<<name<<"\n";
                }
                DEBUG(dbgFileRemove,  "Write back the result to freeMap.");       
                freeMap->WriteBack( kernel->fileSystem->getFreeMapFile() );
            }
        }


    }    

    if(empty) std::cout<<"This directory is empty.\n";
    // return if empty dir.
}





//----------------------------------------------------------------------
// Directory::List
// 	List all the file names in the directory.
//----------------------------------------------------------------------
// void Directory::List(int depth, bool Recursive)
// {

//     for(int i=0; i< tableSize; i++){
//         if(table[i].inUse){
//             // if(depth==0) printf("[D] %s\n", table[i].name);
//             // else {
//             //     printf("  ");
//                 for(int j=0; j<depth; j++)printf("  ");
//                 if(table[i].isDir)  printf("[D] %s\n", table[i].name);
//                 else                printf("[F] %s\n", table[i].name);
//             // }
//             if(Recursive){  // recursively print 
//                 if(table[i].isDir){
//                     Directory* sub_dir = new Directory(NumDirect);
//                     OpenFile* sub_dir_file = new OpenFile(table[i].sector); // return its open file pointer(points to its system-wide entry(stores FCB)).
//                     sub_dir->FetchFrom(sub_dir_file); // Note: For directory::FetchFrom(OpenFile* file); while FileHeader::FetchFrom(int sectorNumber);
//                     sub_dir->List(depth+1, TRUE);

//                     delete sub_dir;
//                     delete sub_dir_file;    // Free memory when return from recursion.
//                 }
//                 // else, a file, do not need to print.
//             }
//         }
//         else{
//             printf("dir entry %d not used.\n", i);
//         }
//     }
// }


//----------------------------------------------------------------------
void Directory::RecursiveList(int depth)
{
    // printf("Run Recursive list!\n");
    bool empty = TRUE;
    Directory *subdir = new Directory(NumDirEntries);
    OpenFile *subdir_openfile;

    DEBUG(dbgFile,"Directory::RecursiveList in depth:　"<<depth);    

    for(int i=0; i< tableSize; i++){
        if(table[i].inUse){
            std::cout<<"\n"; // New line in every (dir)table-item in each recursive call.
            empty = FALSE;
        // std::cout<<"depth: "<<depth;
            for(int k=0; k<depth; k++) std::cout<<"  ";

            if(table[i].isDir){
                printf("[D] %s", table[i].name);
                subdir_openfile = new OpenFile(table[i].sector);
                DEBUG(dbgFile, "table["<<i<<"] is dir, current_dir->FetchFrom( OpenFile* ("<<table[i].sector<<") )");
                subdir->FetchFrom(subdir_openfile); // KEY!　This openfile.sector should be clear when removing!!
                subdir->RecursiveList(depth+1);
                
            }
            else{
                printf("[F] %s", table[i].name);
            }
        }
        else{
            ///////// Error here, after -f -d f.
            ///// At dir entry 3 not use.(i==3 error.)
            // std::cout<<"\n";
            // for(int k=0; k<depth; k++) std::cout<<"  ";
            // printf("dir entry %d not used.", i);
        }
    }

    if(empty){
         for(int k=0; k<depth; k++) std::cout<<" ";
         std::cout<<"(Empty directory)";
    }
    depth--; //remember to bring back the depth!
}



void Directory::List()
{
    bool empty = TRUE;
    // printf("Directoyr::List() tableSize: %d\n",tableSize);
    for (int i = 0; i < tableSize; i++){
        // std::cout<<"Entry "<<i<<"inUse: "<<table[i].inUse<<", name: "<<table[i].name<<", sector: "<<table[i].sector<<"\n";
        if (table[i].inUse==TRUE){
            empty = FALSE;
            // printf("%s, %d\n",table[i].name, table[i].isDir);
            if(table[i].isDir==TRUE)
                std::cout<<"[D] "<<table[i].name<<std::endl;
            else std::cout<<"[F] "<<table[i].name<<std::endl;
        }
        // else std::cout<<"Entry "<<i<<" not used.\n";
    }

    if(empty) std::cout<<"The directory is empty\n";
}

//----------------------------------------------------------------------
// Directory::Print
// 	List all the file names in the directory, their FileHeader locations,
//	and the contents of each file.  For debugging.
//----------------------------------------------------------------------

void Directory::Print()
{
    FileHeader *hdr = new FileHeader;

    printf("Directory contents:\n");
    for (int i = 0; i < tableSize; i++)
        if (table[i].inUse)
        {
            printf("Name: %s, Sector: %d\n", table[i].name, table[i].sector);
            hdr->FetchFrom(table[i].sector);
            hdr->Print();
        }
    printf("\n");
    delete hdr;
}
