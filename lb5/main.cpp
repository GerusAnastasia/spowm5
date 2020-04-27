#include "laba_linux.h"

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
   // getchar();
    return 0;
}