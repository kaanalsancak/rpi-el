
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

/*
 * Shared data between the main thread and worker thread.
 *
 * The mutex protects access to the shared state.
 * The condition variable allows the worker to sleep until
 * new data becomes available.
 */
 
 static pthread_mutex_t dataMutex = PTHREAD_MUTEX_INITIALIZER;
 static pthread_cond_t dataCondition = PTHREAD_COND_INITIALIZER;
 
 static int sharedData = 0;
 static int dataReady = 0;
 
 /*
 * Worker thread entry function.
 *
 * The worker waits until the main thread makes data available.
 * pthread_cond_wait() releases the mutex while waiting and
 * reacquires it before returning.
 */
 
 static void *workerThread(void *arg)
 {
	(void)arg;
	
	printf("[WORKER] Thread started. \n");
	
	if(pthread_mutex_lock(&dataMutex) != 0 )
	{
		fprintf(stderr,"[WORKER] failed to lock mutex. \n");
		return NULL;
	}
	
	while(dataReady == 0)
	{
		printf("[WORKER] Data is not ready. Going to sleep... \n");
		if(pthread_cond_wait(&dataCondition, &dataMutex) != 0)
		{
			fprintf(stderr, "[WORKER] Condition wait failed.\n");
			pthread_mutex_unlock(&dataMutex);
			return NULL;
		}
		printf("[WORKER] Woke up. Checking condition again...\n");
	}
	/*
	* The worker owns the mutex here and the predicate is true,
	* so the shared data can be accessed safely.
	*/
	printf("[WORKER] Data received: %d\n", sharedData);

	if(pthread_mutex_unlock(&dataMutex) != 0 )
	{
		fprintf(stderr, "[WORKER] Failed to unlock mutex.\n");
        return NULL;	
	}
	
	printf("[WORKER] Thread completed.\n");

    return NULL;
 }

int main(void)
{
	pthread_t worker;
	
	printf("[MAIN] Creating worker thread.\n");

    if (pthread_create(&worker, NULL, workerThread, NULL) != 0)
    {
        fprintf(stderr, "[MAIN] Failed to create worker thread.\n");
        return EXIT_FAILURE;
    }
	
	/* Simulate some work before producing data.*/
	 
	printf("[MAIN] Simulating data preparation...\n");
    sleep(3);
	
	
  if (pthread_mutex_lock(&dataMutex) != 0)
	{
		fprintf(stderr, "[MAIN] Failed to lock mutex.\n");
        pthread_join(worker, NULL);
        return EXIT_FAILURE;	
	}
	
	/* Update the shared state while holding the mutex.*/
	sharedData = 42;
    dataReady = 1;
	
	printf("[MAIN] Data is ready: %d\n", sharedData);
    printf("[MAIN] Sending condition signal.\n");
	
	/* Notify one thread waiting on the condition variable.*/
	 
	if(pthread_cond_signal(&dataCondition) != 0)
	{
		fprintf(stderr, "[MAIN] Failed to unlock mutex.\n");
        pthread_join(worker, NULL);
        return EXIT_FAILURE;
	}		

    if (pthread_mutex_unlock(&dataMutex) != 0)
    {
        fprintf(stderr, "[MAIN] Failed to unlock mutex.\n");
        pthread_join(worker, NULL);
        return EXIT_FAILURE;
    }
	
	if(pthread_join(worker,NULL) != 0)
	{
		fprintf(stderr, "[MAIN] Failed to join worker thread.\n");
        return EXIT_FAILURE;
	}
	
	/* Destroy synchronization objects after all threads have finished using them.*/
	 
	pthread_cond_destroy(&dataCondition);
	pthread_mutex_destroy(&dataMutex);
	
	printf("[MAIN] Program completed.\n");

    return EXIT_SUCCESS;
}
