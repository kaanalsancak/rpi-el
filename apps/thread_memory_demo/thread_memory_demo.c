#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

/*
 * Shared global variable.
 *
 * All threads in the same process access the same instance of this variable
 * because the global data section belongs to the process address space.
 */
static int globalCounter = 0;

/*
 * Return the Linux thread ID of the calling thread.
 *
 * getpid() returns the process ID, which is shared by all threads.
 * gettid() returns a unique kernel thread ID for each thread.
 */
static int getThreadID(void)
{
	return syscall(SYS_gettid);
}


/*
 * Worker thread entry function.
 *
 * The argument points to dynamically allocated memory created by the main
 * thread. The heap belongs to the process address space, so both threads
 * can access the same memory location.
 */
static void *workerThread(void *arg)
{
	int localCounter = 0;
	int *heapValue = (int *)arg;
	
	printf("[WORKER] PID : %d TID : %d \n", getpid(), getThreadID());
	
	printf("[WORKER] Address of globalCounter : %p\n", (void *)&globalCounter);
    printf("[WORKER] Address of heapValue     : %p\n", (void *)heapValue);
    printf("[WORKER] Address of localCounter  : %p\n", (void *)&localCounter);
	
	globalCounter++;
    (*heapValue)++;
    localCounter++;

    printf("[WORKER] globalCounter = %d\n", globalCounter);
    printf("[WORKER] heapValue     = %d\n", *heapValue);
    printf("[WORKER] localCounter  = %d\n", localCounter);

    return NULL;
}

int main(void)
{
	pthread_t worker;
	int localCounter = 0;
	int *heapValue = NULL;
	
	heapValue = malloc(sizeof(int));
	
	if(heapValue == NULL)
	{
		fprintf(stderr, "[MAIN] malloc failed.\n");
        return EXIT_FAILURE;
	}
	
	*heapValue = 0;
	
	printf("[MAIN] PID: %d, TID: %d\n", getpid(), getThreadID());
    printf("[MAIN] Address of globalCounter : %p\n", (void *)&globalCounter);
    printf("[MAIN] Address of heapValue     : %p\n", (void *)heapValue);
    printf("[MAIN] Address of localCounter  : %p\n", (void *)&localCounter);
	
	if(pthread_create(&worker, NULL, workerThread, heapValue) != 0)
	{
		fprintf(stderr, "[MAIN] pthread_create failed.\n");
        free(heapValue);
		
        return EXIT_FAILURE;
	}
	if (pthread_join(worker, NULL) != 0)
    {
        fprintf(stderr, "[MAIN] pthread_join failed.\n");
        free(heapValue);

        return EXIT_FAILURE;
    }
	
	printf("[MAIN] After worker thread:\n");
    printf("[MAIN] globalCounter = %d\n", globalCounter);
    printf("[MAIN] heapValue     = %d\n", *heapValue);
    printf("[MAIN] localCounter  = %d\n", localCounter);

    free(heapValue);

    return EXIT_SUCCESS;
}