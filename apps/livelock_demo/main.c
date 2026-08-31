#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <time.h>

#define MAX_ATTEMPTS 10

static atomic_int threadOneWantsToProceed = 1;
static atomic_int threadTwoWantsToProceed = 1;

/**
 * @brief Sleeps for a short duration to make the livelock behavior observable.
 */
static void shortDelay(void)
{
    struct timespec delay = {.tv_sec = 0, .tv_nsec = 100000000};
    nanosleep(&delay, NULL);
}

/**
 * @brief Simulates the first polite thread.
 *
 * The thread repeatedly gives priority to Thread 2.
 * Thread 2 behaves the same way, so both threads remain active
 * without making useful progress.
 *
 * @param arg Unused thread argument.
 *
 * @return Always returns NULL.
 */
static void *threadOne(void *arg)
{
    (void)arg;

    for (int attempt = 1; attempt <= MAX_ATTEMPTS; ++attempt)
    {
        printf("[THREAD 1] Attempt %d: I want to proceed.\n", attempt);

        atomic_store(&threadOneWantsToProceed, 1);

        shortDelay();

        if (atomic_load(&threadTwoWantsToProceed) != 0)
        {
            printf("[THREAD 1] Thread 2 also wants to proceed. I will step aside.\n");
            atomic_store(&threadOneWantsToProceed, 0);
            shortDelay();
            continue;
        }

        printf("[THREAD 1] Proceeding with the work.\n");
        atomic_store(&threadOneWantsToProceed, 0);

        return NULL;
    }

    printf("[THREAD 1] No progress after %d attempts.\n", MAX_ATTEMPTS);

    return NULL;
}

/**
 * @brief Simulates the second polite thread.
 *
 * The thread repeatedly gives priority to Thread 1.
 *
 * @param arg Unused thread argument.
 *
 * @return Always returns NULL.
 */
static void *threadTwo(void *arg)
{
    (void)arg;

    for (int attempt = 1; attempt <= MAX_ATTEMPTS; ++attempt)
    {
        printf("[THREAD 2] Attempt %d: I want to proceed.\n", attempt);

        atomic_store(&threadTwoWantsToProceed, 1);

        shortDelay();

        if (atomic_load(&threadOneWantsToProceed) != 0)
        {
            printf("[THREAD 2] Thread 1 also wants to proceed. I will step aside.\n");
            atomic_store(&threadTwoWantsToProceed, 0);
            shortDelay();
            continue;
        }

        printf("[THREAD 2] Proceeding with the work.\n");
        atomic_store(&threadTwoWantsToProceed, 0);

        return NULL;
    }

    printf("[THREAD 2] No progress after %d attempts.\n", MAX_ATTEMPTS);

    return NULL;
}

/**
 * @brief Starts the livelock demonstration.
 *
 * @return Returns 0 when the program terminates normally.
 */
int main(void)
{
    pthread_t thread1;
    pthread_t thread2;

    printf("[MAIN] Livelock demonstration started.\n");

    pthread_create(&thread1, NULL, threadOne, NULL);
    pthread_create(&thread2, NULL, threadTwo, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("[MAIN] Livelock demonstration finished.\n");

    return 0;
}

