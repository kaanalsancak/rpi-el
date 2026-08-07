#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define ITERATION_COUNT 5000000UL

static uint64_t sharedCounter = 0U;

static pthread_mutex_t counterMutex = PTHREAD_MUTEX_INITIALIZER;

static void *incrementCount(void *argument)
{
	(void)argument;
	
	for(uint64_t i = 0U; i < ITERATION_COUNT; i++)
	{
		pthread_mutex_lock(&counterMutex);
		
		sharedCounter++;
		
		pthread_mutex_unlock(&counterMutex);
	}
	
	return NULL;
}

int main(void)
{
	pthread_t thread1;
	pthread_t thread2;
	
	int result;
	
	printf("[MAIN] Mutex counter demo started\n");
	
	result = pthread_create(&thread1, NULL, incrementCount, NULL);
    if (result != 0)
    {
        fprintf(stderr, "[ERROR] Failed to create thread 1.\n");
        return EXIT_FAILURE;
    }

    result = pthread_create(&thread2, NULL, incrementCount, NULL);

    if (result != 0)
    {
        fprintf(stderr, "[ERROR] Failed to create thread 2.\n");

        pthread_join(thread1, NULL);

        return EXIT_FAILURE;
    }

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("[MAIN] Expected counter : %" PRIu64 "\n", (uint64_t)(2UL * ITERATION_COUNT));
    printf("[MAIN] Actual counter   : %" PRIu64 "\n", sharedCounter);

    result = pthread_mutex_destroy(&counterMutex);

    if (result != 0)
    {
        fprintf(stderr, "[ERROR] Failed to destroy mutex.\n");
        return EXIT_FAILURE;
    }

    printf("[MAIN] Mutex counter demo finished.\n");

    return EXIT_SUCCESS;
}