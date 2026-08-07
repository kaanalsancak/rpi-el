# Lesson 007 - Thread Memory Model

## 1. Objective

The purpose of this lesson is to understand how memory is organized and shared between threads that belong to the same Linux process.

The practical application demonstrates:

* Process IDs and thread IDs
* Shared global variables
* Shared heap memory
* Per-thread stack memory
* Data visibility between threads
* The importance of variable initialization

The application used in this lesson is:

```text
apps/thread_memory_demo
```

---

## 2. Process and Thread Relationship

Threads that belong to the same process share the same virtual address space.

Therefore, they share several process resources, including:

* Program code
* Global variables
* Static variables
* Heap memory
* Open file descriptors

However, each thread has its own execution context, including:

* Thread ID
* Stack
* CPU registers
* Program counter
* Stack pointer
* Scheduling state

A simplified representation is:

```text
                    PROCESS
                       |
        +--------------+--------------+
        |              |              |
       Code           Data           Heap
        |              |              |
        +--------------+--------------+
                       |
                    Shared
                       |
             +---------+---------+
             |                   |
         Main Thread          Worker Thread
             |                   |
          Registers           Registers
             |                   |
             PC                  PC
             |                   |
           Stack               Stack
             |                   |
          Private             Private
```

---

## 3. Application Structure

The application creates one worker thread using POSIX threads.

The process therefore contains:

```text
Main Thread
Worker Thread
```

The main thread creates the worker with:

```c
pthread_create(&worker,
               NULL,
               workerThread,
               heapValue);
```

The main thread then waits for the worker to terminate:

```c
pthread_join(worker, NULL);
```

The application observes three different types of data:

```c
static int globalCounter = 0;
```

A global variable stored in the process data area.

```c
int *heapValue = malloc(sizeof(int));
```

A dynamically allocated object stored on the process heap.

```c
int localCounter = 0;
```

A local automatic variable stored on the current thread's stack.

---

## 4. PID and TID

The program prints both the Process ID and Linux Thread ID.

Example output:

```text
[MAIN] PID: 3228, TID: 3228
[WORKER] PID : 3228 TID : 3229
```

Both threads have the same PID:

```text
MAIN PID   = 3228
WORKER PID = 3228
```

This proves that both execution flows belong to the same process.

Their thread IDs are different:

```text
MAIN TID   = 3228
WORKER TID = 3229
```

This shows that the Linux kernel identifies them as separate execution contexts.

For the initial thread of a Linux process, the PID and TID normally have the same numeric value.

Additional threads receive their own TIDs.

---

## 5. Shared Global Memory

The program prints the address of `globalCounter` from both threads.

Observed output:

```text
[MAIN] Address of globalCounter   : 0x55839b007c
[WORKER] Address of globalCounter : 0x55839b007c
```

Both threads observe exactly the same address.

Therefore:

```text
                globalCounter
                     |
              0x55839b007c
                 /       \
                /         \
          Main Thread   Worker Thread
```

There is only one `globalCounter` object in the process.

When the worker executes:

```c
globalCounter++;
```

the value changes from:

```text
0 -> 1
```

After `pthread_join()`, the main thread also observes:

```text
[MAIN] globalCounter = 1
```

This demonstrates that global data is shared between threads in the same process.

---

## 6. Shared Heap Memory

The main thread allocates memory using:

```c
int *heapValue = malloc(sizeof(int));
```

The allocated object belongs to the process heap.

The address stored in `heapValue` is passed to the worker:

```c
pthread_create(&worker,
               NULL,
               workerThread,
               heapValue);
```

No heap object is copied during this operation.

Both threads receive access to the same dynamically allocated memory.

Observed output:

```text
[MAIN] Address of heapValue   : 0x55a5cc42a0
[WORKER] Address of heapValue : 0x55a5cc42a0
```

The address is identical.

Conceptually:

```text
Main Thread
     |
     |
     +----------+
                |
                v
          +------------+
          | Heap Object|
          | value = 0  |
          +------------+
                ^
                |
     +----------+
     |
Worker Thread
```

The worker executes:

```c
(*heapValue)++;
```

and changes the shared heap object:

```text
0 -> 1
```

The main thread later observes:

```text
[MAIN] heapValue = 1
```

This demonstrates that heap memory belongs to the process address space and can be accessed by multiple threads.

### Important Pointer Detail

The program prints:

```c
(void *)heapValue
```

This is the address of the dynamically allocated heap object.

It is not the address of the local pointer variable itself.

The pointer variable in `main()` and the pointer variable in `workerThread()` are separate local variables stored on separate thread stacks.

If their own addresses were printed using:

```c
&heapValue
```

those addresses would be different.

However, the pointer values stored inside them point to the same shared heap object.

---

## 7. Per-Thread Stack Memory

Both the main thread and worker thread contain a local variable named:

```c
int localCounter = 0;
```

Despite having the same name, these are two independent C objects.

Observed addresses:

```text
[MAIN] Address of localCounter   : 0x7fe639ae5c
[WORKER] Address of localCounter : 0x7fa077e8c4
```

The addresses are different.

Conceptually:

```text
MAIN THREAD STACK

+-------------------------+
| localCounter = 0        |
| 0x7fe639ae5c            |
+-------------------------+


WORKER THREAD STACK

+-------------------------+
| localCounter = 0        |
| 0x7fa077e8c4            |
+-------------------------+
```

The worker executes:

```c
localCounter++;
```

and obtains:

```text
[WORKER] localCounter = 1
```

However, after the worker terminates, the main thread still prints:

```text
[MAIN] localCounter = 0
```

The worker's modification does not affect the main thread's local variable because each thread has its own stack.

---

## 8. Final Experimental Result

The worker modifies all three variables:

```c
globalCounter++;
(*heapValue)++;
localCounter++;
```

Worker output:

```text
[WORKER] globalCounter = 1
[WORKER] heapValue     = 1
[WORKER] localCounter  = 1
```

After `pthread_join()`, the main thread prints:

```text
[MAIN] globalCounter = 1
[MAIN] heapValue     = 1
[MAIN] localCounter  = 0
```

The result can be summarized as:

| Object          | Memory Region | Shared | Worker Modification Visible to Main |
| --------------- | ------------- | -----: | ----------------------------------: |
| `globalCounter` | Global data   |    Yes |                                 Yes |
| `*heapValue`    | Heap          |    Yes |                                 Yes |
| `localCounter`  | Thread stack  |     No |                                  No |

---

## 9. Variable Initialization Issue

During the first execution of the application, the main thread contained:

```c
int localCounter;
```

instead of:

```c
int localCounter = 0;
```

The program produced:

```text
[MAIN] localCounter = 127
```

The value `127` was not produced by the worker thread.

The variable was an uninitialized automatic variable.

Its stack memory contained an indeterminate value, and reading such a variable results in undefined behavior.

The correct declaration is:

```c
int localCounter = 0;
```

After initializing the variable, the expected result was obtained:

```text
[MAIN] localCounter = 0
```

This demonstrates an important C programming rule:

> Automatic local variables are not automatically initialized.

Variables should be explicitly initialized before their values are read.

---

## 10. Why `pthread_join()` Matters

The main thread calls:

```c
pthread_join(worker, NULL);
```

before inspecting the final shared values.

This guarantees that the worker has completed its operations before the main thread reads:

```c
globalCounter
```

and:

```c
*heapValue
```

It also guarantees that the worker no longer uses the heap object before:

```c
free(heapValue);
```

is executed.

The lifetime is therefore:

```text
malloc()
   |
   v
pthread_create()
   |
   v
Worker uses heap memory
   |
   v
pthread_join()
   |
   v
Worker is finished
   |
   v
free()
```

Freeing the memory before the worker has finished could cause the worker to access invalid memory.

---

## 11. Thread Memory Model Summary

Threads share the process address space, but they do not share their execution stacks.

The main observations from this experiment are:

```text
Global data      -> Shared
Static data      -> Shared
Heap             -> Shared
Code             -> Shared

Stack            -> Per thread
Registers        -> Per thread
Program counter  -> Per thread
Thread ID        -> Per thread
```

The central concept is:

> Threads share process resources but maintain independent execution contexts.

This distinction is fundamental to understanding multithreaded Linux applications.

---

## 12. Connection to Synchronization

Shared memory makes communication between threads efficient, but it also introduces concurrency problems.

For example:

```c
globalCounter++;
```

looks like a single C operation, but internally it can involve multiple steps:

```text
LOAD globalCounter
ADD 1
STORE globalCounter
```

If multiple threads execute these operations concurrently, their accesses can overlap.

This can produce a:

```text
Race Condition
```

Therefore, shared data often requires synchronization mechanisms such as:

```text
Mutexes
Semaphores
Condition Variables
Atomic Operations
```

These mechanisms will be studied in later lessons.

---

## 13. Key Takeaways

1. Threads belonging to the same process have the same PID.

2. Each Linux thread has its own TID.

3. Global and static variables are shared between threads.

4. Dynamically allocated heap objects are shared through the process address space.

5. Each thread has its own stack.

6. Local automatic variables normally belong to the stack of the thread executing the function.

7. Two local variables can have the same name while representing completely different objects.

8. A pointer variable and the memory object it points to are different concepts.

9. Automatic variables must be initialized before their values are read.

10. `pthread_join()` can be used to wait for thread completion and correctly manage shared resource lifetime.

11. Shared memory provides efficient communication but creates the need for synchronization.

---

## 14. Practical Application

Application:

```text
apps/thread_memory_demo
```

Build:

```bash
cmake -S . -B build
cmake --build build --target thread_memory_demo
```

Run:

```bash
./build/apps/thread_memory_demo/thread_memory_demo
```

The application experimentally demonstrates the Linux thread memory model by comparing process IDs, thread IDs, shared memory addresses, and per-thread stack addresses.
