#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <process.h>
#include <locale.h>
#include "win_laba.h"


int main()
{
	struct Data data;
	printf("Waiting...\n");
	CreateEventForThreads(&data);
	Search(&data);
	OpenLib();
	CreateThreads(&data);
	WaitThreads(&data);
	CloseApp(&data);
	printf("Press any key...\n");
	getchar();
	return 0;
}
