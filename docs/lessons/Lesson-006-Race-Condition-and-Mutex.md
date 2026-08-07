# Race Conditions and Mutexes in POSIX Threads

## 1. Objective

The purpose of this lesson is to understand:

* Shared data between threads
* Race conditions
* Lost updates
* Critical sections
* POSIX mutexes
* Mutex locking and unlocking
* The performance cost of synchronization

Two applications are used:

* `race_condition_demo`
* `mutex_counter_demo`

The first application intentionally creates a race condition.

The second application protects the shared resource using a POSIX mutex.

---

## 2. Shared Data Between Threads

Threads belonging to the same process share the same virtual address space.

This means that global and static variables can be accessed by multiple threads.

Example:

```c
static uint64_t sharedCounter = 0U;
```

If two worker threads access and modify this variable, both threads operate on the same memory location.

Conceptually:

```text
Thread 1 ----\
              ---> sharedCounter
Thread 2 ----/
```

This makes communication between threads easy, but it also introduces synchronization problems.

---

## 3. Race Condition

A race condition occurs when the behavior of a program depends on the execution order of multiple concurrent operations.

In the first application, two threads execute:

```c
sharedCounter++;
```

Each thread performs this operation:

```text
5,000,000 times
```

Therefore, the expected result is:

```text
5,000,000 + 5,000,000
= 10,000,000
```

However, `sharedCounter++` is not necessarily a single atomic operation.

Conceptually, it can be considered as:

```text
LOAD sharedCounter
ADD  1
STORE sharedCounter
```

Consider the following situation:

```text
Initial sharedCounter = 100

Thread 1                   Thread 2
--------                   --------

LOAD  -> 100
                           LOAD  -> 100

ADD   -> 101
                           ADD   -> 101

STORE -> 101
                           STORE -> 101
```

Two increment operations were executed, but the final value became:

```text
101
```

instead of:

```text
102
```

One update was lost.

This is called a:

```text
Lost Update
```

---

## 4. Race Condition Demo

The application uses two worker threads that modify the same global counter without synchronization.

The critical statement is:

```c
sharedCounter++;
```

No mutex or atomic operation protects the shared variable.

An observed execution produced:

```text
[MAIN] Race condition demo started.
[MAIN] Expected counter : 10000000
[MAIN] Actual counter   : 5013500
[MAIN] Race condition demo finished.
```

The expected value was:

```text
10000000
```

but the actual value was:

```text
5013500
```

This demonstrates that many updates were lost because both threads accessed the shared variable concurrently.

From the C language perspective, unsynchronized concurrent accesses to the same memory location, where at least one access is a write, constitute a data race and result in undefined behavior.

Therefore, the exact incorrect value must not be relied upon.

---

## 5. Critical Section

A critical section is a section of code that accesses shared state and must not be executed concurrently by multiple threads.

In this example:

```c
sharedCounter++;
```

is the critical section.

Only one thread should execute this operation at a time.

One method for providing this protection is a mutex.

---

## 6. Mutex

Mutex means:

```text
Mutual Exclusion
```

A mutex allows only one thread at a time to own a particular lock.

The POSIX mutex type is:

```c
pthread_mutex_t
```

A mutex can be initialized statically:

```c
static pthread_mutex_t counterMutex =
    PTHREAD_MUTEX_INITIALIZER;
```

The basic mutex operations are:

```c
pthread_mutex_init()
pthread_mutex_lock()
pthread_mutex_unlock()
pthread_mutex_destroy()
```

---

## 7. `pthread_mutex_init()`

A mutex can be initialized dynamically using:

```c
int pthread_mutex_init(
    pthread_mutex_t *mutex,
    const pthread_mutexattr_t *attr);
```

Example:

```c
pthread_mutex_t mutex;

pthread_mutex_init(&mutex, NULL);
```

The second parameter specifies mutex attributes.

Passing:

```c
NULL
```

uses the default mutex attributes.

Alternatively, a mutex with static storage duration can be initialized using:

```c
PTHREAD_MUTEX_INITIALIZER
```

---

## 8. `pthread_mutex_lock()`

The function:

```c
pthread_mutex_lock(&counterMutex);
```

attempts to acquire the mutex.

If the mutex is available:

```text
Thread acquires mutex
        ?
Thread continues execution
```

If another thread already owns the mutex:

```text
Thread calls lock
        ?
Mutex is already locked
        ?
Thread blocks
        ?
Mutex becomes available
        ?
Thread acquires mutex
```

`pthread_mutex_lock()` is therefore a blocking operation.

---

## 9. `pthread_mutex_unlock()`

The function:

```c
pthread_mutex_unlock(&counterMutex);
```

releases a mutex owned by the calling thread.

Typical usage is:

```c
pthread_mutex_lock(&counterMutex);

sharedCounter++;

pthread_mutex_unlock(&counterMutex);
```

The protected code between `lock()` and `unlock()` is the critical section.

---

## 10. Mutex-Protected Counter

The second application protects the shared counter using:

```c
pthread_mutex_lock(&counterMutex);

sharedCounter++;

pthread_mutex_unlock(&counterMutex);
```

The execution flow becomes:

```text
Thread 1
    |
    +--> lock
    |
    +--> sharedCounter++
    |
    +--> unlock
             |
             +--> Thread 2 can acquire the mutex
```

If Thread 2 attempts to acquire the mutex while Thread 1 owns it, Thread 2 must wait.

Therefore, the critical section cannot be executed concurrently by both threads.

---

## 11. Mutex Demo Result

The mutex-protected application produced:

```text
[MAIN] Mutex counter demo started
[MAIN] Expected counter : 10000000
[MAIN] Actual counter   : 10000000
[MAIN] Mutex counter demo finished.
```

The actual counter now matches the expected counter:

```text
Expected = 10000000
Actual   = 10000000
```

The race condition was eliminated because access to the shared counter was serialized by the mutex.

---

## 12. Mutex Does Not Directly Protect a Variable

A mutex is not automatically associated with a particular variable.

For example:

```c
static uint64_t sharedCounter;
static pthread_mutex_t counterMutex;
```

The operating system does not know that `counterMutex` is intended to protect `sharedCounter`.

The relationship is established by the program design.

All threads accessing the protected shared state must follow the same locking protocol:

```text
Acquire counterMutex
        ?
Access sharedCounter
        ?
Release counterMutex
```

If another thread accesses `sharedCounter` without acquiring the mutex, the protection is broken.

---

## 13. Mutex and Performance

The mutex-protected application was noticeably slower than the race-condition version.

Each thread executes:

```text
5,000,000 iterations
```

Therefore, approximately:

```text
10,000,000 lock operations
+
10,000,000 unlock operations
```

are performed.

The synchronized version introduces two important costs:

1. Mutex lock/unlock operations have synchronization overhead.
2. A thread may have to wait while another thread owns the mutex.

The current implementation performs:

```text
LOCK
counter++
UNLOCK

LOCK
counter++
UNLOCK

LOCK
counter++
UNLOCK

...
```

millions of times.

This implementation is useful for demonstrating mutex behavior, but it is not necessarily an efficient design.

---

## 14. Reducing Lock Contention

A more efficient design can reduce access to shared state.

Each thread could maintain a private local counter:

```c
uint64_t localCounter = 0U;

for (...)
{
    localCounter++;
}
```

Because `localCounter` belongs to the thread's own execution context, no mutex is required while incrementing it.

After the calculation is complete, the result can be added to the global counter once:

```c
pthread_mutex_lock(&counterMutex);

sharedCounter += localCounter;

pthread_mutex_unlock(&counterMutex);
```

Instead of millions of lock/unlock operations, each thread would only need to acquire the mutex once.

This illustrates an important multithreaded design principle:

> Minimize shared mutable state and minimize the time and frequency for which locks are held.

---

## 15. Mutex Lock Variants

POSIX provides different ways to attempt mutex acquisition.

### Blocking Lock

```c
pthread_mutex_lock(&mutex);
```

Waits until the mutex can be acquired.

### Non-Blocking Lock

```c
pthread_mutex_trylock(&mutex);
```

Attempts to acquire the mutex immediately.

If the mutex is already locked, it returns without waiting, typically with:

```text
EBUSY
```

### Timed Lock

```c
pthread_mutex_timedlock(
    &mutex,
    &absoluteTimeout);
```

Waits until either:

* The mutex becomes available, or
* The timeout expires.

If the timeout expires:

```text
ETIMEDOUT
```

is returned.

---

## 16. Important Mutex Rules

A mutex should follow a clear lifecycle:

```text
Initialize
    ?
Lock
    ?
Critical Section
    ?
Unlock
    ?
...
    ?
Destroy
```

Important rules include:

* A mutex must be initialized before use.
* A mutex should only protect the minimum necessary critical section.
* Every successful `lock()` must eventually have a matching `unlock()`.
* The thread that locks a normal mutex should also unlock it.
* All accesses to the protected shared state must follow the same mutex protocol.
* A mutex should not be destroyed while it is locked or still being used.
* Long-running or blocking operations should generally not be performed while holding a mutex.

---

## 17. Mutex and Scheduling

Mutex synchronization and thread scheduling are different concepts.

The scheduler determines:

```text
Which thread runs on the CPU?
```

The mutex determines:

```text
Which thread may enter the protected critical section?
```

A thread can acquire a mutex and then be preempted by the scheduler.

Another thread may execute, but if it attempts to acquire the same mutex, it must wait until the owner releases it.

Therefore, a mutex does not prevent thread scheduling or context switches.

It only controls concurrent access to the protected critical section.

---

## 18. `volatile` Is Not a Mutex

Declaring the counter as:

```c
volatile uint64_t sharedCounter;
```

would not solve the race condition.

`volatile` does not:

* Provide mutual exclusion
* Make `counter++` atomic
* Prevent lost updates
* Provide a locking mechanism
* Replace thread synchronization

Mutexes, atomic operations, and other synchronization primitives are required for thread-safe access to shared mutable state.

---

## 19. Applications Created

### Race Condition Demo

```text
apps/race_condition_demo/
+-- CMakeLists.txt
+-- race_condition_demo.c
```

Purpose:

* Demonstrate concurrent access to shared data
* Observe lost updates
* Observe race-condition behavior

### Mutex Counter Demo

```text
apps/mutex_counter_demo/
+-- CMakeLists.txt
+-- mutex_counter_demo.c
```

Purpose:

* Protect the same shared counter using a POSIX mutex
* Eliminate the race condition
* Observe synchronization overhead

---

## 20. Key Takeaways

This lesson demonstrated the transition:

```text
Shared Data
    ?
Concurrent Access
    ?
Race Condition
    ?
Lost Updates
    ?
Critical Section
    ?
Mutex Protection
    ?
Correct Result
```

The most important concepts are:

1. Threads inside the same process can access shared memory.
2. Concurrent modification of shared state requires synchronization.
3. `sharedCounter++` is not inherently thread-safe.
4. A mutex provides mutual exclusion around a critical section.
5. Mutex protection produces the correct result but introduces synchronization overhead.
6. Good multithreaded design minimizes both shared mutable state and lock contention.
