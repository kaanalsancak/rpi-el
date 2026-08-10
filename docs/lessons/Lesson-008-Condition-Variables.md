# Lesson 008 - POSIX Condition Variables

## 1. Objective

The purpose of this lesson is to understand how POSIX condition variables can be used to efficiently synchronize threads without busy waiting.

The practical application demonstrates:

- Waiting for a shared condition
- Sleeping without continuously polling
- Using a mutex together with a condition variable
- Signaling a waiting thread
- Rechecking the condition after waking up
- Safely accessing shared data

The application used in this lesson is:

```text
apps/condition_variable_demo
```

---

## 2. The Problem with Busy Waiting

A thread may need to wait until another thread produces new data.

A simple but inefficient implementation could be:

```c
while (dataReady == 0)
{
}
```

This is called **busy waiting**.

The thread continuously checks the shared variable and may consume CPU time even though it has no useful work to perform.

A better design is:

```text
No data
   |
   v
Thread sleeps
   |
   | condition signal
   v
Thread wakes
   |
   v
Process data
```

POSIX condition variables provide this behavior.

---

## 3. Condition Variable

A POSIX condition variable is represented by:

```c
pthread_cond_t
```

In this application:

```c
static pthread_cond_t dataCondition =
    PTHREAD_COND_INITIALIZER;
```

The condition variable itself does not store the application state.

The actual shared state is:

```c
static int dataReady = 0;
```

The roles are different:

```text
dataReady
    |
    +--> Actual shared state

dataCondition
    |
    +--> Notification mechanism
```

The condition variable tells waiting threads that the shared state may have changed.

---

## 4. Shared State and Mutex

The application contains:

```c
static pthread_mutex_t dataMutex =
    PTHREAD_MUTEX_INITIALIZER;

static pthread_cond_t dataCondition =
    PTHREAD_COND_INITIALIZER;

static int sharedData = 0;
static int dataReady = 0;
```

The mutex protects access to:

```text
sharedData
dataReady
```

The condition variable allows the worker thread to sleep until the state changes.

The relationship is:

```text
             Shared State
                 |
          +------+------+
          |             |
      sharedData     dataReady
          |
      protected by
          |
        Mutex
          |
      Condition
      Variable
```

---

## 5. Worker Thread

The worker first locks the mutex:

```c
pthread_mutex_lock(&dataMutex);
```

It then checks whether the required condition is true:

```c
while (dataReady == 0)
{
    pthread_cond_wait(&dataCondition,
                      &dataMutex);
}
```

The condition is checked using a `while` loop instead of an `if` statement.

A wake-up does not guarantee that the expected condition is true.

Therefore, the predicate must always be checked again.

---

## 6. `pthread_cond_wait()`

The most important operation in this lesson is:

```c
pthread_cond_wait(&dataCondition,
                  &dataMutex);
```

Conceptually, it performs the following sequence:

```text
Worker owns mutex
       |
       v
Condition is false
       |
       v
pthread_cond_wait()
       |
       +--> Release mutex
       |
       +--> Sleep
       |
       | signal arrives
       v
Wake up
       |
       +--> Reacquire mutex
       |
       v
Return from pthread_cond_wait()
```

The mutex must be released while the worker is sleeping.

Otherwise, the producer would not be able to acquire the mutex and change the shared state.

---

## 7. Why Unlock and Wait Must Not Be Separate

The program should not manually perform:

```c
pthread_mutex_unlock(&dataMutex);
pthread_cond_wait(&dataCondition,
                  &dataMutex);
```

There would be a timing window between these operations.

For example:

```text
Worker                       Main

unlock mutex

                             lock mutex
                             dataReady = 1
                             signal
                             unlock mutex

wait
```

The signal could occur before the worker actually starts waiting.

The worker could then miss the notification.

`pthread_cond_wait()` is designed to safely coordinate releasing the mutex and entering the wait state.

---

## 8. Main Thread as Producer

The main thread simulates data preparation using:

```c
sleep(3);
```

During this period, the worker is sleeping on the condition variable rather than continuously polling.

The main thread then locks the mutex:

```c
pthread_mutex_lock(&dataMutex);
```

and modifies the shared state:

```c
sharedData = 42;
dataReady = 1;
```

It then notifies the worker:

```c
pthread_cond_signal(&dataCondition);
```

Finally, it releases the mutex:

```c
pthread_mutex_unlock(&dataMutex);
```

---

## 9. `pthread_cond_signal()`

The function:

```c
pthread_cond_signal(&dataCondition);
```

notifies a thread waiting on the condition variable.

The signal does not directly transfer the value `42`.

The data is stored separately in:

```c
sharedData
```

The signal only indicates:

```text
The shared state may have changed.
Wake up and check it again.
```

---

## 10. Why the Worker Rechecks the Condition

After waking up, the worker prints:

```text
[WORKER] Woke up. Checking condition again...
```

The worker then evaluates:

```c
while (dataReady == 0)
```

again.

This is necessary because condition variable wake-ups must always be associated with a shared predicate.

The correct pattern is:

```c
pthread_mutex_lock(&mutex);

while (!predicate)
{
    pthread_cond_wait(&condition,
                      &mutex);
}

/* Predicate is true */

pthread_mutex_unlock(&mutex);
```

---

## 11. Practical Execution

The application produced the following output:

```text
[MAIN] Creating worker thread.
[MAIN] Simulating data preparation...
[WORKER] Thread started.
[WORKER] Data is not ready. Going to sleep...
[MAIN] Data is ready: 42
[MAIN] Sending condition signal.
[WORKER] Woke up. Checking condition again...
[WORKER] Data received: 42
[WORKER] Thread completed.
[MAIN] Program completed.
```

The output confirms that:

1. The worker starts before the data is ready.
2. The worker enters the condition wait.
3. The main thread prepares the shared data.
4. The main thread signals the condition variable.
5. The worker wakes up.
6. The worker checks the predicate again.
7. The worker reads the shared value.
8. Both threads terminate normally.

---

## 12. Execution Timeline

The execution can be represented as:

```text
WORKER                         MAIN
  |                             |
  |                       create worker
  |                             |
start                           |
  |                             |
mutex lock                      |
  |                             |
dataReady == 0                  |
  |                             |
cond_wait                       |
  |                             |
mutex released                  |
SLEEP                           |
                                |
                           sleep(3)
                                |
                           mutex lock
                                |
                           sharedData = 42
                           dataReady = 1
                                |
                           cond_signal
                                |
                           mutex unlock
                                |
WAKE <--------------------------+
  |
mutex reacquired
  |
check dataReady
  |
read sharedData
  |
mutex unlock
  |
finish
```

---

## 13. Condition Variable vs Mutex

A mutex and condition variable solve different problems.

### Mutex

A mutex answers:

```text
Who is allowed to access the shared state now?
```

It provides mutual exclusion.

### Condition Variable

A condition variable answers:

```text
How can a thread efficiently wait until
the shared state changes?
```

They are commonly used together.

---

## 14. Condition Variable vs Busy Waiting

Busy waiting:

```c
while (dataReady == 0)
{
}
```

continuously consumes CPU resources.

Condition-variable waiting:

```c
while (dataReady == 0)
{
    pthread_cond_wait(&dataCondition,
                      &dataMutex);
}
```

allows the thread to sleep until useful work may be available.

This makes condition variables especially useful in Embedded Linux systems where CPU resources may be shared by multiple real-time or background activities.

---

## 15. Embedded Linux Example

A similar design could be used with a UART, CAN, or sensor thread.

For example:

```text
CAN RX Thread
     |
     v
Receive frame
     |
     v
Lock mutex
     |
     v
Update shared data
     |
     v
Set dataReady
     |
     v
Signal condition
     |
     v
Unlock mutex
```

A processing thread can then remain asleep until new data arrives:

```text
Processing Thread
       |
       v
Wait for condition
       |
       v
New CAN data
       |
       v
Wake up
       |
       v
Process data
```

This avoids continuous polling.

---

## 16. Key Takeaways

1. Condition variables allow threads to wait without busy polling.
2. A condition variable does not contain the application state.
3. The actual condition is represented by shared state such as `dataReady`.
4. Shared state should be protected by a mutex.
5. `pthread_cond_wait()` releases the mutex while the thread sleeps.
6. The mutex is reacquired before `pthread_cond_wait()` returns.
7. The predicate must be checked using a `while` loop.
8. `pthread_cond_signal()` wakes a waiting thread but does not transfer application data.
9. The shared state contains the data; the condition variable only provides notification.
10. Mutexes and condition variables solve different synchronization problems and are commonly used together.

---

## 17. Practical Application

Application:

```text
apps/condition_variable_demo
```

Configure:

```bash
cmake -S . -B build
```

Build:

```bash
cmake --build build --target condition_variable_demo
```

Run:

```bash
./build/apps/condition_variable_demo/condition_variable_demo
```
