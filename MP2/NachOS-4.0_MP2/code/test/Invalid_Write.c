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
	fid = Open("invalid_write.test");
	
    PrintInt(fid);
	if (fid < 0) MSG("Failed on opening file");
	
	// PrintInt(fid);
	
	
	for (i = 0; i < 26; ++i) {
		int count = Write(test + i, 1, fid);
		// MSG("count : ");
		// PrintInt((OpenFileId)(count));
		if (count != 1) MSG("Failed on writing file");
	}
	
       
	success = Close(fid);
	// PrintInt(success); // success is 0.
	if (success != 1) MSG("Failed on closing file");
	MSG("Success on creating file1.test");
	Halt();
}

