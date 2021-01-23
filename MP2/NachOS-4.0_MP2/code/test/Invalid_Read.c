#include "syscall.h"
// #include "stdio.h"

int main(void)
{
	char test[26];
	char check[] = "abcdefghijklmnopqrstuvwxyz";
	OpenFileId fid;
	int count, success, i;
	fid = Open("Invalid_read.test");
    PrintInt(fid);
	if (fid < 0) MSG("Failed on opening file");
	count = Read(test, 26, fid);
	
	// PrintInt(count);
	// cou

	if (count != 26) MSG("Failed on reading file");
	
	success = Close(fid);
	if (success != 1) MSG("Failed on closing file");

	for (i = 0; i < 26; ++i) {
		if (test[i] != check[i]) MSG("Failed: reading wrong result");
	}
	MSG("Passed! ^_^");
	Halt();
}

