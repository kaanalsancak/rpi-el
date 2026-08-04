
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define CHILD_OBSERVATION_CYCLE_COUNT    (120U)
#define CHILD_OBSERVATION_PERIOD_SECONDS (1U)

/*****************************************************************************/
/**
 * @brief Runs the orphan process observation in the child process.
 *
 * @param synchronizationWriteFd Pipe descriptor used to notify the parent.
 *
 * @return EXIT_SUCCESS when the demonstration completes successfully,
 *         otherwise EXIT_FAILURE.
 */
static int runChildProcess(int synchronizationWriteFd)
{
	const char readyToken = 'R';
	const pid_t originalParentId = getppid();
	pid_t currentParentId;
	unsigned int cycleCount;

	printf("[CHILD] Process started. PID: %ld\n", (long)getpid());
	printf("[CHILD] Original parent PID: %ld\n", (long)originalParentId);
	fflush(stdout);

  /*
  * Notify the parent that the child has already captured and printed
  * its original parent PID.
  */

if(write(synchronizationWriteFd, &readyToken, sizeof(readyToken)) != 
		 (ssize_t)sizeof(readyToken) )
	{
		perror("write");
		close(synchronizationWriteFd);
		return EXIT_FAILURE;
	}
	
	close(synchronizationWriteFd);
	
	for(cycleCount = 0U; cycleCount < CHILD_OBSERVATION_CYCLE_COUNT; cycleCount++)
	{
		sleep(CHILD_OBSERVATION_PERIOD_SECONDS);
		currentParentId = getppid();
		printf("[CHILD] Cycle: %u, current PPID: %ld\n",
				cycleCount,
				(long)currentParentId);
				
		if (currentParentId != originalParentId)
        {
            printf("[CHILD] Parent changed from %ld to %ld. "
                   "The child has been re-parented.\n",
					(long)originalParentId,
					(long)currentParentId);
        }
		
		fflush(stdout);
	}
    printf("[CHILD] Observation completed. Child is terminating.\n");

    return EXIT_SUCCESS;
}


/*****************************************************************************/
/**
 * @brief Demonstrates orphan process creation and Linux re-parenting.
 *
 * @return EXIT_SUCCESS when the demonstration completes successfully,
 *         otherwise EXIT_FAILURE.
 */
 int main(void)
 {
	int synchronizationPipe[2];
	pid_t childProcessId;
	char readyToken;
	ssize_t readResult;
	
	if(pipe(synchronizationPipe) != 0)
	{
		perror("pipe");
		
		return EXIT_FAILURE;
	}
	
	childProcessId = fork();
	
	if (childProcessId < 0)
    {
        perror("fork");

        close(synchronizationPipe[0]);
        close(synchronizationPipe[1]);

        return EXIT_FAILURE;
    }
	
	if(childProcessId == 0)
	{
		close(synchronizationPipe[0]);
		return runChildProcess(synchronizationPipe[1]);
	}
	
	close(synchronizationPipe[1]);
	
	
    /*
     * Wait until the child captures its original parent PID.
     * This prevents the parent from terminating too early.
     */
	 
	readResult = read( synchronizationPipe[0], &readyToken, sizeof(readyToken));
	
	close(synchronizationPipe[0]);
	
	if (readResult != (ssize_t)sizeof(readyToken))
    {
		if (readResult < 0)
        {
            perror("read");
        }
        else
        {
            fprintf(stderr, "[PARENT] Synchronization failed.\n");
        }

        return EXIT_FAILURE;
    }
	
	printf("[PARENT] Process PID: %ld\n", (long)getpid());
    printf("[PARENT] Child PID  : %ld\n", (long)childProcessId);
    printf("[PARENT] Child is ready. Parent is terminating now.\n");
    fflush(stdout);

    return EXIT_SUCCESS;
	
 }