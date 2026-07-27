#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*****************************************************************************/
/**
 * @brief Demonstrates the lifecycle of a zombie process.
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

    if (childProcessId < 0)
    {
        perror("fork");
        return EXIT_FAILURE;
    }

   if (childProcessId == 0)
    {
        printf("[CHILD] PID : %ld\n", (long)getpid());
        printf("[CHILD] PPID: %ld\n", (long)getppid());
        printf("[CHILD] Terminating now.\n");

        exit(42);
    }

    printf("[PARENT] Child created. PID: %ld\n", (long)childProcessId);
    printf("[PARENT] Sleeping for 60 seconds.\n");
    printf("[PARENT] During this period, the child will be a zombie.\n");
    printf("[PARENT] Inspect child PID %ld from another terminal.\n",
           (long)childProcessId);

    sleep(60);

    printf("[PARENT] Calling waitpid() to collect child status.\n");

    terminatedProcessId = waitpid(childProcessId, &childStatus, 0);


        if (terminatedProcessId < 0)
    {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(childStatus))
    {
        printf("[PARENT] Child exit status: %d\n",
               WEXITSTATUS(childStatus));
    }

    printf("[PARENT] Zombie process has been removed.\n");

    return EXIT_SUCCESS;
}
