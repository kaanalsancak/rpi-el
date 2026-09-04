# Lesson 011 – Read-Write Locks and Barriers

## 1. Objective

The objective of this lesson is to understand two POSIX synchronization mechanisms used in multithreaded Linux applications:

- Read-Write Locks
- Barriers

These mechanisms solve different synchronization problems.

A read-write lock protects shared data while allowing multiple readers to access the data concurrently.

A barrier synchronizes the execution phases of multiple threads.

---

## 2. Why a Mutex Is Not Always the Best Choice

A mutex provides exclusive access to a protected resource.

```c
pthread_mutex_lock(&mutex);

/* Critical section */

pthread_mutex_unlock(&mutex);
```

Only one thread can own the mutex at a time.

This behavior is necessary when shared data is being modified.

However, if multiple threads only need to read the same data, exclusive access may unnecessarily reduce concurrency.

Consider the following threads:

```text
Control Thread    -> READ
Telemetry Thread  -> READ
Logger Thread     -> READ
Monitor Thread    -> READ
```

If a mutex is used, the threads must access the shared data one at a time.

```text
Reader 1 -> ACCESS

Reader 2 -> WAIT
Reader 3 -> WAIT
Reader 4 -> WAIT
```

In many cases, multiple readers can safely access the same data simultaneously.

This is the problem addressed by a read-write lock.

---

# Part I – Read-Write Locks

## 3. Read-Write Lock

The POSIX read-write lock type is:

```c
pthread_rwlock_t
```

A read-write lock provides two different locking operations:

```text
Read Lock
Write Lock
```

The fundamental rules are:

```text
READ  + READ  -> Allowed
READ  + WRITE -> Not allowed
WRITE + WRITE -> Not allowed
```

This means several readers may access the protected data at the same time.

A writer, however, requires exclusive access.

---

## 4. Read Lock

A thread that only reads shared data can request a read lock:

```c
pthread_rwlock_rdlock(&sharedDataLock);
```

For example:

```text
Reader 1 ----\
Reader 2 -----+--> Shared Data
Reader 3 ----/
```

All three readers may hold the read lock simultaneously.

After reading the protected data:

```c
pthread_rwlock_unlock(&sharedDataLock);
```

releases the lock.

---

## 5. Write Lock

A thread that modifies the protected data must request a write lock:

```c
pthread_rwlock_wrlock(&sharedDataLock);
```

A write lock is exclusive.

While the writer owns the lock:

```text
Writer
  |
  v
Shared Data
```

no reader or other writer may enter the protected section.

After the update:

```c
pthread_rwlock_unlock(&sharedDataLock);
```

releases the lock.

---

## 6. Important Read-Write Lock Rule

A read lock must not be used when modifying protected data.

Correct:

```c
pthread_rwlock_rdlock(&sharedDataLock);

localValue = sharedValue;

pthread_rwlock_unlock(&sharedDataLock);
```

Modification requires a write lock:

```c
pthread_rwlock_wrlock(&sharedDataLock);

sharedValue = newValue;

pthread_rwlock_unlock(&sharedDataLock);
```

The programmer is responsible for selecting the correct lock type.

---

## 7. Practical Read-Write Lock Application

The practical application contains:

```text
3 Reader Threads
1 Writer Thread
```

The readers repeatedly read:

```c
sharedValue
```

while the writer modifies the value.

The protected objects are:

```c
static pthread_rwlock_t sharedDataLock = PTHREAD_RWLOCK_INITIALIZER;
static int sharedValue = 0;
```

Readers acquire the lock using:

```c
pthread_rwlock_rdlock(&sharedDataLock);
```

The writer uses:

```c
pthread_rwlock_wrlock(&sharedDataLock);
```

---

## 8. Reader Concurrency Observation

During the Raspberry Pi test, several readers successfully acquired the read lock before any of them released it.

Example observed behavior:

```text
[READER 1] Read lock acquired. sharedValue = 0
[READER 3] Read lock acquired. sharedValue = 0
[READER 2] Read lock acquired. sharedValue = 0
```

This demonstrates:

```text
Reader 1 ─┐
Reader 2 ─┼──> Shared Data
Reader 3 ─┘
```

Multiple read locks can exist simultaneously.

This would not occur with a normal mutex because a mutex provides only one owner at a time.

---

## 9. Writer Exclusivity Observation

When the writer attempted to acquire the write lock while readers were active:

```text
[WRITER] Waiting for write lock.
```

the writer had to wait.

Only after all current readers released their locks did the writer acquire exclusive access:

```text
[WRITER] Write lock acquired. sharedValue updated to 1
```

While the writer held the lock, new readers waited:

```text
[READER 1] Waiting for read lock.
[READER 3] Waiting for read lock.
[READER 2] Waiting for read lock.
```

After the writer released the lock:

```text
[WRITER] Releasing write lock.
```

the readers could acquire read locks again.

This confirms:

```text
READ + READ   -> Allowed

READ + WRITE  -> Blocked

WRITE + WRITE -> Blocked
```

---

## 10. Final Read-Write Lock Result

The writer updated the value three times.

The final result was:

```text
[MAIN] Final sharedValue = 3
```

This was the expected result.

---

## 11. When Is a Read-Write Lock Useful?

A read-write lock is particularly useful when:

```text
Reads are frequent
Writes are relatively rare
Several threads access the same data
Readers can safely execute concurrently
```

An example is system configuration.

```text
Control Thread    -> READ
Telemetry Thread  -> READ
Logger Thread     -> READ
Monitor Thread    -> READ

Configuration Thread -> WRITE occasionally
```

The readers can execute concurrently while configuration updates receive exclusive access.

---

## 12. Read-Write Lock vs Mutex

A read-write lock is not automatically better than a mutex.

A read-write lock requires more internal state management because the implementation must track:

- active readers,
- active writers,
- waiting readers,
- waiting writers.

Therefore, lock and unlock operations may have more overhead than a simple mutex.

If the critical section is very short or writes are frequent, a mutex may be simpler and faster.

General guideline:

```text
Mostly reads + occasional writes
        |
        v
Consider RW Lock
```

For:

```text
Frequent writes
Low contention
Very short critical section
        |
        v
Mutex may be preferable
```

Synchronization primitives should be selected according to the problem rather than according to their complexity.

---

## 13. Consistent Data Snapshots

Read-write locks can also protect a group of related values.

Consider:

```c
struct Config
{
    float kp;
    float ki;
    float maxSpeed;
    float maxCurrent;
};
```

Without proper synchronization, a reader might observe:

```text
kp       -> old configuration
ki       -> new configuration
maxSpeed -> new configuration
```

This creates an inconsistent snapshot.

A read lock allows the complete configuration to be read while preventing a writer from changing it in the middle of the operation.

```c
pthread_rwlock_rdlock(&configLock);

/* Read complete configuration */

pthread_rwlock_unlock(&configLock);
```

---

## 14. Read-Write Lock Starvation

Read-write locks do not automatically guarantee fairness.

For example, if readers continuously enter the protected section:

```text
Reader 1
Reader 2
Reader 3
Reader 4
Reader 5
...
```

a waiting writer may experience a long delay.

This is called:

```text
Writer Starvation
```

Depending on the implementation and scheduling policy, reader starvation may also be possible.

Therefore:

```text
Thread-safe
```

does not necessarily mean:

```text
Fair
```

or:

```text
Deterministic
```

---

## 15. Real-Time Considerations

Using:

```c
pthread_rwlock_t
```

does not guarantee a maximum lock acquisition time.

Timing is also affected by:

- scheduler behavior,
- thread priorities,
- CPU load,
- contention,
- critical section duration.

Therefore, if a requirement says:

```text
The writer must obtain access within 2 ms.
```

using an RW lock alone does not prove that requirement.

Synchronization correctness and real-time determinism are separate concerns.

---

# Part II – Barriers

## 16. Barrier

A barrier solves a different problem.

A barrier does not protect shared data.

Instead, it creates a synchronization point.

Conceptually:

```text
Thread 1 -----\
Thread 2 ------> BARRIER ---> Continue
Thread 3 -----/
```

No participant passes the barrier until the configured number of participants has reached it.

---

## 17. Barrier Example

Consider three initialization threads:

```text
Thread 1 -> Sensor initialization
Thread 2 -> Communication initialization
Thread 3 -> Control initialization
```

The system must not enter the next phase until all three initialization operations are complete.

The synchronization can be represented as:

```text
Sensor Init --------\
Communication Init ---> BARRIER ---> RUN
Control Init -------/
```

The faster threads wait for the slowest thread.

---

## 18. Initializing a Barrier

A POSIX barrier can be initialized using:

```c
pthread_barrier_init(&phaseBarrier, NULL, WORKER_COUNT);
```

For:

```c
#define WORKER_COUNT 3
```

the barrier opens only after three participants execute:

```c
pthread_barrier_wait(&phaseBarrier);
```

---

## 19. Barrier Count

The barrier count must match the number of participants.

For three worker threads:

```text
Count = 3
```

Behavior:

```text
Worker 1 arrives -> 1 / 3 -> WAIT

Worker 2 arrives -> 2 / 3 -> WAIT

Worker 3 arrives -> 3 / 3 -> RELEASE
```

After the final participant arrives:

```text
Worker 1 -> Continue
Worker 2 -> Continue
Worker 3 -> Continue
```

---

## 20. Practical Barrier Application

The practical application contains three worker threads.

Each worker performs Phase 1 for a different amount of time.

```text
Worker 1 -> 500 ms
Worker 2 -> 1000 ms
Worker 3 -> 1500 ms
```

Each worker then executes:

```c
pthread_barrier_wait(&phaseBarrier);
```

Expected execution:

```text
Worker 1 finishes Phase 1
    |
    +--> waits at barrier

Worker 2 finishes Phase 1
    |
    +--> waits at barrier

Worker 3 finishes Phase 1
    |
    +--> barrier opens
```

Only after Worker 3 reaches the barrier can all workers start Phase 2.

---

## 21. Barrier Timing

Consider the following processing times:

```text
Thread A = 2 ms
Thread B = 5 ms
Thread C = 12 ms
```

All three synchronize at a barrier.

Thread A reaches the barrier at approximately:

```text
2 ms
```

Thread B reaches it at:

```text
5 ms
```

Thread C reaches it at:

```text
12 ms
```

The barrier cannot open until approximately:

```text
12 ms
```

Therefore:

```text
Thread A waits approximately 10 ms
Thread B waits approximately 7 ms
Thread C waits approximately 0 ms
```

This demonstrates an important characteristic of barriers:

> The execution phase is often limited by the slowest participant.

---

## 22. Cyclic Barrier Behavior

POSIX barriers can be reused.

For example:

```text
Cycle 1

Thread A ----\
Thread B -----> BARRIER
Thread C ----/

Cycle 2

Thread A ----\
Thread B -----> BARRIER
Thread C ----/
```

This makes barriers useful for algorithms divided into repeated processing phases.

---

## 23. Special Barrier Return Value

When a barrier opens, one participant receives:

```c
PTHREAD_BARRIER_SERIAL_THREAD
```

while the other successful participants receive:

```text
0
```

Example:

```c
int result = pthread_barrier_wait(&phaseBarrier);

if (result == PTHREAD_BARRIER_SERIAL_THREAD)
{
    /* Exactly one participant executes this section. */
}
```

This can be useful when exactly one thread must perform an operation after all participants complete a phase.

For example:

```text
Worker 1 ----\
Worker 2 -----> BARRIER ---> One worker combines results
Worker 3 ----/
```

The specific thread receiving `PTHREAD_BARRIER_SERIAL_THREAD` should not normally be assumed in advance.

---

## 24. Barrier Count Errors

Incorrect barrier counts can prevent correct execution.

Consider:

```text
3 worker threads
```

but:

```c
pthread_barrier_init(&barrier, NULL, 4);
```

The result is:

```text
Worker 1 -> 1 / 4
Worker 2 -> 2 / 4
Worker 3 -> 3 / 4

Fourth participant never arrives.
```

All workers remain blocked.

Therefore, barrier participant count must be carefully designed.

---

## 25. Main Thread as a Barrier Participant

The main thread can also participate in a barrier.

For example:

```text
Worker 1
Worker 2
Main Thread
```

If all three execute:

```c
pthread_barrier_wait()
```

then the barrier count should be:

```text
3
```

The count represents participants, not necessarily worker threads.

---

## 26. Barrier vs Mutex

A mutex protects shared data.

Its rule is:

```text
Only one thread enters the protected section.
```

A barrier synchronizes execution.

Its rule is:

```text
Nobody continues until everybody arrives.
```

Therefore:

```text
Mutex   -> Resource protection

Barrier -> Execution synchronization
```

A barrier does not make operations such as:

```c
sharedCounter++;
```

thread-safe.

After a barrier opens, multiple threads may execute the increment simultaneously.

A mutex or another appropriate synchronization mechanism would still be required.

---

## 27. Barrier vs Condition Variable

A condition variable waits for a state condition.

Example:

```text
Wait until queue is not empty.
```

A barrier waits for participants.

Example:

```text
Wait until all three workers finish Phase 1.
```

Therefore:

```text
Condition Variable
    -> Wait for state/event condition

Barrier
    -> Wait for execution group
```

---

# Part III – Synchronization Primitive Selection

## 28. Selecting the Correct Primitive

At this stage, the project has covered several synchronization mechanisms.

| Primitive | Main Purpose |
|---|---|
| Mutex | Exclusive shared-resource protection |
| Read-Write Lock | Multiple concurrent readers or one exclusive writer |
| Semaphore | Resource counting or signaling |
| Condition Variable | Wait for a state change |
| Barrier | Wait for all participants to reach an execution point |

The important engineering skill is not memorizing APIs.

The important question is:

```text
What synchronization problem am I trying to solve?
```

---

## 29. Example Decisions

### Shared SPI Peripheral

Requirement:

```text
Only one thread may access the SPI peripheral at a time.
```

Suitable primitive:

```text
Mutex
```

---

### Shared Configuration

Requirement:

```text
Several threads frequently read configuration.
One thread rarely modifies configuration.
```

Suitable candidate:

```text
Read-Write Lock
```

---

### Sensor Data Arrival

Requirement:

```text
Control thread should sleep until new sensor data is available.
```

Suitable primitive:

```text
Condition Variable
```

---

### Limited Resource Pool

Requirement:

```text
Only three identical resources are available.
```

Suitable primitive:

```text
Semaphore
```

---

### Processing Phases

Requirement:

```text
All workers must complete Phase 1 before Phase 2 begins.
```

Suitable primitive:

```text
Barrier
```

---

## 30. Embedded Linux Considerations

In Embedded Linux applications, synchronization may be required around:

- shared configuration,
- communication buffers,
- hardware interfaces,
- telemetry state,
- control state,
- device access,
- logging resources,
- processing pipelines.

Correct synchronization design should consider:

```text
Correctness
Concurrency
Fairness
Latency
Scheduling
Determinism
Maintainability
```

Adding more locks does not automatically create a better system.

The synchronization architecture should remain as simple as possible while satisfying the system requirements.

---

## 31. Practical Applications

Applications created during this lesson:

```text
apps/rwlock_demo/
apps/barrier_demo/
```

### Read-Write Lock Demo

Demonstrates:

- multiple concurrent readers,
- exclusive writer access,
- readers waiting while the writer owns the lock,
- correct update of shared data.

### Barrier Demo

Demonstrates:

- synchronization between worker threads,
- different worker execution durations,
- waiting for the slowest participant,
- transition from Phase 1 to Phase 2 only after all workers arrive.

---

## 32. Build and Run

Configure the project:

```bash
cmake -S . -B build
```

Build the read-write lock application:

```bash
cmake --build build --target rwlock_demo
```

Run:

```bash
./build/apps/rwlock_demo/rwlock_demo
```

Build the barrier application:

```bash
cmake --build build --target barrier_demo
```

Run:

```bash
./build/apps/barrier_demo/barrier_demo
```

---

## 33. Key Takeaways

Read-write lock:

```text
Many readers
OR
one writer
```

It is particularly useful for read-heavy shared data.

However:

```text
RW Lock != automatically faster than Mutex
```

Performance, fairness, and timing must still be considered.

Barrier:

```text
Everybody arrives
        |
        v
Everybody continues
```

A barrier synchronizes execution phases but does not protect shared data.

The most important lesson is:

> Select synchronization primitives according to the problem being solved.

Correct concurrent software requires not only race-free access but also reasonable progress, timing behavior, fairness, and understandable synchronization rules.