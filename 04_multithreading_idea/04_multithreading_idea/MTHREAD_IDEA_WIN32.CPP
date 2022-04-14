//* Thread Synchronization in Windows *
#include <stdlib.h>
#include <stdio.h>
#include <windows.h>


typedef struct {
    unsigned long int duration;
    char              print_debug;
} parameters_t;

HANDLE Mutex_for_GlobalSum;
volatile unsigned long int GlobalSum; /* data is shared by the thread(s) */

/* the threads runs in these separate functions */
DWORD WINAPI Summation1(PVOID Param)
{
    parameters_t * P = (parameters_t*) Param;
    unsigned long int i, j;
    // local variables might be quicker to access and/or easier to optimize for a compiler
    const unsigned long int upper = P->duration;    
    const char              prn   = P->print_debug;

    for (i = 1; i <= 1000; i++) {
        for (j = 1; j <= upper; j++) {
            WaitForSingleObject(Mutex_for_GlobalSum,INFINITE);
            GlobalSum++;
            ReleaseMutex(Mutex_for_GlobalSum);
        }
        printf("%c", prn);
    }

    return 0;
}

DWORD WINAPI Summation2(PVOID Param)
{
    parameters_t * P = (parameters_t*) Param;
    unsigned long int i, j;
    // local variables might be quicker to access and/or easier to optimize for a compiler
    const unsigned long int upper = P->duration;    
    const char              prn   = P->print_debug;

    for (i = 1; i <= 1000; i++) {
        for (j = 1; j <= upper; j++) {
            WaitForSingleObject(Mutex_for_GlobalSum,INFINITE);
            GlobalSum--;
            ReleaseMutex(Mutex_for_GlobalSum);
        }
        printf("%c", prn);
    }

    return 0;
}


int main(int argc, char *argv[])
{
    DWORD ThreadId1;
    HANDLE ThreadHandle1;
    DWORD ThreadId2;
    HANDLE ThreadHandle2;

    parameters_t ThreadParam1;
    parameters_t ThreadParam2;
    long int Param;

    // do some basic error checking
    if (argc != 2) {
        fprintf(stderr,"Use: \"%s <DURATION>\"\n    or enter the duration now\n\n"
                       "Loop count = ", argv[0]);
        fscanf(stdin," %ld", &Param); // <- MSVS compiler warning is irrelevant here and can be avoided
    } else {
        Param = atol(argv[1]);
    }

    if (Param < 0) {
        fprintf(stderr, "an integer >= 0 is required \n");
        return -1;
    }

    ThreadParam1.duration    = Param;
    ThreadParam1.print_debug = 'x';

    ThreadParam2.duration    = Param;
    ThreadParam2.print_debug = 'o';

    // create the mutex
    Mutex_for_GlobalSum = CreateMutex(NULL, FALSE, NULL);
    // (security options,taken/available, pointer to name)

    // create the thread
    ThreadHandle1 = CreateThread(NULL, 0, Summation1, &ThreadParam1, 0, &ThreadId1);
    ThreadHandle2 = CreateThread(NULL, 0, Summation2, &ThreadParam2, 0, &ThreadId2);

    if (ThreadHandle1 != NULL) {
        WaitForSingleObject(ThreadHandle1, INFINITE);
        CloseHandle(ThreadHandle1);
    }
    if (ThreadHandle2 != NULL) {
        WaitForSingleObject(ThreadHandle2, INFINITE);
        CloseHandle(ThreadHandle2);
    }

    CloseHandle(Mutex_for_GlobalSum);

    if (GlobalSum==0)
        fprintf(stdout,"sum = %ld\n", GlobalSum);
    else
        fprintf(stdout,"sum = %ld (Shouldn\'t it be 0\a?)\n", GlobalSum);

    return(0);
}
