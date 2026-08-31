#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static pthread_mutex_t mutexA = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t mutexB = PTHREAD_MUTEX_INITIALIZER;

static void *threadOne(void *arg)
{
	(void)arg;
	
	printf("[THREAD 1] Trying to lock Mutex A...\n");
	pthread_mutex_lock(&mutexA);
	
	printf("[THREAD 1] Mutex A locked.\n");

    sleep(1);
	
	printf("[THREAD 1] Trying to lock Mutex B...\n");
	pthread_mutex_lock(&mutexB);
	
	printf("[THREAD 1] Mutex B locked.\n");
	
	pthread_mutex_unlock(&mutexB);
	pthread_mutex_unlock(&mutexA);
	
	printf("[THREAD 1] Finished.\n");

    return NULL;
}

static void *threadTwo(void *arg)
{
	(void)arg;
	
	printf("[THREAD 2] Trying to lock Mutex B...\n");
	pthread_mutex_lock(&mutexB);
	
	printf("[THREAD 2] Mutex B locked.\n");
	
	sleep(1);
	
	printf("[THREAD 2] Trying to lock Mutex A...\n");
	pthread_mutex_lock(&mutexA);
	
    printf("[THREAD 2] Mutex A locked.\n");

	pthread_mutex_unlock(&mutexA);
	pthread_mutex_unlock(&mutexB);
	
	printf("[THREAD 2] Finished.\n");

    return NULL;
}

int main(void)
{
	pthread_t thread1;
	pthread_t thread2;
	
	printf("[MAIN] Deadlock demonstration started.\n");
	
	pthread_create(&thread1, NULL, threadOne, NULL);
	pthread_create(&thread2, NULL, threadTwo, NULL);
	
	pthread_join(thread1,NULL);
	pthread_join(thread2,NULL);
	
	printf("[MAIN] Program finished,\n");
	
	return 0;
}