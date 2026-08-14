#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/*Counting semaphore used to represent pending events.*/
static sem_t eventSemaphore;

/* Counting semaphore used to represent pending events.*/
static void *workerThread(void *arg)
{
	const int eventCount = 4;
	
	(void)arg;
	
	printf("[WORKER] Thread Started");
	
	for(int i = 0; i < eventCount; i++)
	{
		printf("[WORKER] Waiting for event %d...\n", i + 1);
		if(sem_wait(&eventSemaphore) != 0)
		{
			perror("[WORKER] sem_wait failed");
			return NULL;
		}
		printf("[WORKER] Event %d received.\n", i + 1);
	}
	printf("[WORKER] All events processed.\n");

    return NULL;
}

int main(void)
{
	pthread_t worker;
	
	if(sem_init(&eventSemaphore,0,0) != 0)
	{
		perror("[MAIN] sem_init failed");
        return EXIT_FAILURE;
	}
	
	printf("[MAIN] Posting three events before worker starts.\n");

    for (int i = 0; i < 3; ++i)
    {
        if (sem_post(&eventSemaphore) != 0)
        {
            perror("[MAIN] sem_post failed");
            sem_destroy(&eventSemaphore);
            return EXIT_FAILURE;
        }

        printf("[MAIN] Event %d posted.\n", i + 1);
    }
	
	printf("[MAIN] Creating worker thread.\n");

    if (pthread_create(&worker, NULL, workerThread, NULL) != 0)
    {
        fprintf(stderr, "[MAIN] Failed to create worker thread.\n");
        sem_destroy(&eventSemaphore);
        return EXIT_FAILURE;
    }
	
	sleep(2);
	
	printf("[MAIN] Posting fourth event.\n");
	
	if(sem_post(&eventSemaphore) != 0)
	{
		perror("[MAIN] sem_post failed.\n");
		pthread_join(worker, NULL);
        sem_destroy(&eventSemaphore);
        return EXIT_FAILURE;
	}
	
	if (pthread_join(worker, NULL) != 0)
    {
        fprintf(stderr, "[MAIN] Failed to join worker thread.\n");
        sem_destroy(&eventSemaphore);
        return EXIT_FAILURE;
    }
	
	/* Destroy the semaphore only after all threads have finished using it. */
    if (sem_destroy(&eventSemaphore) != 0)
    {
        perror("[MAIN] sem_destroy failed");
        return EXIT_FAILURE;
    }
	
	printf("[MAIN] Program completed.\n");

    return EXIT_SUCCESS;
}
