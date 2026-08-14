# Lesson 009 - POSIX Semaphores

## 1. Objective

The purpose of this lesson is to understand POSIX semaphores and how they can be used for thread synchronization in Linux.

The lesson covers:

- Semaphore fundamentals
- Counting semaphore behavior
- `sem_init()`
- `sem_wait()`
- `sem_post()`
- `sem_destroy()`
- Blocking behavior when no semaphore token is available
- Accumulation of semaphore posts
- Differences between semaphores, mutexes, and condition variables
- Internal Linux behavior at a conceptual level
- A practical Embedded Linux synchronization example

The practical application used in this lesson is:

```text
apps/semaphore_demo
```

---

## 2. What Is a Semaphore?

A semaphore can be viewed as a synchronization-safe counter representing available resources or pending events.

For example:

```text
Semaphore value = 3
```

can represent three available resources or three pending events.

A successful `sem_wait()` consumes one semaphore token:

```text
3 -> 2
```

A `sem_post()` adds one token:

```text
2 -> 3
```

If the semaphore value is zero, a thread calling `sem_wait()` may block until another thread posts a token.

---

## 3. POSIX Semaphore Type

POSIX semaphores are provided through:

```c
#include <semaphore.h>
```

The semaphore object type is:

```c
sem_t
```

Example:

```c
static sem_t eventSemaphore;
```

---

## 4. Basic API

The main POSIX semaphore functions used in this lesson are:

```c
sem_init()
sem_wait()
sem_post()
sem_destroy()
```

Their basic responsibilities are:

| Function | Purpose |
|---|---|
| `sem_init()` | Initialize a semaphore |
| `sem_wait()` | Consume a token or block if none is available |
| `sem_post()` | Add a token and potentially wake a waiting thread |
| `sem_destroy()` | Destroy an unused semaphore |

---

## 5. Semaphore Initialization

The practical application initializes the semaphore with:

```c
sem_init(&eventSemaphore, 0, 0);
```

The parameters are:

```text
&eventSemaphore
    |
    +--> Semaphore object

0
    |
    +--> Shared between threads of the same process

0
    |
    +--> Initial semaphore value
```

The initial state is therefore:

```text
eventSemaphore = 0
```

No events are initially available.

---

## 6. `sem_post()`

The main thread produces events using:

```c
sem_post(&eventSemaphore);
```

Conceptually, each call adds one token:

```text
0 -> 1
1 -> 2
2 -> 3
```

This is one of the most important properties of a semaphore:

> Semaphore posts can accumulate even if no thread is currently waiting.

In the practical application, the main thread posts three events before the worker thread is created.

```c
for (int i = 0; i < 3; ++i)
{
    sem_post(&eventSemaphore);
}
```

After these calls:

```text
Semaphore value = 3
```

The worker thread does not need to exist yet for these tokens to be stored.

---

## 7. `sem_wait()`

The worker consumes events using:

```c
sem_wait(&eventSemaphore);
```

If the semaphore value is greater than zero, one token is consumed and the thread continues immediately.

Example:

```text
Before sem_wait(): 3
After sem_wait():  2
```

The first three waits in the practical application therefore behave as:

```text
3 -> 2
2 -> 1
1 -> 0
```

No blocking is required.

---

## 8. Blocking When the Semaphore Reaches Zero

After the first three events are consumed:

```text
Semaphore = 0
```

The worker performs a fourth:

```c
sem_wait(&eventSemaphore);
```

No token is available, so the worker blocks.

Conceptually:

```text
WORKER
  |
  v
sem_wait()
  |
  v
Semaphore == 0
  |
  v
BLOCKED / SLEEPING
```

The worker remains blocked until another thread calls:

```c
sem_post(&eventSemaphore);
```

---

## 9. Practical Application Flow

The main thread posts three events before creating the worker:

```text
MAIN

sem_post()
    |
    v
Semaphore = 1

sem_post()
    |
    v
Semaphore = 2

sem_post()
    |
    v
Semaphore = 3
```

The worker is then created.

It consumes the stored events:

```text
WORKER

sem_wait()
3 -> 2

sem_wait()
2 -> 1

sem_wait()
1 -> 0
```

The worker then waits for the fourth event:

```text
sem_wait()
    |
Semaphore = 0
    |
    v
BLOCK
```

The main thread later posts another event:

```text
MAIN

sem_post()
    |
    v
new token available
    |
    v
wake worker
```

The worker consumes the token and continues.

---

## 10. Practical Output

A typical execution is:

```text
[MAIN] Posting three events before worker starts.
[MAIN] Event 1 posted.
[MAIN] Event 2 posted.
[MAIN] Event 3 posted.
[MAIN] Creating worker thread.
[WORKER] Thread started.
[WORKER] Waiting for event 1...
[WORKER] Event 1 received.
[WORKER] Waiting for event 2...
[WORKER] Event 2 received.
[WORKER] Waiting for event 3...
[WORKER] Event 3 received.
[WORKER] Waiting for event 4...
[MAIN] Posting fourth event.
[WORKER] Event 4 received.
[WORKER] All events processed.
[MAIN] Program completed.
```

The important observation is:

```text
The first three events were posted before the worker existed,
but they were not lost.
```

---

## 11. Semaphore vs Condition Variable

A condition variable and a semaphore can both be used to wake waiting threads, but their semantics are different.

### Condition Variable

A condition variable is primarily a notification mechanism.

The application state is stored separately.

For example:

```c
int dataReady;
pthread_cond_t condition;
```

The condition variable does not behave as an application-level event counter.

A notification sent when no thread is waiting is not accumulated as a pending event count.

### Semaphore

A semaphore maintains a count.

For example:

```text
sem_post()
sem_post()
sem_post()

Semaphore = 3
```

A worker arriving later can consume all three tokens.

This is why semaphores are useful when events or resources need to be counted.

---

## 12. Semaphore vs Mutex

A binary semaphore can sometimes look similar to a mutex because both may conceptually have two states.

However, they serve different purposes.

### Mutex

A mutex protects a shared resource and has an ownership concept.

Typical use:

```text
Thread A
    |
mutex lock
    |
access shared data
    |
mutex unlock
```

The thread that locks the mutex is normally responsible for unlocking it.

### Semaphore

A semaphore represents availability or event count.

One thread can wait:

```c
sem_wait(&eventSemaphore);
```

while another thread posts:

```c
sem_post(&eventSemaphore);
```

This is normal semaphore usage.

Therefore:

```text
Mutex
    -> ownership and mutual exclusion

Semaphore
    -> count, availability, or event signaling
```

A binary semaphore should not automatically be treated as equivalent to a mutex.

---

## 13. Counting Semaphore

A counting semaphore can represent multiple available resources.

For example, suppose a system contains three reusable buffers:

```text
Buffer 0
Buffer 1
Buffer 2
```

The semaphore can start with:

```text
availableBuffers = 3
```

As threads acquire buffers:

```text
3 -> 2
2 -> 1
1 -> 0
```

A fourth thread must wait.

When a buffer is released:

```text
0 -> 1
```

one waiting thread can continue.

This pattern is useful for:

- Buffer pools
- Resource pools
- Worker pools
- Connection pools
- Producer-consumer systems

---

## 14. Queue Example

A semaphore is often combined with a queue.

Suppose a producer inserts messages into a queue:

```c
queuePush(message);
sem_post(&messagesAvailable);
```

The consumer waits for available messages:

```c
sem_wait(&messagesAvailable);
message = queuePop();
```

The semaphore can represent:

```text
Number of messages currently available for processing
```

For example:

```text
messagesAvailable = 4
```

means that four pending items can be consumed.

---

## 15. Semaphore Does Not Automatically Protect Shared Memory

A semaphore counting queue items does not necessarily protect the queue's internal memory structure.

If multiple threads modify the same queue, a mutex may still be required.

Producer:

```c
pthread_mutex_lock(&queueMutex);

queuePush(message);

pthread_mutex_unlock(&queueMutex);

sem_post(&messagesAvailable);
```

Consumer:

```c
sem_wait(&messagesAvailable);

pthread_mutex_lock(&queueMutex);

message = queuePop();

pthread_mutex_unlock(&queueMutex);
```

The responsibilities are different:

```text
Mutex
    |
    +--> Protect queue memory consistency

Semaphore
    |
    +--> Represent number of available messages
```

---

## 16. Why Semaphore Posts Are Not Lost

A semaphore does not store a history of signals.

Instead, it stores an available-token count.

For example:

```text
Initial count = 0

sem_post()
sem_post()
sem_post()

Final count = 3
```

The semaphore does not need to remember three separate wake-up messages.

It only needs to represent:

```text
Three tokens are available.
```

Later:

```text
sem_wait() -> count 2
sem_wait() -> count 1
sem_wait() -> count 0
```

Each wait consumes one stored token.

A useful mental model is:

> `sem_post()` produces a token, and `sem_wait()` consumes a token.

---

## 17. Internal Linux Behavior

From the application point of view, the semaphore is accessed through:

```c
sem_wait()
sem_post()
```

Internally, the implementation can use atomic operations to update the semaphore count safely.

Conceptually, `sem_post()` performs:

```text
atomically add one token
        |
        v
is a thread waiting?
        |
     +--+--+
     |     |
    no    yes
     |     |
 return   wake waiter
```

If a token is already available, `sem_wait()` can often complete using a fast userspace path:

```text
Semaphore = 3
     |
sem_wait()
     |
atomic decrement
     |
3 -> 2
     |
return
```

If the semaphore value is zero, the implementation may need kernel support to block the thread.

On Linux, this kind of wait/wake behavior is commonly implemented using futex-based mechanisms.

Conceptually:

```text
sem_wait()
    |
Semaphore == 0
    |
    v
kernel-assisted wait
    |
    v
thread sleeps
```

Later:

```text
sem_post()
    |
increment count
    |
wake waiter
    |
    v
thread becomes runnable
```

This creates an important performance model:

```text
FAST PATH
    |
token available
    |
userspace atomic operation
    |
continue


SLOW PATH
    |
no token available
    |
kernel-assisted wait
    |
sleep
```

---

## 18. Why Concurrent Posts Do Not Lose Counts

A normal unsynchronized increment such as:

```c
counter++;
```

can suffer from a race condition.

Two threads could both read the same old value and overwrite each other's updates.

Semaphore implementations avoid this by using synchronization-safe atomic operations.

Conceptually:

```text
Initial semaphore = 0

Thread A sem_post()
    |
    +--> atomic update -> 1

Thread B sem_post()
    |
    +--> atomic update -> 2
```

The final value correctly represents both posts.

---

## 19. `sem_trywait()`

Sometimes a thread should not block.

POSIX provides:

```c
sem_trywait(&semaphore);
```

Conceptually:

```text
Token available?
      |
   +--+--+
   |     |
  yes    no
   |     |
consume  return immediately
token    with failure
```

This is useful in non-blocking designs.

---

## 20. `sem_timedwait()`

A thread may also need to wait only for a limited time.

POSIX provides:

```c
sem_timedwait()
```

This can be useful in Embedded Linux systems where a thread should not block indefinitely.

Example design:

```text
Wait for sensor data
        |
        v
 maximum timeout
        |
    +---+---+
    |       |
   data   timeout
    |       |
 process   fault handling
```

---

## 21. Embedded Linux Example

A realistic Embedded Linux system may contain:

```text
CAN RX Thread
Processing Thread
```

The CAN receive thread receives messages and inserts them into a queue:

```text
CAN frame received
       |
       v
queuePush()
       |
       v
sem_post()
```

The processing thread waits efficiently:

```text
sem_wait()
    |
    v
queuePop()
    |
    v
process CAN message
```

If multiple CAN frames arrive before the processing thread executes:

```text
Frame A -> sem_post()
Frame B -> sem_post()
Frame C -> sem_post()
```

the semaphore count can represent three pending messages.

The consumer can later process three queue entries without losing the event count.

---

## 22. Relation to RTOS Systems

Semaphore concepts are also widely used in RTOS environments.

Common mechanisms include:

```text
Binary Semaphore
Counting Semaphore
Mutex
Queue
Task Notification
```

The same design principle appears frequently:

```text
Producer / ISR / I/O source
          |
          v
     give/post event
          |
          v
      semaphore
          |
          v
     waiting task
          |
          v
        wakes
```

Understanding POSIX semaphores therefore helps build a transferable synchronization model for both Embedded Linux and RTOS development.

---

## 23. Lifecycle

The semaphore lifecycle in the practical application is:

```text
sem_init()
    |
    v
sem_post()
sem_post()
sem_post()
    |
    v
pthread_create()
    |
    v
sem_wait()
sem_wait()
sem_wait()
sem_wait()
    |
    v
sem_post()
    |
    v
pthread_join()
    |
    v
sem_destroy()
```

The semaphore is destroyed only after all threads have finished using it.

---

## 24. Practical Application

Application:

```text
apps/semaphore_demo
```

Configure:

```bash
cmake -S . -B build
```

Build:

```bash
cmake --build build --target semaphore_demo
```

Run:

```bash
./build/apps/semaphore_demo/semaphore_demo
```

---

## 25. Key Takeaways

1. A semaphore behaves conceptually like a synchronization-safe token counter.
2. `sem_post()` adds one token.
3. `sem_wait()` consumes one available token.
4. If no token is available, `sem_wait()` can block the calling thread.
5. Semaphore posts can accumulate even if no thread is currently waiting.
6. A semaphore does not store signal history; it stores an available-token count.
7. Counting semaphores can represent multiple resources or pending events.
8. A semaphore is different from a mutex because semaphore usage is not based on lock ownership.
9. A semaphore is different from a condition variable because semaphore counts can accumulate.
10. A semaphore does not automatically protect the memory structure associated with the resource; a mutex may still be required.
11. Linux semaphore implementations can use atomic userspace operations for the fast path and kernel-assisted wait/wake behavior when blocking is required.
12. Producer-consumer systems, queues, resource pools, and event-driven Embedded Linux designs are common semaphore use cases.
