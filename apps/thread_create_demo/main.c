#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAIN_CYCLE_COUNT	(3U)
#define MAIN_PERIOD_SECONDS     (1U)

#define WORKER_CYCLE_COUNT      (5U)
#define WORKER_PERIOD_SECONDS   (1U)

#define WORKER_RESULT_CODE      (42)

/*****************************************************************************/
/**
 * @brief Contains the configuration and result of the worker thread.
 */
 typedef struct
 {
	unsigned int cycleCount;
	unsigned int periodSeconds;
	int resultCode;
 }WorkerConfiguration;
 
/*****************************************************************************/
/**
 * @brief Runs the worker thread operation.
 *
 * @param argument Pointer to a WorkerConfiguration object.
 *
 * @return Pointer to the worker result code stored in the configuration.
 */
 
 static void *workerThreadFunction(void *argument)
 {
	WorkerConfiguration *configuration;
	unsigned int cycleCount;
	
	configuration = (WorkerConfiguration *)argument;
	
	if(configuration == NULL)
	{
		return NULL;
	}
	printf("[WORKER] Thread started. PID: %ld, TID: %ld\n",
			(long)getpid(),
			(long)gettid());
			
    fflush(stdout);
	
	for(cycleCount = 0U; cycleCount <configuration->cycleCount; cycleCount++)
	{
		printf("[WORKER] Running. Cycle: %u\n", cycleCount);
		
        fflush(stdout);

        sleep(configuration->periodSeconds);
	}
	
	configuration->resultCode = WORKER_RESULT_CODE;

    printf("[WORKER] Operation completed. Result: %d\n",
			configuration->resultCode);
    
	printf("[WORKER] Thread is terminating.\n");
    fflush(stdout);

    return &configuration->resultCode;
 }
 
 /*****************************************************************************/
/**
 * @brief Demonstrates POSIX thread creation, execution, and joining.
 *
 * @return EXIT_SUCCESS when the demonstration completes successfully,
 *         otherwise EXIT_FAILURE.
 */
 int main(void)
 {
	pthread_t workerThread;
	WorkerConfiguration workerConfiguration =
	{
		.cycleCount = WORKER_CYCLE_COUNT,
		.periodSeconds = WORKER_PERIOD_SECONDS,
		.resultCode = 0
	};
	
	void *threadReturnValue = NULL;
	unsigned int cycleCount;
	int createResult;
	int joinResult;
	int workerResult;
	
	printf("[MAIN] Thread started. PID: %ld, TID: %ld\n",
			(long)getpid(),
			(long)gettid());
			
	createResult = pthread_create(&workerThread,
								  NULL,
								  workerThreadFunction,
								  &workerConfiguration);
								  
	if (createResult != 0)
    {
        fprintf(stderr, "[MAIN] pthread_create failed: %s\n", strerror(createResult));

        return EXIT_FAILURE;
    }							  

	printf("[MAIN] Worker thread created successfully.\n");
	fflush(stdout);
	
	for (cycleCount = 0U; cycleCount < MAIN_CYCLE_COUNT; cycleCount++)
    {
		printf("[MAIN] Running. Cycle: %u\n", cycleCount);		
		fflush(stdout);

		sleep(MAIN_PERIOD_SECONDS);
    }
	
	printf("[MAIN] Waiting for the worker thread.\n");
    fflush(stdout);
	
	joinResult = pthread_join(workerThread,&threadReturnValue);
	
	if (joinResult != 0)
    {
        fprintf(stderr, "[MAIN] pthread_join failed: %s\n", strerror(joinResult));

        return EXIT_FAILURE;
    }
	
	if (threadReturnValue == NULL)
    {
        fprintf(stderr, "[MAIN] Worker thread returned an invalid result.\n");

        return EXIT_FAILURE;
    }
		
	workerResult = *((int *)threadReturnValue);
	
	printf("[MAIN] Worker thread completed. Result: %d\n", workerResult);
    printf("[MAIN] Process is terminating normally.\n");

    return EXIT_SUCCESS;
	
 }