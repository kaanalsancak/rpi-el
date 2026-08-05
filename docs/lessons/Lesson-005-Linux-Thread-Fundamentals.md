# Lesson 005 — Linux Thread Fundamentals

## 1. Lesson Overview

This lesson introduces threads in Linux and explains the fundamental differences between processes and threads.

The theoretical and practical sections cover:

- The definition of a thread
- Single-threaded and multithreaded processes
- Process and thread differences
- Resources shared by threads
- Resources private to each thread
- Concurrency and parallelism
- Linux PID and TID values
- POSIX thread handles
- Creating threads with `pthread_create()`
- Passing arguments to a thread
- Waiting for a thread with `pthread_join()`
- Returning a result from a thread
- Thread scheduling behavior
- Basic thread lifetime rules
- Embedded Linux thread architecture considerations

The following practical application was created:

```text
apps/thread_create_demo
```

---

## 2. What Is a Thread?

A process is a running program together with the resources required by that program.

A thread is an execution flow inside a process.

A simplified model is:

```text
Process = Resource container
Thread  = Execution flow inside the container
```

Every process starts with at least one thread.

When a program begins execution, Linux creates the process and its initial thread.

That initial thread executes:

```c
int main(void)
{
    /* Application execution begins here. */
}
```

The initial thread is commonly called the:

```text
Main thread
```

All applications created in the previous lessons were single-threaded processes, even though no thread API was used explicitly.

---

## 3. Single-Threaded Process

A single-threaded process contains one execution flow:

```text
Process
L¦¦ Main thread
      +¦¦ Read input
      +¦¦ Process data
      +¦¦ Write output
      L¦¦ Wait
```

If the only thread blocks, the process cannot perform another application operation until that thread continues.

For example:

```c
read(socketFileDescriptor, buffer, bufferSize);
```

may wait until data becomes available.

If the process contains only one thread:

```text
Main thread blocks
        |
        +-- No other application thread can continue the work
```

This may be unsuitable when the application must perform several independent activities.

---

## 4. Multithreaded Process

A process may contain multiple threads:

```text
Process
+¦¦ Main thread
+¦¦ Control thread
+¦¦ Communication thread
L¦¦ Logger thread
```

Each thread is an independent execution flow managed by the Linux scheduler.

For example:

```text
Control thread:
    Execute a control algorithm periodically

Communication thread:
    Wait for UART or network messages

Logger thread:
    Write queued records to a file
```

If the communication thread is blocked in `read()`, the control and logger threads may continue running.

---

## 5. Process and Thread Difference

Separate processes normally have separate virtual address spaces:

```text
Process A
+¦¦ Code
+¦¦ Global data
+¦¦ Heap
L¦¦ Stack

Process B
+¦¦ Code
+¦¦ Global data
+¦¦ Heap
L¦¦ Stack
```

Process A cannot normally access the global variables of Process B directly.

Threads inside the same process share the same virtual address space:

```text
One process
+¦¦ Shared code
+¦¦ Shared global data
+¦¦ Shared heap
+¦¦ Main thread stack
L¦¦ Worker thread stack
```

This makes communication between threads efficient because they can access common memory.

However, it also creates synchronization risks because several threads may access the same data at the same time.

---

## 6. Resources Shared by Threads

Threads in the same process generally share:

- Code segment
- Global variables
- Static variables
- Heap memory
- Open file descriptors
- Sockets
- Pipes
- Process environment
- Current working directory
- Signal dispositions
- Process ID
- Virtual address space

Example shared variable:

```c
static int g_motorCommand = 0;
```

Several threads in the same process can access it:

```text
Communication thread › Writes the command
Control thread       › Reads the command
Logger thread        › Records the command
```

Because the variable is shared, synchronization may be required.

---

## 7. Resources Private to Each Thread

Each thread has its own execution context.

Thread-specific resources include:

- Stack
- CPU register values
- Program counter
- Stack pointer
- Linux thread ID
- Scheduling state
- Scheduling parameters
- Signal mask
- Thread-local storage
- POSIX thread-specific `errno` state

This allows different threads to execute different functions or different parts of the same code simultaneously.

---

## 8. Thread Stack

Each thread has a separate stack.

Example:

```c
static void *controlThreadFunction(void *argument)
{
    int localCounter = 0;

    return NULL;
}
```

The local variable:

```c
localCounter
```

is stored on the control thread's stack.

Another thread may contain a local variable with the same name:

```c
static void *loggerThreadFunction(void *argument)
{
    int localCounter = 100;

    return NULL;
}
```

These are separate variables:

```text
Control thread stack › localCounter = 0
Logger thread stack  › localCounter = 100
```

Local stack variables are not automatically shared between threads.

Their addresses may still be passed to another thread, but their lifetime must remain valid while they are being used.

---

## 9. CPU Context

Each thread has its own CPU execution state.

This state includes:

- Register values
- Program counter
- Stack pointer
- Scheduling information

When Linux switches from one thread to another, it conceptually performs:

```text
Save Thread A context
        |
        +-- Load Thread B context
        |
        +-- Execute Thread B
```

When Thread A runs again, its saved context is restored.

This operation is called a:

```text
Context switch
```

Context switches have a runtime cost and are not free.

---

## 10. Concurrency and Parallelism

Concurrency and parallelism are related but different concepts.

### Concurrency

Concurrency means several tasks make progress during the same time interval.

On one CPU core:

```text
Thread A runs
Thread B runs
Thread A runs again
Thread C runs
```

Only one thread may be executing physically at a given instant, but all threads make progress over time.

### Parallelism

Parallelism means multiple threads execute physically at the same time on different CPU cores.

```text
CPU Core 0 › Thread A
CPU Core 1 › Thread B
```

A multithreaded application enables parallelism, but it does not guarantee it.

The Linux scheduler decides:

- When a thread runs
- Which CPU runs the thread
- How long the thread runs
- When a context switch occurs

---

## 11. Linux Threads as Tasks

Linux internally represents execution flows as tasks.

Threads and processes are not completely separate scheduler object types.

A thread is conceptually a task that shares selected resources with other tasks.

Threads in the same process commonly share:

- Memory mappings
- File descriptor table
- Signal disposition table
- Other process resources

Each thread still has its own:

- Execution context
- Kernel scheduling identity
- Linux thread ID

POSIX thread implementations on Linux are commonly built using Linux task creation mechanisms such as `clone()`.

Applications should normally use the POSIX thread API rather than calling `clone()` directly.

---

## 12. PID and TID

Linux distinguishes between a process or thread-group identity and individual thread identities.

### PID

The function:

```c
getpid();
```

returns the process ID visible to the application.

All threads in the same process receive the same value from `getpid()`.

Example:

```text
Main thread:
    getpid() › 1289

Worker thread:
    getpid() › 1289
```

### TID

Each Linux thread has its own thread ID.

The function:

```c
gettid();
```

returns the Linux kernel thread ID of the calling thread.

Example:

```text
Main thread:
    gettid() › 1289

Worker thread:
    gettid() › 1290
```

The main thread's TID is normally equal to the visible process PID.

Additional worker threads have different TID values.

---

## 13. POSIX Thread Handle

POSIX represents a thread using:

```c
pthread_t
```

Example:

```c
pthread_t workerThread;
```

This value is a POSIX thread handle.

It is not required to be identical to:

- Linux PID
- Linux TID
- A simple integer value

A thread can obtain its own POSIX handle using:

```c
pthread_self();
```

Portable POSIX code should compare two `pthread_t` values using:

```c
pthread_equal(threadA, threadB);
```

rather than assuming that the handles can always be compared as ordinary integers.

---

## 14. POSIX Thread Header

The POSIX thread API is declared in:

```c
#include <pthread.h>
```

This header provides interfaces such as:

```c
pthread_create()
pthread_join()
pthread_exit()
pthread_self()
pthread_equal()
```

The program must also be linked with thread support.

CMake provides the portable target:

```cmake
Threads::Threads
```

---

## 15. Creating a Thread

A new POSIX thread is created using:

```c
pthread_create();
```

Prototype:

```c
int pthread_create(
    pthread_t *thread,
    const pthread_attr_t *attributes,
    void *(*startRoutine)(void *),
    void *argument);
```

The demonstration uses:

```c
createResult = pthread_create(
    &workerThread,
    NULL,
    workerThreadFunction,
    &workerConfiguration);
```

The parameters mean:

### `&workerThread`

The POSIX handle of the new thread is written to this object.

### `NULL`

Default thread attributes are used.

### `workerThreadFunction`

This is the function executed by the new worker thread.

### `&workerConfiguration`

The address of the worker configuration is passed to the thread function.

---

## 16. `pthread_create()` Return Value

`pthread_create()` returns:

```text
0        › Success
Nonzero  › Error number
```

Unlike many system calls, POSIX thread functions normally return an error number directly.

The following pattern should be used:

```c
createResult = pthread_create(
    &workerThread,
    NULL,
    workerThreadFunction,
    &workerConfiguration);

if (createResult != 0)
{
    fprintf(
        stderr,
        "pthread_create failed: %s\n",
        strerror(createResult));
}
```

Using only:

```c
perror("pthread_create");
```

may be incorrect because `pthread_create()` does not normally communicate its error through `errno`.

---

## 17. Thread Entry Function

A POSIX thread entry function has the following form:

```c
void *threadFunction(void *argument);
```

The worker function in the demonstration is:

```c
static void *workerThreadFunction(void *argument)
```

The generic `void *` argument allows the caller to pass the address of any object.

The generic `void *` return value allows the thread to return an address when it terminates.

---

## 18. Worker Configuration Structure

The demonstration defines:

```c
typedef struct
{
    unsigned int cycleCount;
    unsigned int periodSeconds;
    int resultCode;
} WorkerConfiguration;
```

The fields are:

### `cycleCount`

Defines how many worker cycles are executed.

### `periodSeconds`

Defines the sleep period between cycles.

### `resultCode`

Stores the result generated by the worker.

The structure is initialized in `main()`:

```c
WorkerConfiguration workerConfiguration =
{
    .cycleCount = WORKER_CYCLE_COUNT,
    .periodSeconds = WORKER_PERIOD_SECONDS,
    .resultCode = 0
};
```

---

## 19. Passing an Argument to the Worker

The worker receives:

```c
&workerConfiguration
```

as its thread argument.

Inside the worker, the generic pointer is converted back to the correct type:

```c
WorkerConfiguration *configuration;

configuration = (WorkerConfiguration *)argument;
```

After the conversion, the worker can access:

```c
configuration->cycleCount
configuration->periodSeconds
configuration->resultCode
```

Only the address is passed to the new thread.

The structure is not automatically copied.

---

## 20. Argument Lifetime

The object passed to a thread must remain valid while the thread uses it.

The demonstration stores the configuration in the `main()` stack:

```c
WorkerConfiguration workerConfiguration;
```

This is safe because:

```text
main() creates the configuration
        |
        +-- Worker receives its address
        |
        +-- Main remains active
        |
        +-- Main waits using pthread_join()
        |
        +-- Worker finishes
        |
        +-- Main reads the result
        |
        +-- main() returns
```

The configuration remains valid until after the worker terminates.

An unsafe design would pass the address of a local variable from a function that returns before the worker finishes.

Example unsafe pattern:

```c
static void createWorker(void)
{
    WorkerConfiguration configuration;

    pthread_create(
        &workerThread,
        NULL,
        workerThreadFunction,
        &configuration);
}
```

After `createWorker()` returns, the configuration object no longer has a valid lifetime.

The worker could then access invalid stack memory.

---

## 21. Null Argument Check

The worker checks:

```c
if (configuration == NULL)
{
    return NULL;
}
```

This protects the function from dereferencing a null argument.

If the argument is invalid, the worker terminates without attempting to access the configuration fields.

The returned value is also `NULL`, which the main thread later recognizes as an invalid worker result.

---

## 22. Main and Worker Identity

The main thread prints:

```c
printf(
    "[MAIN] Thread started. PID: %ld, TID: %ld\n",
    (long)getpid(),
    (long)gettid());
```

The worker prints:

```c
printf(
    "[WORKER] Thread started. PID: %ld, TID: %ld\n",
    (long)getpid(),
    (long)gettid());
```

Observed values:

```text
Main PID   : 1289
Main TID   : 1289

Worker PID : 1289
Worker TID : 1290
```

This proves:

```text
Main PID == Worker PID
Main TID != Worker TID
```

The threads belong to the same process but are separate Linux execution tasks.

---

## 23. Main Thread Execution

The main thread executes:

```c
#define MAIN_CYCLE_COUNT    (3U)
#define MAIN_PERIOD_SECONDS (1U)
```

The main loop is:

```c
for (cycleCount = 0U;
     cycleCount < MAIN_CYCLE_COUNT;
     cycleCount++)
{
    printf(
        "[MAIN] Running. Cycle: %u\n",
        cycleCount);

    fflush(stdout);

    sleep(MAIN_PERIOD_SECONDS);
}
```

The cycle counter runs through:

```text
Cycle 0
Cycle 1
Cycle 2
```

The main thread executes three cycles.

---

## 24. Worker Thread Execution

The worker configuration uses:

```c
#define WORKER_CYCLE_COUNT    (5U)
#define WORKER_PERIOD_SECONDS (1U)
```

The worker loop is:

```c
for (cycleCount = 0U;
     cycleCount < configuration->cycleCount;
     cycleCount++)
{
    printf(
        "[WORKER] Running. Cycle: %u\n",
        cycleCount);

    fflush(stdout);

    sleep(configuration->periodSeconds);
}
```

The worker cycle counter runs through:

```text
Cycle 0
Cycle 1
Cycle 2
Cycle 3
Cycle 4
```

The worker executes five cycles.

Because the worker runs longer than the main loop, the main thread must wait for it after completing its own cycles.

---

## 25. Scheduler-Dependent Output Order

The observed output included:

```text
[WORKER] Running. Cycle: 0
[WORKER] Running. Cycle: 1
[MAIN] Running. Cycle: 1
```

The main and worker output did not alternate in a strict sequence.

This is expected.

Even though both threads call:

```c
sleep(1U);
```

the function does not guarantee that a thread executes immediately when one second expires.

A more accurate interpretation is:

```text
Sleep for at least approximately one second.
After the sleep expires, become eligible to run.
```

The Linux scheduler then decides when the thread receives CPU time.

Program correctness must not depend on an assumed print order.

---

## 26. Waiting for the Worker

After completing its own cycles, the main thread prints:

```text
[MAIN] Waiting for the worker thread.
```

It then calls:

```c
joinResult = pthread_join(
    workerThread,
    &threadReturnValue);
```

`pthread_join()` blocks the calling thread until the selected worker thread terminates.

The relationship is conceptually similar to:

```text
Process synchronization:
    waitpid() waits for a child process

Thread synchronization:
    pthread_join() waits for a thread
```

The APIs manage different operating-system objects, but both allow one execution flow to wait for another to finish.

---

## 27. Joinable Threads

Threads created with default attributes are normally joinable.

A joinable thread retains some termination resources until another thread calls:

```c
pthread_join();
```

Only one successful join should be performed for a joinable thread.

After a successful join:

- The worker has terminated
- Its return value can be obtained
- Join-related resources can be released
- The thread should not be joined again

A terminated but unjoined thread should not normally be described as a zombie process, although the concepts have a limited similarity.

---

## 28. Returning a Worker Result

The worker stores:

```c
configuration->resultCode = WORKER_RESULT_CODE;
```

The result constant is:

```c
#define WORKER_RESULT_CODE (42)
```

The worker then returns the address of the result field:

```c
return &configuration->resultCode;
```

The return type is `void *`, so an object address can be returned.

The worker does not return the integer value directly.

It returns:

```text
The address of the integer result
```

---

## 29. Receiving the Worker Result

The main thread declares:

```c
void *threadReturnValue = NULL;
```

The address returned by the worker is stored through:

```c
pthread_join(
    workerThread,
    &threadReturnValue);
```

After successful completion, the main thread checks:

```c
if (threadReturnValue == NULL)
{
    /* Invalid worker result. */
}
```

It then converts the generic address to an integer pointer and reads the value:

```c
workerResult = *((int *)threadReturnValue);
```

This expression contains two operations.

First:

```c
(int *)threadReturnValue
```

interprets the generic pointer as an `int *`.

Then:

```c
*
```

dereferences the pointer and reads the integer stored at that address.

The resulting value is:

```text
42
```

---

## 30. Why the Returned Address Is Valid

The worker returns:

```c
&configuration->resultCode
```

This field belongs to:

```c
workerConfiguration
```

which is stored in the `main()` stack.

The main thread does not leave `main()` before joining the worker.

Therefore, the configuration object still exists when the worker returns the field address and when the main thread reads it.

The design would be unsafe if the returned address referred to a local variable in the worker function.

Unsafe example:

```c
static void *workerThreadFunction(void *argument)
{
    int localResult = 42;

    return &localResult;
}
```

When the worker returns, its stack is no longer valid for normal use.

The returned address would refer to an object whose lifetime had ended.

---

## 31. `pthread_join()` and Memory Visibility

The worker writes:

```c
configuration->resultCode = 42;
```

The main thread reads the result only after successful `pthread_join()`.

The join operation establishes the required synchronization between the terminating worker and the joining main thread.

Operations performed by the worker before termination are visible to the thread after a successful join.

No mutex is needed for this specific result exchange because:

```text
Main does not read resultCode while the worker is modifying it.
Main reads it only after pthread_join() succeeds.
```

Concurrent access to shared data during worker execution would require separate synchronization.

---

## 32. Thread Termination

A thread entry function can terminate by returning:

```c
return threadResult;
```

It can also terminate by calling:

```c
pthread_exit(threadResult);
```

For a worker thread, these are similar in effect.

The demonstration uses:

```c
return &configuration->resultCode;
```

because it is simple and readable.

---

## 33. `pthread_exit()` and `exit()`

These functions have different scopes.

### `pthread_exit()`

```c
pthread_exit(NULL);
```

terminates only the calling thread.

Other threads in the process may continue running.

### `exit()`

```c
exit(EXIT_FAILURE);
```

terminates the entire process.

All threads in the process are terminated.

### Returning from a Worker Function

```c
return NULL;
```

terminates only that worker thread.

### Returning from `main()`

```c
return EXIT_SUCCESS;
```

terminates the process.

Other threads do not continue after the process terminates.

Summary:

```text
Return from worker function › Calling worker terminates
pthread_exit()              › Calling thread terminates
exit()                      › Entire process terminates
Return from main()          › Entire process terminates
```

---

## 34. Error Handling for `pthread_join()`

The demonstration checks:

```c
joinResult = pthread_join(
    workerThread,
    &threadReturnValue);

if (joinResult != 0)
{
    fprintf(
        stderr,
        "[MAIN] pthread_join failed: %s\n",
        strerror(joinResult));

    return EXIT_FAILURE;
}
```

Like `pthread_create()`, `pthread_join()` returns an error number directly.

Possible errors may include:

- The thread is not joinable
- The thread handle is invalid
- The thread has already been joined
- A deadlock would be created
- A thread attempts to join itself

All thread API return values should be checked.

---

## 35. Output Buffer Flushing

The demonstration calls:

```c
fflush(stdout);
```

after important output messages.

This ensures that buffered output is sent to the terminal promptly.

This is useful in a multithreaded demonstration because:

- Main and worker write to the same terminal
- Execution order is scheduler-dependent
- Delayed buffering could make the observed order confusing
- The program intentionally demonstrates interleaved execution

Output order is still not guaranteed, but flushing reduces buffering delays.

---

## 36. CMake Thread Configuration

The application uses:

```cmake
find_package(Threads REQUIRED)
```

This asks CMake to locate the platform's thread support.

The executable is defined with:

```cmake
add_executable(thread_create_demo
    main.c
)
```

Thread support is linked using:

```cmake
target_link_libraries(thread_create_demo PRIVATE
    Threads::Threads
)
```

`Threads::Threads` is preferred over manually adding a platform-specific library name because CMake selects the correct compile and link options for the current platform.

---

## 37. GNU Feature Definition

The application calls:

```c
gettid();
```

This is a Linux-specific interface rather than a portable POSIX thread function.

The build enables GNU/Linux declarations using:

```cmake
target_compile_definitions(thread_create_demo PRIVATE
    _GNU_SOURCE
)
```

This makes the required declaration visible through the system headers.

Portable code should not assume that `gettid()` exists on non-Linux systems.

The POSIX thread handle remains available through:

```c
pthread_self();
```

---

## 38. Practical Test Output

The application produced:

```text
[MAIN] Thread started. PID: 1289, TID: 1289
[MAIN] Worker thread created successfully.
[MAIN] Running. Cycle: 0
[WORKER] Thread started. PID: 1289, TID: 1290
[WORKER] Running. Cycle: 0
[WORKER] Running. Cycle: 1
[MAIN] Running. Cycle: 1
[WORKER] Running. Cycle: 2
[MAIN] Running. Cycle: 2
[WORKER] Running. Cycle: 3
[MAIN] Waiting for the worker thread.
[WORKER] Running. Cycle: 4
[WORKER] Operation completed. Result: 42
[WORKER] Thread is terminating.
[MAIN] Worker thread completed. Result: 42
[MAIN] Process is terminating normally.
```

---

## 39. Practical Result Analysis

The test confirmed:

```text
Main PID       : 1289
Worker PID     : 1289
Main TID       : 1289
Worker TID     : 1290
Worker result  : 42
```

The same PID confirms that both threads belong to the same process.

The different TID values confirm that Linux schedules them as separate execution tasks.

The main thread completed three cycles and then blocked in `pthread_join()`.

The worker completed five cycles and returned result code `42`.

After the worker terminated, the main thread resumed and read the returned result.

---

## 40. Race Condition Introduction

Threads share global data and heap memory.

Consider:

```c
static int g_counter = 0;
```

Two threads may both execute:

```c
g_counter++;
```

This expression may conceptually involve:

```text
1. Read the current value
2. Add one
3. Write the new value
```

If both threads read the same original value before either writes the result, one increment can be lost.

Example:

```text
Initial value: 10

Thread A reads 10
Thread B reads 10
Thread A writes 11
Thread B writes 11

Expected value: 12
Actual value  : 11
```

This is a:

```text
Race condition
```

The first thread demonstration avoids this problem because shared result data is read only after `pthread_join()`.

Later lessons will introduce:

- Critical sections
- Mutexes
- Atomic operations
- Condition variables
- Semaphores
- Thread-safe queues

---

## 41. Thread Failure Scope

Threads share the same process address space.

If one thread performs an invalid memory access and causes:

```text
SIGSEGV
```

the entire process normally terminates.

Example:

```text
Logger thread causes SIGSEGV
        |
        +-- Process terminates
        |
        +-- Main thread terminates
        +-- Control thread terminates
        +-- Communication thread terminates
```

Threads provide efficient resource sharing, but they do not provide strong fault isolation.

Separate processes may be more appropriate when one component must be able to fail or restart without terminating another component.

---

## 42. Process or Thread Selection

Threads may be appropriate when:

- Components share large amounts of data
- Low-latency communication is required
- The operations belong to one application
- Multicore execution is useful
- Resource overhead should be limited

Separate processes may be appropriate when:

- Fault isolation is important
- Different privileges are required
- Components must restart independently
- Memory corruption must be contained
- Security boundaries are required
- Resource ownership must be strongly separated

Many systems use a hybrid architecture:

```text
System
+¦¦ Control process
-     +¦¦ Control thread
-     L¦¦ Sensor thread
-
+¦¦ Logger process
-     L¦¦ Logger thread
-
L¦¦ Communication process
      +¦¦ Receiver thread
      L¦¦ Transmitter thread
```

---

## 43. Embedded Linux Thread Architecture

An embedded Linux motor-control application may contain:

```text
Main process
+¦¦ Control thread
+¦¦ ADC acquisition thread
+¦¦ Communication thread
+¦¦ Logger thread
L¦¦ Health-monitor thread
```

### Control Thread

Possible responsibilities:

- Run periodically
- Read the latest feedback
- Execute control algorithms
- Calculate actuator outputs
- Detect deadline violations

### ADC Acquisition Thread

Possible responsibilities:

- Read sensor samples
- Validate measurements
- Update feedback snapshots
- Detect hardware communication errors

### Communication Thread

Possible responsibilities:

- Receive external commands
- Parse protocol messages
- Validate reference values
- Publish new commands to the control logic

### Logger Thread

Possible responsibilities:

- Receive log records through a queue
- Write records to storage
- Avoid blocking time-critical threads

### Health-Monitor Thread

Possible responsibilities:

- Check thread heartbeats
- Check communication timeouts
- Detect missed control deadlines
- Evaluate system health
- Allow watchdog refresh only when critical components are healthy

---

## 44. Important Embedded Design Questions

A multithreaded embedded application should define:

```text
Which thread owns each hardware resource?
Which thread may write each shared variable?
Which threads may only read the variable?
How is shared data synchronized?
What happens if a thread blocks too long?
What happens if a deadline is missed?
What happens if one thread stops responding?
Which thread performs shutdown and cleanup?
How are hardware outputs placed in a safe state?
```

Creating a separate thread for every function without a clear ownership model can make the system difficult to verify and maintain.

---

## 45. Thread Count Considerations

More threads do not automatically improve performance.

Each thread introduces:

- Stack memory usage
- Kernel scheduling state
- Context-switch cost
- Cache interference
- Synchronization requirements
- Debugging complexity
- Possible lock contention

A large number of unnecessary threads may reduce performance.

Thread count should be based on:

- Independent blocking operations
- Timing requirements
- CPU-core availability
- Data ownership
- Fault containment needs
- Scheduling strategy
- Memory constraints

---

## 46. Key Takeaways

- A thread is an execution flow inside a process.
- Every process starts with at least one main thread.
- Threads in the same process share the virtual address space.
- Global variables, static variables, heap, and file descriptors are shared.
- Each thread has its own stack, registers, program counter, and TID.
- `getpid()` returns the same process ID in all threads.
- `gettid()` returns a different Linux thread ID for each thread.
- `pthread_t` is a POSIX thread handle.
- `pthread_create()` creates a new thread.
- The thread entry function has a `void *` argument and return type.
- Thread arguments are passed as addresses rather than automatically copied objects.
- An argument object must remain valid while the worker uses it.
- `pthread_join()` waits for a joinable thread to terminate.
- A worker may return an address through `pthread_join()`.
- A returned address must refer to an object whose lifetime remains valid.
- POSIX thread functions return error numbers directly.
- Thread execution order is controlled by the Linux scheduler.
- Equal sleep periods do not guarantee alternating execution.
- Returning from a worker function terminates only that worker.
- Returning from `main()` or calling `exit()` terminates the entire process.
- `pthread_join()` provides synchronization with the completed worker.
- Shared-memory access may cause race conditions.
- Threads provide efficient communication but limited fault isolation.
- Embedded Linux thread designs require explicit ownership and synchronization rules.