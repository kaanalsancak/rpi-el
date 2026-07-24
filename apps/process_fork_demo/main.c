#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>


/******************************************************************************/
/**
 * @brief Demonstrates the behavior of fork() for parent and child processes.
 *
 * @return Returns EXIT_SUCCESS when the demonstration completes successfully.
 */
int main(void){
 pid_t processId;
 printf("Before fork: PID=%ld, PPID=%ld\n",
       	(long)getpid(),
       	(long)getppid());
 		
fflush(stdout); 
 processId = fork();

 if(processId < 0)
 {
 perror("fork");
 return EXIT_FAILURE;
 }

 if(processId == 0)
 {
 printf("Child: PID=%ld, PPID=%ld, fork return %ld\n",
 	(long)getpid(),
 	(long)getppid(),
 	(long)processId);
 }
 else 
 {
  printf("Parent: PID=%ld, PPID=%ld, child PID=%ld\n",
 	(long)getpid(),
 	(long)getppid(),
 	(long)processId);
 }
  return EXIT_SUCCESS;
}

