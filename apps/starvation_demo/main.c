#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <unistd.h>

#define GREEDY_ITERATIONS 1000000

static pthread_mutex_t resourceMutex = PTHREAD_MUTEX_INITIALIZER;

static atomic_int greedyFinished = 0;

static unsigned long greedyAccessCount = 0;
static unsigned long victimAccessCount = 0;
static unsigned long victimMissCount = 0;

static void *greedyThread(void *arg)
{
	(void)arg;
	
	for(unsigned long i = 0; i < GREEDY_ITERATIONS; i++ )
	{
		pthread_mutex_lock(&resourceMutex);
		
		++greedyAccessCount;
		
		pthread_mutex_unlock(&resourceMutex);

	}
	
	atomic_store(&greedyFinished, 1);
	
	printf("[GREEDY] Finished.\n");
	
	return NULL;
}

static void *victimThread(void *arg)
{
	(void)arg;
	
	while(atomic_load(&greedyFinished) == 0)
	{
		if(pthread_mutex_trylock(&resourceMutex) == 0)
		{
			++victimAccessCount;
			pthread_mutex_unlock(&resourceMutex);
		}
		else
		{
			++victimMissCount;
			
			struct timespec delay = {
                .tv_sec = 0,
                .tv_nsec = 100000
            };

            nanosleep(&delay, NULL);
		}
	}
	
	printf("[VICTIM] Finished.\n");
	
    return NULL;
}

int main(void)
{
	pthread_t greedy;
    pthread_t victim;

    printf("[MAIN] Starvation demonstration started.\n");

    pthread_create(&greedy, NULL, greedyThread, NULL);
    pthread_create(&victim, NULL, victimThread, NULL);

    pthread_join(greedy, NULL);
    pthread_join(victim, NULL);

    printf("\n--- RESULTS ---\n");
    printf("Greedy accesses : %lu\n", greedyAccessCount);
    printf("Victim accesses : %lu\n", victimAccessCount);
    printf("Victim misses   : %lu\n", victimMissCount);

    pthread_mutex_destroy(&resourceMutex);

    return 0;
}
