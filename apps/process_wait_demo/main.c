#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*****************************************************************************/
/**
 * @brief Runs the child process operation.
 *
 * @return This function does not return because the child process is
 *         terminated using exit().
 */
 static void runChildProcess(void)
 {
    printf("[CHILD] PID : %ld\n",(long)getpid());
    printf("[CHILD] PPID : %ld\n",(long)getppid());
    printf("[CHILD] Performing operation...\n");

    sleep(2);

    printf("[CHILD] Operation completed.\n");
    printf("[CHILD] Terminating with exit status 42.\n");

    exit(42);
 }


/*****************************************************************************/
/**
 * @brief Demonstrates child process synchronization and exit status handling.
 *
 * @return EXIT_SUCCESS when the demonstration completes successfully,
 *         otherwise EXIT_FAILURE.
 */
 int main(void)
 {
    pid_t childProcessId;
    pid_t terminatedProcessId;
    int childStatus;

    printf("[PARENT] Process started. PID: %ld\n", (long)getpid());

    childProcessId = fork();

    if(childProcessId < 0 )
    {
        perror("fork");
        return EXIT_FAILURE;
    }

    if(childProcessId == 0)
    {
        runChildProcess();
    }

    printf("[PARENT] Child created. PID: %ld\n", (long)childProcessId);
    printf("[PARENT] Waiting for child process...\n");

    terminatedProcessId = waitpid(childProcessId, &childStatus, 0);

    if(terminatedProcessId < 0)
    {
        perror("waitpid");
        return EXIT_FAILURE;
    }
    
    printf("[PARENT] Child process terminated.\n");

    if( WIFEXITED(childStatus))
    {
        printf("[PARENT] Child exited normally with status: %d\n",
                WEXITSTATUS(childStatus));
    }
    else if(WIFSIGNALED(childStatus))
    {
        printf("[PARENT] Child terminated by signal: %d\n",
                WTERMSIG(childStatus)); 
    }
    else
    {
        printf("[PARENT] Child terminated unexpectedly.\n");
    }
    return EXIT_SUCCESS;
 }