#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/******************************************************************************/
/**
 * @brief Demonstrates how fork(), exec(), and wait() work together.
 *
 * @return Returns EXIT_SUCCESS when the demonstration completes successfully.
 */
int main(void)
{
    pid_t childProcessId;
    int childStatus;

    printf("Parent before fork: PID=%ld, PPID=%ld\n",
           (long)getpid(),
           (long)getppid());

    fflush(stdout);

    childProcessId = fork();

    if (childProcessId < 0)
    {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (childProcessId == 0)
    {
        printf("Child before exec: PID=%ld, PPID=%ld\n",
               (long)getpid(),
               (long)getppid());

        fflush(stdout);

        execl(
            "/bin/echo",
            "echo",
            "Child program replaced by /bin/echo",
            (char *)NULL);

        perror("execl");
        return EXIT_FAILURE;
    }

    printf("Parent after fork: PID=%ld, child PID=%ld\n",
           (long)getpid(),
           (long)childProcessId);

    if (waitpid(childProcessId, &childStatus, 0) < 0)
    {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(childStatus))
    {
        printf("Parent: child exited with status=%d\n",
               WEXITSTATUS(childStatus));
    }
    else
    {
        printf("Parent: child did not exit normally\n");
    }

    return EXIT_SUCCESS;
}