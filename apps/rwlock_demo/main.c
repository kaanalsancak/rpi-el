#define _POSIX_C_SOURCE 200809L

#include <pthread.h>
#include <stdio.h>
#include <time.h>

#define READER_COUNT 3
#define READER_ITERATIONS 5
#define WRITER_ITERATIONS 3

static pthread_rwlock_t sharedDataLock = PTHREAD_RWLOCK_INITIALIZER;
static int sharedValue = 0;

static void sleepMilliseconds(long milliseconds)
{
    struct timespec delay = {.tv_sec = milliseconds / 1000, .tv_nsec = (milliseconds % 1000) * 1000000L};
    nanosleep(&delay, NULL);
}

static void *readerThread(void *arg)
{
    long readerId = *(long *)arg;

    for (int iteration = 0; iteration < READER_ITERATIONS; ++iteration)
    {
        printf("[READER %ld] Waiting for read lock.\n", readerId);

        pthread_rwlock_rdlock(&sharedDataLock);

        printf("[READER %ld] Read lock acquired. sharedValue = %d\n", readerId, sharedValue);

        sleepMilliseconds(200);

        printf("[READER %ld] Releasing read lock.\n", readerId);

        pthread_rwlock_unlock(&sharedDataLock);

        sleepMilliseconds(100);
    }

    printf("[READER %ld] Finished.\n", readerId);

    return NULL;
}

static void *writerThread(void *arg)
{
    (void)arg;

    for (int iteration = 0; iteration < WRITER_ITERATIONS; ++iteration)
    {
        sleepMilliseconds(300);

        printf("[WRITER] Waiting for write lock.\n");

        pthread_rwlock_wrlock(&sharedDataLock);

        ++sharedValue;

        printf("[WRITER] Write lock acquired. sharedValue updated to %d\n", sharedValue);

        sleepMilliseconds(300);

        printf("[WRITER] Releasing write lock.\n");

        pthread_rwlock_unlock(&sharedDataLock);
    }

    printf("[WRITER] Finished.\n");

    return NULL;
}

int main(void)
{
    pthread_t readers[READER_COUNT];
    pthread_t writer;
    long readerIds[READER_COUNT] = {1, 2, 3};

    printf("[MAIN] Read-write lock demonstration started.\n");

    for (int i = 0; i < READER_COUNT; ++i)
    {
        pthread_create(&readers[i], NULL, readerThread, &readerIds[i]);
    }

    pthread_create(&writer, NULL, writerThread, NULL);

    for (int i = 0; i < READER_COUNT; ++i)
    {
        pthread_join(readers[i], NULL);
    }

    pthread_join(writer, NULL);

    pthread_rwlock_destroy(&sharedDataLock);

    printf("[MAIN] Final sharedValue = %d\n", sharedValue);
    printf("[MAIN] Program finished.\n");

    return 0;
}