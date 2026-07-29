#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*****************************************************************************/
/**
 * @brief Stores the termination signal received by the process.
 */
static volatile sig_atomic_t g_receivedSignal = 0;

/*****************************************************************************/
/**
 * @brief Records a termination signal for processing by the main loop.
 *
 * @param signalNumber Received signal number.
 */
static void terminationSignalHandler(int signalNumber)
{
    g_receivedSignal = signalNumber;
}

/*****************************************************************************/
/**
 * @brief Installs handlers for SIGINT and SIGTERM.
 *
 * @return EXIT_SUCCESS when all signal handlers are installed successfully,
 *         otherwise EXIT_FAILURE.
 */
static int installSignalHandlers(void)
{
    struct sigaction action = {0};

    action.sa_handler = terminationSignalHandler;
    action.sa_flags = 0;

    if (sigemptyset(&action.sa_mask) != 0)
    {
        perror("sigemptyset");
        return EXIT_FAILURE;
    }

    if (sigaction(SIGINT, &action, NULL) != 0)
    {
        perror("sigaction SIGINT");
        return EXIT_FAILURE;
    }

    if (sigaction(SIGTERM, &action, NULL) != 0)
    {
        perror("sigaction SIGTERM");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*****************************************************************************/
/**
 * @brief Returns a readable name for a supported termination signal.
 *
 * @param signalNumber Signal number to identify.
 *
 * @return Pointer to the signal name string.
 */
static const char *getSignalName(int signalNumber)
{
    const char *signalName;

    switch (signalNumber)
    {
        case SIGINT:
        {
            signalName = "SIGINT";
            break;
        }

        case SIGTERM:
        {
            signalName = "SIGTERM";
            break;
        }

        default:
        {
            signalName = "UNKNOWN";
            break;
        }
    }

    return signalName;
}

/*****************************************************************************/
/**
 * @brief Performs simulated application cleanup before termination.
 */
static void performCleanup(void)
{
    printf("[APPLICATION] Stopping normal operations.\n");
    printf("[APPLICATION] Disabling simulated outputs.\n");
    printf("[APPLICATION] Closing simulated resources.\n");
    printf("[APPLICATION] Cleanup completed.\n");
}

/*****************************************************************************/
/**
 * @brief Demonstrates graceful termination using SIGINT and SIGTERM.
 *
 * @return EXIT_SUCCESS when the application terminates normally,
 *         otherwise EXIT_FAILURE.
 */
int main(void)
{
    unsigned int cycleCount = 0U;

    if (installSignalHandlers() != EXIT_SUCCESS)
    {
        return EXIT_FAILURE;
    }

    printf("[APPLICATION] Process started. PID: %ld\n", (long)getpid());
    printf("[APPLICATION] Press Ctrl+C to send SIGINT.\n");
    printf("[APPLICATION] You can also send SIGTERM from another terminal.\n");

    while (g_receivedSignal == 0)
    {
        printf("[APPLICATION] Running. Cycle: %u\n", cycleCount);
        fflush(stdout);

        cycleCount++;

        sleep(1U);
    }

    printf(
        "[APPLICATION] Received signal: %s (%d)\n",
        getSignalName(g_receivedSignal),
        (int)g_receivedSignal);

    performCleanup();

    printf("[APPLICATION] Process terminating normally.\n");

    return EXIT_SUCCESS;
}