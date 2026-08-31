
# Lesson 010 – Deadlock, Starvation, and Livelock

## 1. Objective

The objective of this lesson is to understand three important concurrency problems that can occur in multithreaded Linux applications:

* Deadlock
* Starvation
* Livelock

These problems are especially important in Embedded Linux systems where multiple threads may share hardware resources, communication interfaces, buffers, state variables, or device drivers.

---

## 2. Deadlock

A deadlock occurs when two or more threads wait indefinitely for resources held by each other.

Consider two mutexes:

```text
Mutex A
Mutex B
```

and two threads:

```text
Thread 1: Lock A -> Lock B
Thread 2: Lock B -> Lock A
```

The following situation can occur:

```text
Thread 1 owns Mutex A
Thread 2 owns Mutex B

Thread 1 waits for Mutex B
Thread 2 waits for Mutex A
```

Neither thread can continue.

### 2.1 Practical Observation

The `deadlock_demo` application intentionally created this situation.

Observed output:

```text
[MAIN] Deadlock demonstration started.
[THREAD 1] Trying to lock Mutex A...
[THREAD 1] Mutex A locked.
[THREAD 2] Trying to lock Mutex B...
[THREAD 2] Mutex B locked.
[THREAD 1] Trying to lock Mutex B...
[THREAD 2] Trying to lock Mutex A...
```

The program stopped making progress after this point.

The expected messages:

```text
[THREAD 1] Finished.
[THREAD 2] Finished.
[MAIN] Program finished.
```

were never reached.

The application had to be terminated using:

```bash
Ctrl+C
```

---

## 3. Preventing Deadlock with Lock Ordering

The original implementation used inconsistent mutex acquisition orders:

```text
Thread 1: A -> B
Thread 2: B -> A
```

This allows circular waiting.

The implementation was corrected so that both threads acquire mutexes in the same order:

```text
Thread 1: A -> B
Thread 2: A -> B
```

The mutexes are then normally released in reverse order:

```text
Lock A
Lock B

Critical Section

Unlock B
Unlock A
```

Example:

```c
pthread_mutex_lock(&mutexA);
pthread_mutex_lock(&mutexB);

/* Critical section */

pthread_mutex_unlock(&mutexB);
pthread_mutex_unlock(&mutexA);
```

After applying consistent lock ordering, the application completed normally.

### Important Rule

When multiple mutexes are required by different threads:

> Define a global lock acquisition order and use the same order everywhere.

For example:

```text
sensorMutex -> stateMutex -> logMutex
```

A different thread should not acquire these resources using an incompatible order such as:

```text
logMutex -> sensorMutex
```

---

## 4. Starvation

Starvation occurs when a thread is technically able to run but repeatedly fails to obtain enough CPU time or access to a required resource.

Unlike deadlock, the entire system does not necessarily stop.

One thread may continue making progress while another thread receives very little access.

The practical application used two threads:

```text
Greedy Thread
Victim Thread
```

The greedy thread repeatedly acquired and released the same mutex without voluntarily delaying itself.

Conceptually:

```text
LOCK
WORK
UNLOCK

LOCK
WORK
UNLOCK

LOCK
WORK
UNLOCK
...
```

The victim thread used:

```c
pthread_mutex_trylock()
```

If the resource was unavailable, it backed off temporarily.

---

## 5. Starvation Practical Result

The application was executed on the Raspberry Pi Zero 2 W.

Observed result:

```text
[MAIN] Starvation demonstration started.
[GREEDY] Finished.
[VICTIM] Finished.

--- RESULTS ---
Greedy accesses : 1000000
Victim accesses : 887
Victim misses   : 1018
```

The greedy thread accessed the protected resource:

```text
1,000,000 times
```

while the victim accessed it only:

```text
887 times
```

The victim also failed to acquire the mutex:

```text
1018 times
```

This demonstrates highly unfair resource access.

The victim was not completely prevented from running, so this particular execution is more accurately described as demonstrating a **strong starvation tendency** rather than mathematically infinite starvation.

---

## 6. Deadlock vs Starvation

Deadlock:

```text
Thread 1 -> waiting
Thread 2 -> waiting

System progress -> NO
```

Starvation:

```text
Greedy Thread -> progressing rapidly
Victim Thread -> progressing very slowly

System progress -> YES
Fair progress   -> NO
```

The important difference is that a deadlocked system cannot make progress, while a system suffering from starvation may still perform useful work.

---

## 7. Reducing Starvation

One simple experiment was to allow the greedy thread to briefly yield its aggressive resource usage.

Instead of immediately attempting to acquire the mutex again, a small delay can be introduced:

```c
pthread_mutex_unlock(&resourceMutex);

struct timespec delay = {
    .tv_sec = 0,
    .tv_nsec = 100000
};

nanosleep(&delay, NULL);
```

This is useful for demonstrating the concept, but inserting arbitrary delays is not generally a production-quality synchronization solution.

Real systems should use proper scheduling, synchronization, queueing, ownership, and resource-sharing designs.

---

## 8. POSIX Sleep Functions

During the starvation experiment, `usleep()` produced a compilation error because the project uses strict standards and modern POSIX feature definitions.

The original call:

```c
usleep(100);
```

was replaced with:

```c
struct timespec delay = {
    .tv_sec = 0,
    .tv_nsec = 100000
};

nanosleep(&delay, NULL);
```

The target was configured with:

```cmake
target_compile_definitions(starvation_demo
    PRIVATE
    _POSIX_C_SOURCE=200809L
)
```

This exposes the required modern POSIX declarations.

Using `nanosleep()` is preferable to relying on the obsolete `usleep()` interface.

---

## 9. Livelock

A livelock occurs when threads are not blocked but continuously react to each other in a way that prevents useful progress.

A common analogy is two people meeting in a narrow corridor.

```text
Person A moves right.
Person B moves right.

Both notice the conflict.

Person A moves left.
Person B moves left.

Both notice the conflict again.

Repeat...
```

Both people are actively moving, but neither actually passes.

This is different from deadlock.

In deadlock:

```text
No activity
No progress
```

In livelock:

```text
Continuous activity
No useful progress
```

---

## 10. Livelock Practical Demo

The `livelock_demo` application creates two cooperative threads.

Each thread:

1. Announces that it wants to continue.
2. Detects that the other thread also wants to continue.
3. Politely steps aside.
4. Tries again.
5. Repeats the same behavior.

Conceptually:

```text
Thread 1: I want to proceed.
Thread 2: I want to proceed.

Thread 1: You go first.
Thread 2: You go first.

Thread 1: I want to proceed.
Thread 2: I want to proceed.

...
```

The threads remain active but useful work is not completed.

The demonstration limits the number of attempts using:

```c
#define MAX_ATTEMPTS 10
```

Without such a limit, a theoretical livelock could continue indefinitely.

---

## 11. Why Livelock Is Dangerous

Livelock can be more difficult to diagnose than deadlock.

A deadlocked program often appears completely frozen.

A livelocked program may still:

* consume CPU time,
* execute instructions,
* change internal states,
* generate logs,
* respond to scheduling,
* repeatedly retry operations.

Because the application appears active, the absence of real progress may be harder to recognize.

---

## 12. Preventing Livelock

One common solution is to prevent all participants from retrying with exactly the same behavior and timing.

A typical technique is:

```text
Random Backoff
```

Instead of:

```text
Thread 1 -> retry immediately
Thread 2 -> retry immediately
```

threads can wait for different periods before retrying.

For example:

```text
Thread 1 -> wait 2 ms
Thread 2 -> wait 7 ms
```

Thread 1 may then successfully proceed before Thread 2 retries.

Similar backoff strategies are used in networking, distributed systems, resource arbitration, and concurrent algorithms.

---

## 13. Deadlock, Starvation, and Livelock Comparison

| Problem    | Threads Active? | System Progress?   | Main Problem                     |
| ---------- | --------------- | ------------------ | -------------------------------- |
| Deadlock   | No              | No                 | Circular waiting                 |
| Starvation | Some            | Yes                | Unfair resource access           |
| Livelock   | Yes             | No useful progress | Continuous conflicting reactions |

A simple way to remember them is:

```text
Deadlock:
Everybody waits.

Starvation:
Somebody waits too much.

Livelock:
Everybody moves, but nobody gets anywhere.
```

---

## 14. Embedded Linux Example

Consider an Embedded Linux application containing:

```text
Sensor Thread
Control Thread
Communication Thread
Logging Thread
```

They may share resources such as:

```text
SPI bus
I2C bus
UART
shared state
configuration data
logging buffers
device drivers
```

Poor synchronization design can therefore create serious runtime problems.

### Deadlock Example

```text
Control Thread:
stateMutex -> spiMutex

Logging Thread:
spiMutex -> stateMutex
```

This inconsistent lock ordering may cause deadlock.

### Starvation Example

A high-frequency thread may repeatedly acquire a mutex and prevent a lower-frequency thread from receiving reasonable access.

### Livelock Example

Two workers may repeatedly detect a conflict, release resources, retry simultaneously, and recreate the same conflict.

---

## 15. Design Guidelines

When designing multithreaded Embedded Linux software:

1. Keep critical sections short.
2. Avoid unnecessary nested mutexes.
3. Define a consistent lock acquisition order.
4. Do not hold a mutex while performing long blocking operations unless necessary.
5. Avoid aggressive polling loops.
6. Consider fairness when several threads share a resource.
7. Use backoff strategies when repeated retries can collide.
8. Monitor whether the system is making actual progress rather than only checking whether threads are running.
9. Prefer message passing or queues when shared-memory locking becomes overly complex.
10. Document synchronization ownership and locking rules.

---

## 16. Practical Applications

Applications created during this lesson:

```text
apps/deadlock_demo/
apps/starvation_demo/
apps/livelock_demo/
```

The applications demonstrate three different concurrency failure modes.

### Deadlock Demo

Demonstrates:

```text
Circular wait
```

and its solution:

```text
Consistent lock ordering
```

### Starvation Demo

Demonstrates:

```text
Unfair resource access
```

between a greedy thread and a victim thread.

### Livelock Demo

Demonstrates:

```text
Active threads without useful progress
```

caused by both threads repeatedly reacting to each other.

---

## 17. Build and Run

Configure the project:

```bash
cmake -S . -B build
```

Build the deadlock demo:

```bash
cmake --build build --target deadlock_demo
```

Run:

```bash
./build/apps/deadlock_demo/deadlock_demo
```

Build the starvation demo:

```bash
cmake --build build --target starvation_demo
```

Run:

```bash
./build/apps/starvation_demo/starvation_demo
```

Build the livelock demo:

```bash
cmake --build build --target livelock_demo
```

Run:

```bash
./build/apps/livelock_demo/livelock_demo
```

---

## 18. Key Takeaways

The most important concepts from this lesson are:

```text
Deadlock
    -> Threads wait forever for each other.

Starvation
    -> One thread receives insufficient access to a resource.

Livelock
    -> Threads remain active but useful progress does not occur.
```

For multiple mutexes:

```text
Always define and follow a consistent lock acquisition order.
```

For shared resources:

```text
Resource access should not only be thread-safe.
It should also allow reasonable system progress.
```

Correct synchronization therefore requires more than simply adding mutexes.

The design must also consider:

```text
Progress
Fairness
Lock ordering
Scheduling
Resource ownership
Retry behavior
```


