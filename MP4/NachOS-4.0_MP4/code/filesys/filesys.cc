// filesys.cc
//	Routines to manage the overall operation of the file system.
//	Implements routines to map from textual file names to files.
//
//	Each file in the file system has:
//	   A file header, stored in a sector on disk
//		(the size of the file header data structure is arranged
//		to be precisely the size of 1 disk sector)
//	   A number of data blocks
//	   An entry in the file system directory
//
// 	The file system consists of several data structures:
//	   A bitmap of free disk sectors (cf. bitmap.h)
//	   A directory of file names and file headers
//
//      Both the bitmap and the directory are represented as normal
//	files.  Their file headers are located in specific sectors
//	(sector 0 and sector 1), so that the file system can find them
//	on bootup.
//
//	The file system assumes that the bitmap and directory files are
//	kept "open" continuously while Nachos is running.
//
//	For those operations (such as Create, Remove) that modify the
//	directory and/or bitmap, if the operation succeeds, the changes
//	are written immediately back to disk (the two files are kept
//	open during all this time).  If the operation fails, and we have
//	modified part of the directory and/or bitmap, we simply discard
//	the changed version, without writing it back to disk.
//
// 	Our implementation at this point has the following restrictions:
//
//	   1. there is no synchronization for concurrent accesses
//	   2. files have a fixed size, set when the file is created
//	   3. files cannot be bigger than about 3KB in size
//	   4. there is no hierarchical directory structure, and only a limited
//	      number of files can be added to the system
//	   5. there is no attempt to make the system robust to failures
//	    (if Nachos exits in the middle of an operation that modifies
//	    the file system, it may corrupt the disk)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.
#ifndef FILESYS_STUB

#include "copyright.h"
#include "debug.h"
#include "disk.h"
#include "pbitmap.h"
#include "directory.h"
#include "filehdr.h"
#include "filesys.h"

// Sectors containing the file headers for the bitmap of free sectors,
// and the directory of files.  These file headers are placed in well-known
// sectors, so that they can be located on boot-up.
#define FreeMapSector 0
#define DirectorySector 1

// FreeMap := bitmap
// Directory := root directory

// Initial file sizes for the bitmap and directory; until the file system
// supports extensible files, the directory size sets the maximum number
// of files that can be loaded onto the disk.
#define FreeMapFileSize (NumSectors / BitsInByte) // Need total NumSectors bits (one bit per entry), and 
// #define NumDirEntries 10                          // divided by 8 since convert bits => bytes(需要多少bytes來存 bitmap 這個file！)

#define DirectoryFileSize (sizeof(DirectoryEntry) * NumDirEntries)


//----------------------------------------------------------------------
// FileSystem::FileSystem
// 	Initialize the file system.  If format = TRUE, the disk has
//	nothing on it, and we need to initialize the disk to contain
//	an empty directory, and a bitmap of free sectors (with almost but
//	not all of the sectors marked as free).

// MP4: should clear everything on Disk!! Make all sector to 0.


//
//	If format = FALSE, we just have to open the files
//	representing the bitmap and the directory.
//
//	"format" -- should we initialize the disk?
//----------------------------------------------------------------------

FileSystem::FileSystem(bool format)
{
    DEBUG(dbgFile, "Initializing the file system.");
    if (format)
    {
        DEBUG(dbgFile,"Total Num of Sectors: "<<NumSectors);
        DEBUG(dbgFile,"Need FreeMap Size: "<<FreeMapFileSize);
        DEBUG(dbgFile,"Need root Dir Size "<<DirectoryFileSize);

        PersistentBitmap *freeMap = new PersistentBitmap(NumSectors);
        Directory *directory = new Directory(NumDirEntries);
        FileHeader *mapHdr = new FileHeader;
        FileHeader *dirHdr = new FileHeader;

        DEBUG(dbgFile, "Formatting the file system.");

        // First, allocate space for FileHeaders for the directory and bitmap
        // (make sure no one else grabs these!)
        // Mark -> 設成1表示此sector已被佔用！
        freeMap->Mark(FreeMapSector);
        freeMap->Mark(DirectorySector);

        // Second, allocate space for the data blocks containing the contents
        // of the directory and bitmap files.  There better be enough space!
        DEBUG(dbgFile,"Formatting FS: allocating freemap dataSectors... ");
        ASSERT(mapHdr->Allocate(freeMap, FreeMapFileSize)); // Allocate bitmap 's dataSectors on Disk

        DEBUG(dbgFile,"Formatting FS: allocating 'root directory' dataSectors... ");
        ASSERT(dirHdr->Allocate(freeMap, DirectoryFileSize));

        // Flush the bitmap and directory FileHeaders back to disk
        // We need to do this before we can "Open" the file, since 'open'
        // reads the file header off of disk (and currently the disk has garbage
        // on it!).

        DEBUG(dbgFile, "Writing headers back to disk.");

        // if(mapHdr->FileLength()>SectorSize){

        // }

        mapHdr->WriteBack(FreeMapSector);
        dirHdr->WriteBack(DirectorySector);

        // OK to open the bitmap and directory files now
        // The file system operations assume these two files are left open
        // while Nachos is running.

        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);

        // Once we have the files "open", we can write the initial version
        // of each file back to disk.  The directory at this point is completely
        // empty; but the bitmap has been changed to reflect the fact that
        // sectors on the disk have been allocated for the file headers and
        // to hold the file data for the directory and bitmap.

        DEBUG(dbgFile, "Writing bitmap and directory back to disk.");
        freeMap->WriteBack(freeMapFile); // flush changes to disk
        directory->WriteBack(directoryFile);

        if (debug->IsEnabled('f'))
        {
            freeMap->Print();  //Print bitmap's on-disk DataSectors.
            directory->Print();
        }


        // std::cout<<"Checking by re-opening freeMap:\n";
        // // freeMapFile = new OpenFile(FreeMapSector);
        // // directoryFile = new OpenFile(DirectorySector);
        // freeMap = new PersistentBitmap(freeMapFile, NumSectors);
        // freeMap->Print();

        // std::cout<<"Done checking freeMapFile\n";

        delete freeMap;
        delete directory;
        delete mapHdr;
        delete dirHdr;
    }
    else
    {
        // if we are not formatting the disk, just open the files representing
        // the bitmap and directory; these are left open while Nachos is running
        freeMapFile = new OpenFile(FreeMapSector);
        directoryFile = new OpenFile(DirectorySector);
    }
}

//----------------------------------------------------------------------
// MP4 mod tag
// FileSystem::~FileSystem
//----------------------------------------------------------------------
FileSystem::~FileSystem()
{

    // Delete what?
    //////// MP4 ERROR!!! NEVER delete opfile!!!! ///////// Or you will get lots of memory map error.
    delete freeMapFile;
    delete directoryFile;
}

//----------------------------------------------------------------------
// FileSystem::Create
// 	Create a file in the Nachos file system (similar to UNIX create).
//	Since we can't increase the size of files dynamically, we have
//	to give Create the initial size of the file.
//
//	The steps to create a file are:
//	  1. Make sure the file doesn't already exist
//    2. Allocate a sector for the file header
// 	  3. Allocate space on disk for the data blocks for the file
//	  4. Add the name to the directory
//	  5. Store the new file header on disk
//	  6. Flush the changes to the bitmap and the directory back to disk
//
//	Return TRUE if everything goes ok, otherwise, return FALSE.
//
// 	Create fails if:
//   		file is already in directory
//	 	no free space for file header
//	 	no free entry for file in directory
//	 	no free space for data blocks for the file
//
// 	Note that this implementation assumes there is no concurrent access
//	to the file system!
//
//	"name" -- name of file to be created
//	"initialSize" -- size of file to be created
//----------------------------------------------------------------------




// For file.
bool FileSystem::Create(char *name, int initialSize)
{
    Directory *directory;
    PersistentBitmap *freeMap;
    FileHeader *hdr;
    int sector;
    bool success;

    DEBUG(dbgFile, "Creating file " << name << " size " << initialSize);

    directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile); // directoryFiile 是 openfile,
                                         // 讓新建的 directory（視為一個暫存的buffer）去讀取file system目前真正的root directory的內容！
    
    #ifdef DEBUG_CREATE_FILE
    std::cout<<"Init root dir list:\n";
    directory->List();
    #endif

    OpenFile *file_temp = directoryFile;
    OpenFile *last_level_dir_file = file_temp;
    bool should_roll_back_to_last_level_dir = TRUE;
    char *token = strtok(name, "/");
    char *prev_token = token;    
    // int count= 0;



    while( token!=NULL){
            
        #ifdef DEBUG_CREATE_FILE
        std::cout<<"\n\n Iteration in while-loop"<<count++<< "\n";
        std::cout<<"Create(), token: "<<token<<std::endl;
        #endif
        // std::cout<<"Sector of dir->Find(token): "<< directory->Find(token)<<"\n";
        sector = directory->Find(token);
        if(sector == -1){
            should_roll_back_to_last_level_dir = FALSE;
        // if( (sector = directory->Find(token)==-1) ){
            DEBUG(dbgFileCreate, "Haven't have "<<token<<" before! Break \n");
            break;
        } // Haven't have this token before.
        
        last_level_dir_file = file_temp;   // Last level directroy. (KEY here.)
        
        file_temp = new OpenFile(sector);
        directory->FetchFrom(file_temp);
        
        #ifdef DEBUG_CREATE_FILE
        std::cout<<"fetch from sector "<<sector<<", dir list\n";
        #endif
        // directory->List();
        prev_token = token; //update prev_token
        token = strtok(NULL, "/"); // update toke to the next token after "/"
    }

    if(token==NULL) token = prev_token; // if traverse to the end.

    if(should_roll_back_to_last_level_dir) directory->FetchFrom(last_level_dir_file);

    // std::cout<<"Create() File: token "<<token<<"\n";
    // std::cout<<"In Create(): cur directory layout:\n";
    // directory->List();
    // #ifdef DEBUG_CREATE_FILE
    DEBUG(dbgFileCreate, "Create(): Add file \'"<<token<<"\' to directory.");
    // #endif

    if (directory->Find(token) != -1){
        std::cout<<"Err contruct the same File \'"<<token<<"\' in cur dir. Break\n";
        success = FALSE; // file is already in directory
    }
    else // File hasn't been created! CAN create this file
    {

        // std::cout<<"Checking by re-opening freeMap:\n";
        // freeMapFile = new OpenFile(FreeMapSector);
        // directoryFile = new OpenFile(DirectorySector);
        // freeMap = new PersistentBitmap(freeMapFile, NumSectors);
        // freeMap->Print();

        // std::cout<<"Done checking freeMapFile\n";

        // freeMapFile = new OpenFile(FreeMapSector);
        freeMap = new PersistentBitmap(freeMapFile, NumSectors);// 新建的 freemap（視為一個暫存的buffer）去讀取file system目前真正 bitmap 的內容！
         // step 2~4
         // 先找第一個sector, 存fileheader用
        sector = freeMap->FindAndSet(); // find a sector to hold the file header
        if (sector == -1){
            std::cout<<"Err: No enough free sectors for FileHeader of file\'"<<token<<"\' in the whole disk. Break\n";
            success = FALSE; // no free block for file header
        }
            // 再看 root directory還有沒有entry可以存這個file
        // else if (!directory->Add(token, sector))
        
        else if (!directory->Add(token, sector,FALSE)){
            std::cout<<"Err: No free entry for file \'"<<token<<"\' in cur dir. Break\n";
            success = FALSE; // no space in directory
        }
        else
        {
           
            hdr = new FileHeader;
            // 最後看 用header來allocate 'initialsize' 給這個file, disk空間還夠不夠！（用當前的bitmap:=freemap)
            if (!hdr->Allocate(freeMap, initialSize)){
                std::cout<<"Err: No enough free sectors for file\'"<<token<<"\' in the whole disk. Break\n";
                success = FALSE; // no space on disk for data
            }
            else
            {   
                //step 5.& 6.
                success = TRUE;
                DEBUG(dbgFileCreate, "Successfully create file: "<<token);
                // everthing worked, flush all changes back to disk
                // 記得把這個header 寫回 存header該有的sector! ('sector'是上面第212行bitmap幫我們找到的參數)
                hdr->WriteBack(sector);
                directory->WriteBack(file_temp);
                freeMap->WriteBack(freeMapFile);

                /* --------------------
                    這邊所有 operation 每次都先新建 bitmap, directory, 和 header
                    的目的是：可以先對這些 register或memory content操作，但只有
                    最後確認 '這樣的allocation是可行的'時，才會將這三個東西正式寫回到disk
                    若中途發現 'root directory'沒有entry了，或是 'bitmap'中的free disk sectors不夠
                    分配給你指定的 initialsize, 直接 delete freeMap / directory / hdr
                    反正沒有 write back to disk, 指的是真正 file system 的
                    bitmap (存在 sector 0)
                    root dir(存在sector 1) 這兩個sector內的content沒有被改寫掉，
                    實質上就沒有影響到 file system的內容！

                    這邊 disk 只是一個file, nachos 的 disk simulator
                    可以到 machine/ disk.h 和 disk.cc 看
                
                ---------------------- */


            }
            delete hdr;
        }
        delete freeMap;
    }
    delete directory;

    return success;
}


// For sub-dir
bool FileSystem::CreateDir(char *name)
{
    Directory *directory;
    PersistentBitmap *freeMap;
    FileHeader *hdr;
    int sector;
    bool success;

    DEBUG(dbgFile, "Creating dir file " << name << " size " << DirectoryFileSize);

    directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile); // directoryFiile 是 openfile,
                                         // 讓新建的 directory（視為一個暫存的buffer）去讀取file system目前真正的root directory的內容！
    
    // Start searching from root dir...
    OpenFile *file_temp; //  KEY! Should initialize file_temp!
    file_temp = directoryFile;
    char *token = strtok(name, "/");
    char *prev_token = token;    
    OpenFile *last_level_dir_file = file_temp;
    bool should_roll_back_to_last_dir = TRUE;
    
    while( token!=NULL){
        if(  (sector = directory->Find(token)) ==-1 ){
           DEBUG(dbgFileCreate, "First create dir "<<token<<" in cur dir. Break");
            should_roll_back_to_last_dir = FALSE;
            break;
        }
        // Else: should traverse to the last layer!
        std::cout<<"Existed dir: "<<token<<" its FCB at sector "<<sector<<"\n";
        last_level_dir_file = file_temp;        // update last level file.
        
        file_temp = new OpenFile(sector);
        directory->FetchFrom(file_temp);
        prev_token = token; //update prev_token

        
        token = strtok(NULL, "/"); // update toke to the next token after "/"
        // std::cout<<"prev_token "<<prev_token<<", next token"<<token<<"\n";
        DEBUG(dbgdir, "prev_token "<<prev_token);
    }

    if(token==NULL){
        
        token = prev_token;
    }

    // KEY !! 
    if(should_roll_back_to_last_dir) directory->FetchFrom(last_level_dir_file);

    // in case you -mkdir the same path, token == NULL and directory 
    // directory->FetchFrom(file_temp);
    // else: token is t0 or somewhat

    #ifdef DEBUG_CREATE_DIR
     std::cout<<"CreateDir(): Add file \'"<<token<<"\' to directory.\n";

    // std::cout<<"Creating dir: "<<token;
    std::cout<<"Cur dir (should be the upper level dir of the current dir being created) layout: \n"; 
    directory->List();
    #endif


    if (directory->Find(token) != -1){
        std::cout<<"Err contruct the same dir \'"<<token<<"\' in cur dir. Break\n";
        DEBUG(dbgFile, "ERROR!!!　Directory with the same name already exist in current dir. Abort!");
        success = FALSE; // Dir with the same name already exist.
    }
    // if (directory->Find(name) != -1)
    //     success = FALSE; // file is already in directory
    else // File hasn't been created! CAN create this file
    {


        // std::cout<<"Checking by re-opening freeMap:\n";
        // freeMapFile = new OpenFile(FreeMapSector);
        // directoryFile = new OpenFile(DirectorySector);
        // freeMap = new PersistentBitmap(freeMapFile, NumSectors);
        // freeMap->Print();

        // std::cout<<"Done checking freeMapFile\n";


        freeMap = new PersistentBitmap(freeMapFile, NumSectors);// 新建的 freemap（視為一個暫存的buffer）去讀取file system目前真正 bitmap 的內容！
         // step 2~4
         // 先找第一個sector, 存fileheader用
    
        // Debug for freeMap!
        if (debug->IsEnabled('f'))
        {
            freeMap->Print();
            directory->Print();
        } 


        sector = freeMap->FindAndSet(); // find a sector to hold the file header
        // #ifdef DEBUG_CREATE_DIR    
        DEBUG(dbgFileCreate, "Sector number for FCB of 'directory' "<<token<<" is "<<sector);
        // #endif

        if (sector == -1){
            DEBUG(dbgFile, "No free bit from bitmap. => No available free sector for your directory. Abort!");
            success = FALSE; // no free block for file header
        }
            // 再看 root directory還有沒有entry可以存這個file
        // else if (!directory->Add(token, sector))
        else if (!directory->Add(token, sector, TRUE)){
            DEBUG(dbgFile, "No free entry for your directory in current directory(Already 64 entries! FULL). Abort!");
            success = FALSE; // no space in directory
        }
        else
        {
           
            hdr = new FileHeader;
            DEBUG(dbgFileCreate, "Allocating file header for [dir] file: "<<token<<", FCB is at sector "<<sector);
            // 最後看 用header來allocate 'initialsize' 給這個file, disk空間還夠不夠！（用當前的bitmap:=freemap)
            if (!hdr->Allocate(freeMap, DirectoryFileSize)){
                DEBUG(dbgFile, "No enough [number of sectors] on disk for your directory! Abort!");
                success = FALSE; // no space on disk for data
            }
            else
            {   
                //step 5.& 6.
                success = TRUE;
                #ifdef DEBUG_CREATE_DIR
                std::cout<<"Success in Creating [dir] file " << name << ", its FCB(inode) at sector "<<sector<<". size " << DirectoryFileSize<<std::endl;
                // everthing worked, flush all changes back to disk
                // 記得把這個header 寫回 存header該有的sector! ('sector'是上面第212行bitmap幫我們找到的參數)
                std::cout<<"Check dir update: \n";
                directory->List();
                std::cout<<"Writing back FCB(hdr) to disk sector "<<sector<<".. (update its FCB, current directory, and FS's bitmap (freeMap)\n";
                #endif
                
                hdr->WriteBack(sector);
                directory->WriteBack(file_temp);
                freeMap->WriteBack(freeMapFile);

                /* --------------------
                    這邊所有 operation 每次都先新建 bitmap, directory, 和 header
                    的目的是：可以先對這些 register或memory content操作，但只有
                    最後確認 '這樣的allocation是可行的'時，才會將這三個東西正式寫回到disk
                    若中途發現 'root directory'沒有entry了，或是 'bitmap'中的free disk sectors不夠
                    分配給你指定的 initialsize, 直接 delete freeMap / directory / hdr
                    反正沒有 write back to disk, 指的是真正 file system 的
                    bitmap (存在 sector 0)
                    root dir(存在sector 1) 這兩個sector內的content沒有被改寫掉，
                    實質上就沒有影響到 file system的內容！

                    這邊 disk 只是一個file, nachos 的 disk simulator
                    可以到 machine/ disk.h 和 disk.cc 看
                
                ---------------------- */


            }
            delete hdr;
        }
        delete freeMap;
    }
    delete directory;
    return success;
}


// Original Create

// bool FileSystem::Create(char *name, int initialSize)
// {
//     Directory *directory;
//     PersistentBitmap *freeMap;
//     FileHeader *hdr;
//     int sector;
//     bool success;

//     DEBUG(dbgFile, "Creating file " << name << " size " << initialSize);

//     directory = new Directory(NumDirEntries);
//     directory->FetchFrom(directoryFile); // directoryFiile 是 openfile,
//                                          // 讓新建的 directory（視為一個暫存的buffer）去讀取file system目前真正的root directory的內容！
//     if (directory->Find(name) != -1)
//         success = FALSE; // file is already in directory
//     else // File hasn't been created! CAN create this file
//     {
//         freeMap = new PersistentBitmap(freeMapFile, NumSectors);// 新建的 freemap（視為一個暫存的buffer）去讀取file system目前真正 bitmap 的內容！
//          // step 2~4
//          // 先找第一個sector, 存fileheader用
//         sector = freeMap->FindAndSet(); // find a sector to hold the file header
//         if (sector == -1)
//             success = FALSE; // no free block for file header
//             // 再看 root directory還有沒有entry可以存這個file
//         else if (!directory->Add(name, sector))
//             success = FALSE; // no space in directory
//         else
//         {
           
//             hdr = new FileHeader;
//             // 最後看 用header來allocate 'initialsize' 給這個file, disk空間還夠不夠！（用當前的bitmap:=freemap)
//             // 逐一分配 hdr->dataSectors[i] 到 on-disk 的 free sectorNumber!
//             if (!hdr->Allocate(freeMap, initialSize))
//                 success = FALSE; // no space on disk for data
//             else
//             {   
//                 //step 5.& 6.
//                 success = TRUE;
//                 // everthing worked, flush all changes back to disk
//                 // 記得把這個header 寫回 存header該有的sector! ('sector'是上面第212行bitmap幫我們找到的參數)
//                 hdr->WriteBack(sector); // 把整個hdr寫回sector!
//                 directory->WriteBack(directoryFile); // 把 dir的 table寫回 directoryFile的多個 on-disk sectors!
//                 freeMap->WriteBack(freeMapFile); // 把freeMap的map寫回 freeMapFile的 多個on-disk sectors!

//                 /* --------------------
//                     這邊所有 operation 每次都先新建 bitmap, directory, 和 header
//                     的目的是：可以先對這些 register或memory content操作，但只有
//                     最後確認 '這樣的allocation是可行的'時，才會將這三個東西正式寫回到disk
//                     若中途發現 'root directory'沒有entry了，或是 'bitmap'中的free disk sectors不夠
//                     分配給你指定的 initialsize, 直接 delete freeMap / directory / hdr
//                     反正沒有 write back to disk, 指的是真正 file system 的
//                     bitmap (存在 sector 0)
//                     root dir(存在sector 1) 這兩個sector內的content沒有被改寫掉，
//                     實質上就沒有影響到 file system的內容！

//                     這邊 disk 只是一個file, nachos 的 disk simulator
//                     可以到 machine/ disk.h 和 disk.cc 看
                
//                 ---------------------- */


//             }
//             delete hdr;
//         }
//         delete freeMap;
//     }
//     delete directory;
//     return success;
// }



//----------------------------------------------------------------------
// FileSystem::Open
// 	Open a file for reading and writing.
//	To open a file:
//	  Find the location of the file's header, using the directory
//	  Bring the header into memory
//
//	"name" -- the text name of the file to be opened
//----------------------------------------------------------------------


/* -----------Original -----------------*/
/*
OpenFile * FileSystem::Open(char *name)
{
    Directory *directory = new Directory(NumDirEntries);
    OpenFile *openFile = NULL;
    int sector;

    DEBUG(dbgFile, "Opening file" << name);
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name); // return the sector contain this file's header
    if (sector >= 0)
        openFile = new OpenFile(sector); // name was found in directory
    delete directory;
    opfile = openFile; 
    return openFile; // return NULL if not found
}
*/

OpenFile * FileSystem::Open(char *name)
{
    Directory *directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);
    
    OpenFile *openFile = NULL;
    int sector;
    
    OpenFile *file_temp = directoryFile;
    
    // std::cout<<name<<std::endl;
    char *token = strtok(name, "/");
    char *prev_token = token;    
    

    #ifdef DEBUG_OPENFILE   
    std::cout<<"In FileSystem Open():\n";
    std::cout<<"init dir: \n";
    directory->List();
    #endif

    while( token!=NULL){
        #ifdef DEBUG_TOKEN
        std::cout<<"token: "<<token<<" ";
        #endif
        if( (sector = directory->Find(token) ) ==-1  || directory->IsDir(token) == FALSE  ) {
            if(directory->IsDir(token) == FALSE){
                DEBUG(dbgFileCreate, token<<" is a file.");
                break;
            }else{
                std::cout<<token<<" is a directory.\n";
                break;
            }
        }
        #ifdef DEBUG_TOKEN
        std::cout<<"token \'"<<token<<"\' -> sector: "<<sector<<std::endl;
        #endif
        file_temp = new OpenFile(sector);   // FetchFrom subdirectory_File.
        directory->FetchFrom(file_temp);    // read from disk_sector_13
        prev_token = token; //update prev_token
        token = strtok(NULL, "/"); // update toke to the next token after "/"
    }

    if(token==NULL){
        name = prev_token;
    }
    else{
        #ifdef DEBUG_TOKEN
        std::cout<<"name = token: "<< name<<"\n";
        #endif
        name = token;  // if token is a file.

    }
    // std::cout<<"\nOpen file name: "<<name<<std::endl;
    DEBUG(dbgFile, "Opening file" << name);

    #ifdef DEBUG_OPENFILE
    std::cout<<"Print file_temp content.\n";
    #endif

    sector = directory->Find(name); // return the sector contain this file's header
    
    #ifdef DEBUG_OPENFILE
    std::cout<<"Cur dir->list(): \n";
    // directory->List();
    #endif

    if (sector >= 0){
        openFile = new OpenFile(sector); // name was found in directory
        DEBUG(dbgFileCreate, "Success in finding file when Open(), sector: "<<sector<<" for file "<<name);
    }
    delete directory;
    opfile = openFile;
    return openFile; // return NULL if not found
}



/* ----------- MP4 ----------------- */

int FileSystem::Write(char *buffer, int size, OpenFileId id){
    return opfile->Write(buffer, size);
}

int FileSystem::Read(char *buffer, int size, OpenFileId id){
    return opfile->Read(buffer,size);
}

int FileSystem::Close(OpenFileId id){
    delete opfile;
    return 1;
}

/* ---------------------------------- */






//----------------------------------------------------------------------
// FileSystem::Remove
// 	Delete a file from the file system.  This requires:
//	    Remove it from the directory
//	    Delete the space for its header
//	    Delete the space for its data blocks
//	    Write changes to directory, bitmap back to disk
//
//	Return TRUE if the file was deleted, FALSE if the file wasn't
//	in the file system.
//
//	"name" -- the text name of the file to be removed
//----------------------------------------------------------------------

bool FileSystem::Remove(char *name, bool Recursive)
{
    Directory *directory;
    PersistentBitmap *freeMap;
    FileHeader *fileHdr;
    int sector, last_sector;

    directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);
    OpenFile *file_temp = directoryFile;
    OpenFile *last_level_dir_file = file_temp;
    bool should_roll_back_to_last_level_dir = TRUE;
    // std::cout<<name<<std::endl;
    char *token = strtok(name, "/");
    char *prev_token = token;   

    /* MP4 */
    // STEP 1. take the token.
    int count=0;

    while( token!=NULL){
        last_sector = sector;
        sector = directory->Find(token);
        if(sector == -1){
            std::cout<<"No such file \'"<<token<<"\' in cur dir. Break\n";
            delete directory;
            return FALSE;
        } // Haven't have this token before.
        
        last_level_dir_file = file_temp;   // Last level directroy. (KEY here.)
       
        // std::cout<<"sector of token "<<token<<": "<<sector<<"\n\n";

        file_temp = new OpenFile(sector);
        directory->FetchFrom(file_temp);
        
        #ifdef DEBUG_CREATE_FILE
        std::cout<<"fetch from sector "<<sector<<", dir list\n";
        #endif
        prev_token = token; //update prev_token
        token = strtok(NULL, "/"); // update toke to the next token after "/"
    }

    OpenFile* actual_dir_opfile = file_temp;

    if(token==NULL){
        token = prev_token; // if traverse to the end.
        should_roll_back_to_last_level_dir = TRUE;
        directory->FetchFrom(last_level_dir_file);     //////// KEY!!!!
    }
    fileHdr = new FileHeader;   // The file to be remove.
    fileHdr->FetchFrom(sector);
    /*
        Note: This file header is the FCB of fild 'name'
        We should also remove file headers of 'sub_file's of this file. ( Clear the dataSectors of bitmap)
        And clear the bitmap of those sub_files' FCB sectors.   ( Clear the FCBsectors of bitmap)
    */

    //////// KEY!!!!
    // if(should_roll_back_to_last_level_dir){
        // directory->FetchFrom(file_temp);  // Since: the directory contain this file.
        #ifdef DEBUG_REMOVE
        std::cout<<"Cur dir: list\n";
        directory->List();
        #endif

        // std::cout<<"Try removing file \'"<<token<<"\' on disk sector "<<sector<<", numSectors: "<<fileHdr->get_num_of_sectors()<<"\n";
        DEBUG(dbgFileRemove, "Try removing file \'"<<token<<"\' on disk sector "<<sector<<", numSectors: "<<fileHdr->get_num_of_sectors());
    // }


    freeMap = new PersistentBitmap(freeMapFile, NumSectors);
    
    if(Recursive){
        if(directory->IsDir(token)){
            // std::cout<<"token "<<token<<" is a directory!\n";
            DEBUG(dbgFileRemove,  "token "<<token<<" is a directory!");
            // 1. Enter this absolute dir. /t0/bb
            directory->FetchFrom(actual_dir_opfile);
            DEBUG(dbgFileRemove,"Enter "<<token<<" dir:" );
            directory->List();

            directory->RecursiveRemove(token); // Pass original pathname (e.g. /t0).
                                        // Since we want to traverse all 'absolute path name' under this (cur) directory!
            // After return from recursive remove: 
            // cur dir: /bb
            directory->WriteBack(actual_dir_opfile); // flush to disk   //////// KEY!!!! 
            DEBUG(dbgFileRemove,"Return from Directoyr::RecursiveRemove("<<token<<"), check dir "<<token );
            // std::cout<<"Return from Directoyr::RecursiveRemove("<<token<<"), check dir "<<token<<"\n";
            // list bb.
            directory->List();             

            

            // std::cout<<"Now, want to deallocate the header of "<<token<<". \n";
            // fileHdr : /t0/bb
            // Clear /t0/bb's bitmap
            fileHdr->Deallocate(freeMap); // remove data blocks
            // std::cout<<"Now, want to clear the header-bit (sector number ) on bitmap\n";
            
            // sector of /bb 's FCB.
            freeMap->Clear(sector);       // remove header block                
            
            
            // Note: Go back to the last level dir and remove /bb!   
            // Now: in /t0
            directory->FetchFrom(last_level_dir_file);      
            // From t0 : remove /bb
            directory->Remove(token);         

            // Write back result to /t0
            directory->WriteBack(last_level_dir_file); // flush to disk   //////// KEY!!!! 
           
               
     
    
            DEBUG(dbgFileRemove, "Success in RecursiveRemove dir "<<token);
        
        }
        else{
            fileHdr->Deallocate(freeMap); // remove data blocks
            freeMap->Clear(sector);       // remove header block
            directory->Remove(token);
            directory->WriteBack(last_level_dir_file); // flush to disk   //////// KEY!!!!
        }
        //  directory->RecursiveRemove(token);
    }
    else {
            if(directory->IsDir(token)){
                std::cout<<"Cannot remove directory \'"<<token<<"\' using '-r'! Use '-rr' instead.\n";
                return FALSE;
            }
            fileHdr->Deallocate(freeMap); // remove data blocks of this 'File'
            freeMap->Clear(sector);       // remove one header block of this 'File'   (Clear that one bit on bitmap)    
            directory->Remove(token); // remove this file on the directory of this 'File'
            directory->WriteBack(last_level_dir_file); // flush to disk   //////// KEY!!!!
    }
    // std::cout<<"Return from Remove("<<token<<").\n";

    freeMap->WriteBack(freeMapFile);     // flush to disk
    // directory->WriteBack(directoryFile); // flush to disk
    // directory->WriteBack(file_temp); // flush to disk   //////// KEY!!!!
    delete fileHdr;
    delete directory;
    delete freeMap;
    return TRUE;
}


bool FileSystem::Remove(char *name)
{
    Directory *directory;
    PersistentBitmap *freeMap;
    FileHeader *fileHdr;
    int sector;

    directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile);
    sector = directory->Find(name);
    if (sector == -1)
    {
        delete directory;
        return FALSE; // file not found
    }
    fileHdr = new FileHeader;
    fileHdr->FetchFrom(sector);

    freeMap = new PersistentBitmap(freeMapFile, NumSectors);

    fileHdr->Deallocate(freeMap); // remove data blocks
    freeMap->Clear(sector);       // remove header block
    directory->Remove(name);

    freeMap->WriteBack(freeMapFile);     // flush to disk
    directory->WriteBack(directoryFile); // flush to disk
    delete fileHdr;
    delete directory;
    delete freeMap;
    return TRUE;
}

//----------------------------------------------------------------------
// FileSystem::List
// 	List all the files in the file system directory.
//----------------------------------------------------------------------


void FileSystem::List(char* dirname, bool Recursive)
{
    Directory *directory = new Directory(NumDirEntries);
    directory->FetchFrom(directoryFile); // first: take root dir!

    OpenFile* dirFile = directoryFile;
    int sector;

    char *token = strtok(dirname, "/");
    char *prev_token = dirname; // e.g. : "/t0" => get t0
    
    while( token!=NULL){
        #ifdef DEBUG_TOKEN
        printf("token: %s\n",token);
        #endif

        if( (sector = directory->Find(token)) ==-1 ){
            printf("No such file or directory: %s\n", token);
            return;
        } // No such file or directory!

        dirFile = new OpenFile(sector);
        directory->FetchFrom(dirFile);
        prev_token = token; //update prev_token
        token = strtok(NULL, "/"); // update toke to the next token after "/"
        DEBUG(dbgFile, "After strtok(NULL), token: "<<token);
    }

    // if(token!=NULL)std::cout<<"Token before processed: "<<token<<"\n";
    // else std::cout<<"Token before processed is NULL\n";

    if(token==NULL){    // only if pass "/"
        token  = prev_token;
        #ifdef DEBUG_TOKEN
        std::cout<<"Traverse path to the last token\n";
        #endif
    }
    #ifdef DEBUG_TOKEN
    std::cout<<"Token after processed: "<<token<<"\n";
    #endif
    
    // if root dir.
    if(strcmp(token, "/")==0) sector = DirectorySector;


    #ifdef DEBUG_TOKEN
    std::cout<<"FileSystem::List(): Dir "<<token<<" Fecth from sector "<<sector<<"\n";
    #endif
    
    if(Recursive){

        // std::cout<<"Checking: Current level :\n";
        // directory->List();
        // std::cout<<"Now passing RecursiveList(depth = 0);\n";

        directory->RecursiveList(0);  // start from depth 0.
        std::cout<<"\n";
    }
    else  directory->List(); 
    delete directory;
}


void FileSystem::List()
{
    Directory *directory = new Directory(NumDirEntries);

    directory->FetchFrom(directoryFile);
    directory->List();
    delete directory;
}


//----------------------------------------------------------------------
// FileSystem::Print
// 	Print everything about the file system:
//	  the contents of the bitmap
//	  the contents of the directory
//	  for each file in the directory,
//	      the contents of the file header
//	      the data in the file
//----------------------------------------------------------------------

void FileSystem::Print()
{
    FileHeader *bitHdr = new FileHeader;
    FileHeader *dirHdr = new FileHeader;
    PersistentBitmap *freeMap = new PersistentBitmap(freeMapFile, NumSectors);
    Directory *directory = new Directory(NumDirEntries);

    printf("Bit map file header:\n");
    bitHdr->FetchFrom(FreeMapSector);
    bitHdr->Print();

    printf("Directory file header:\n");
    dirHdr->FetchFrom(DirectorySector);
    dirHdr->Print();

    freeMap->Print();

    directory->FetchFrom(directoryFile);
    directory->Print();

    delete bitHdr;
    delete dirHdr;
    delete freeMap;
    delete directory;
}

#endif // FILESYS_STUB
