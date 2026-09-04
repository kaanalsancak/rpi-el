#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define WORKER_COUNT 3

static pthread_barrier_t phaseBarrier;

static void sleepMilliseconds(long milliseconds)
{
    struct timespec delay = {.tv_sec = milliseconds / 1000, .tv_nsec = (milliseconds % 1000) * 1000000L};
    nanosleep(&delay, NULL);
}

static void *workerThread(void *arg)
{
	long workerId = *(long *) arg;
	
	printf("[WORKER %ld] Phase 1 started.\n ", workerId);
	
	sleepMilliseconds(workerId * 500);
	
	printf("[WORKER %ld] Phase 1 finished. Waiting at barrier.\n", workerId);
	  
	int result = pthread_barrier_wait(&phaseBarrier);
	
    if (result == PTHREAD_BARRIER_SERIAL_THREAD)
    {
        printf("[WORKER %ld] All workers reached the barrier.\n", workerId);
    }
	
    printf("[WORKER %ld] Phase 2 started.\n", workerId);

    return NULL;
	
}

int main(void)
{
	pthread_t workers[WORKER_COUNT];
	long workerIds[WORKER_COUNT] = {1, 2, 3};
	
	printf("[MAIN] Barrier demonstration started.\n");

    pthread_barrier_init(&phaseBarrier, NULL, WORKER_COUNT);
	for(int i = 0; i < WORKER_COUNT; i++)
	{
	   pthread_create(&workers[i], NULL, workerThread, &workerIds[i]);
	}
	
	for (int i = 0; i < WORKER_COUNT; ++i)
    {
        pthread_join(workers[i], NULL);
    }

    pthread_barrier_destroy(&phaseBarrier);

    printf("[MAIN] Program finished.\n");

    return 0;
}
