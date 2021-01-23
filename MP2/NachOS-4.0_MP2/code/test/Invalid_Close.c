#include "syscall.h"
// #include "stdio.h"

int main(void)
{
	char test[] = "abcdefghijklmnopqrstuvwxyz";
	// int success= Create("file1.test");
	
	OpenFileId fid;
	// PrintInt(1);
    int success;
	int i;
// if (success != 1)
	// MSG("Success Create file1.test.");	
	
	// if (success != 1)
		// MSG("Failed on creating file");
	fid = Open("file1.test");
	
    // MSG("OpenFIleTable index: (a.k.a OpenFileId)");
    PrintInt(fid); // 
	if (fid < 0) MSG("Failed on opening file");
	
	// PrintInt(fid);
    
	
	

	
       
	success = Close(fid+1);
	PrintInt(success); // success is 0.
	if (success != 1) MSG("Failed on closing file");
	// MSG("Success on creating file1.test");
	Halt();
}

