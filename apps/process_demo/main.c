#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
/******************************************************************************/
/**
 * @brief Prints basic process identification information.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array containing command-line argument strings.
 *
 * @return Returns 0 when the program completes successfully.
 */
int main(int argc,char *argv[])
{
  const pid_t processId = getpid();
  const pid_t parentProcessId = getppid();
  const uid_t userId = getuid();

  printf("Executableo : %s\n",argv[0]);
  printf("PID        : %ld\n",(long)processId);
  printf("PPID       : %ld\n", (long)parentProcessId);
  printf("UID        : %ld\n", (long)userId);
  printf("Arg Count  : %d\n", argc);
 
  printf("\nProcess will stay alive for 10 seconds...\n");

  sleep(30);

  return 0;
}
