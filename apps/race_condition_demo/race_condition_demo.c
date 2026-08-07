#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ITERATION_COUNT 5000000UL

static uint64_t sharedCount = 0U;

static void *incrementCounter(void *argument)
{
	(void)argument;
	for(uint64_t i = 0; i < ITERATION_COUNT; i++)
	{
		sharedCount++;
	}
	return NULL;
}

int main(void)
{
	pthread_t thread1;
	pthread_t thread2;
	int result;
	
	printf("[MAIN] Race condition demo started.\n");
	
	result = pthread_create(&thread1, NULL,incrementCounter, NULL);
	
	if(result != 0)
	{
		fprintf(stderr, "[ERROR] Failed to create thread 1 \n");
		return EXIT_FAILURE;
	}
	
	result = pthread_create(&thread2,NULL, incrementCounter,NULL);
	
	if(result != 0)
	{
		fprintf(stderr, "[ERROR] Failed to create thread 2 \n");
		return EXIT_FAILURE;
	}	
	
	pthread_join(thread1, NULL);
	pthread_join(thread2, NULL);
	
	printf("[MAIN] Expected counter : %" PRIu64 "\n",(uint64_t)(2UL * ITERATION_COUNT));

    printf("[MAIN] Actual counter   : %" PRIu64 "\n", sharedCount);
	
	printf("[MAIN] Race condition demo finished.\n");

    return EXIT_SUCCESS;
}