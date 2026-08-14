# Embedded Linux Learning Roadmap

This document provides a high-level overview of the lessons, practical applications,
and main concepts covered in the `rpi-el` repository.

Its purpose is to make it easy to answer:

- Which lesson covers which topic?
- Which practical application belongs to each lesson?
- Which synchronization or Linux concept has already been studied?
- Where should the next lesson continue from?

---

## 1. Current Lesson Map

| Lesson | Topic | Related Practical Application(s) | Main Concepts | Status |
|---|---|---|---|---|
| 001 | Program vs Process | `system_info`, `process_demo`, `process_fork_demo`, `process_exec_demo` | Program, process, PID, process creation, `fork()`, `exec()` | Completed |
| 002 | Process Wait and Zombie | `process_wait_demo`, `process_zombie_demo` | Parent/child synchronization, `wait()`, zombie process | Completed |
| 003 | Linux Signals | `process_signal_demo` | Signals, signal handlers, process notification | Completed |
| 004 | Orphan Processes and Reparenting | `process_orphan_demo` | Orphan process, parent termination, reparenting | Completed |
| 005 | Linux Thread Fundamentals | `thread_create_demo` | `pthread_create()`, `pthread_join()`, PID vs TID, thread lifecycle | Completed |
| 006 | Race Condition and Mutex | `race_condition_demo`, `mutex_counter_demo` | Shared data, race condition, critical section, mutex, mutual exclusion | Completed |
| 007 | Thread Memory Model | `thread_memory_demo` *(expected, but not present in the current CMake app listing provided)* | Shared global data, heap, per-thread stack, process address space | Documentation completed |
| 008 | Condition Variables | `condition_variable_demo` | `pthread_cond_wait()`, `pthread_cond_signal()`, predicate, busy waiting | Completed |
| 009 | POSIX Semaphores | `semaphore_demo` | `sem_init()`, `sem_wait()`, `sem_post()`, counting semaphore, stored tokens | Completed |

---

## 2. Lesson Documentation

The current lesson documents are:

```text
docs/lessons/
├── Lesson-001-Program-vs-Process.md
├── Lesson-002-Process-Wait-and-Zombie.md
├── Lesson-003-Linux-Signals.md
├── Lesson-004-Orphan-Processes-and-Reparenting.md
├── Lesson-005-Linux-Thread-Fundamentals.md
├── Lesson-006-Race-Condition-and-Mutex.md
├── Lesson-007-Thread-Memory-Model.md
├── Lesson-008-Condition-Variables.md
└── Lesson-009-Semaphores.md
```

---

## 3. Practical Applications

The current CMake-based application directories are:

```text
apps/
├── condition_variable_demo/
├── mutex_counter_demo/
├── process_demo/
├── process_exec_demo/
├── process_fork_demo/
├── process_orphan_demo/
├── process_signal_demo/
├── process_wait_demo/
├── process_zombie_demo/
├── race_condition_demo/
├── semaphore_demo/
├── system_info/
└── thread_create_demo/
```

### Note About Lesson 007

`Lesson-007-Thread-Memory-Model.md` exists, but the application list provided from:

```bash
find apps -maxdepth 2 -name CMakeLists.txt -printf "%h\n" | sort
```

does not currently contain:

```text
apps/thread_memory_demo
```

The lesson was designed around a `thread_memory_demo` practical application.

This should be checked in the repository before considering the lesson/application mapping fully synchronized.

Useful commands:

```bash
find apps/thread_memory_demo -maxdepth 2 -type f -print
```

and:

```bash
git status
```

---

## 4. Learning Progression

The lessons currently follow this progression:

```text
Program
   |
   v
Process
   |
   +--> Process Creation
   |      |
   |      +--> fork()
   |      +--> exec()
   |
   +--> Parent / Child Relationship
   |      |
   |      +--> wait()
   |      +--> Zombie Process
   |      +--> Orphan Process
   |      +--> Reparenting
   |
   +--> Signals
   |
   v
Threads
   |
   +--> Thread Creation
   |      |
   |      +--> pthread_create()
   |      +--> pthread_join()
   |
   +--> Shared Process Memory
   |      |
   |      +--> Global Data
   |      +--> Heap
   |      +--> Per-Thread Stack
   |
   +--> Concurrency Problems
   |      |
   |      +--> Race Condition
   |      +--> Critical Section
   |
   +--> Synchronization
          |
          +--> Mutex
          |
          +--> Condition Variable
          |
          +--> Semaphore
```

---

## 5. Process Lessons

### Lesson 001 - Program vs Process

Documents the difference between a program stored on disk and a running process.

Related applications:

```text
system_info
process_demo
process_fork_demo
process_exec_demo
```

Important concepts:

- Executable program
- Running process
- PID
- Process address space
- `fork()`
- `exec()`
- Basic Linux process inspection

---

### Lesson 002 - Process Wait and Zombie

Explains parent/child synchronization and what happens when a terminated child is not collected by its parent.

Related applications:

```text
process_wait_demo
process_zombie_demo
```

Important concepts:

- `wait()`
- Child termination
- Exit status
- Zombie process
- Parent responsibility

---

### Lesson 003 - Linux Signals

Introduces asynchronous process notification through Linux signals.

Related application:

```text
process_signal_demo
```

Important concepts:

- Signals
- Signal delivery
- Signal handler
- Process interruption
- Asynchronous events

---

### Lesson 004 - Orphan Processes and Reparenting

Explains what happens when a parent process terminates before its child.

Related application:

```text
process_orphan_demo
```

Important concepts:

- Orphan process
- Parent PID
- Reparenting
- Process hierarchy

---

## 6. Thread Lessons

### Lesson 005 - Linux Thread Fundamentals

Introduces POSIX threads and the relationship between threads and processes.

Related application:

```text
thread_create_demo
```

Important concepts:

- `pthread_t`
- `pthread_create()`
- `pthread_join()`
- Thread entry function
- PID
- TID
- Thread lifecycle

---

### Lesson 006 - Race Condition and Mutex

Demonstrates how concurrent writes to shared data can produce incorrect results and how a mutex prevents the race.

Related applications:

```text
race_condition_demo
mutex_counter_demo
```

Important concepts:

- Shared variable
- Race condition
- Lost update
- Critical section
- Mutual exclusion
- `pthread_mutex_lock()`
- `pthread_mutex_unlock()`

The race-condition experiment used two threads incrementing the same shared counter.

The expected result was:

```text
10,000,000
```

Without proper synchronization, the actual result was significantly lower.

The mutex-protected version produced the expected result.

---

### Lesson 007 - Thread Memory Model

Explains which memory regions are shared between threads and which execution resources remain thread-specific.

Expected related application:

```text
thread_memory_demo
```

Important concepts:

- Shared process address space
- Global/static memory
- Heap
- Per-thread stack
- PID vs TID
- Shared data visibility
- Local automatic variables

Observed model:

```text
Global data  -> Shared
Heap         -> Shared
Code         -> Shared

Stack        -> Per thread
Registers    -> Per thread
TID          -> Per thread
```

---

### Lesson 008 - Condition Variables

Explains how a thread can efficiently sleep while waiting for shared state to change.

Related application:

```text
condition_variable_demo
```

Important concepts:

- `pthread_cond_wait()`
- `pthread_cond_signal()`
- Predicate
- Mutex + condition variable
- Busy waiting
- Sleeping / waking
- Rechecking the condition with `while`

Core pattern:

```c
pthread_mutex_lock(&mutex);

while (!predicate)
{
    pthread_cond_wait(&condition, &mutex);
}

/* Use shared state */

pthread_mutex_unlock(&mutex);
```

---

### Lesson 009 - POSIX Semaphores

Introduces semaphore-based event and resource counting.

Related application:

```text
semaphore_demo
```

Important concepts:

- `sem_t`
- `sem_init()`
- `sem_wait()`
- `sem_post()`
- `sem_destroy()`
- Counting semaphore
- Blocking wait
- Stored semaphore tokens
- Producer-consumer synchronization

The practical experiment demonstrates an important difference from condition variables:

```text
sem_post()
sem_post()
sem_post()

Semaphore count = 3
```

The three tokens remain available even though the worker thread was not waiting when they were posted.

Later:

```text
sem_wait() -> 2
sem_wait() -> 1
sem_wait() -> 0
```

consumes the stored tokens.

---

## 7. Synchronization Mechanism Summary

| Mechanism | Main Question | Typical Purpose |
|---|---|---|
| Mutex | Who may access the shared resource now? | Mutual exclusion |
| Condition Variable | How can a thread sleep until shared state changes? | State-change waiting |
| Semaphore | How many resources/events are available? | Counting and event synchronization |

A useful mental model is:

```text
MUTEX
"Protect this shared resource."

CONDITION VARIABLE
"Sleep until this shared condition may have changed."

SEMAPHORE
"Wait until at least one token/resource/event is available."
```

---

## 8. Repository Workflow

Each new lesson should normally contain:

```text
1. Theoretical lesson
2. Practical C application
3. CMake integration
4. Runtime observation on Raspberry Pi
5. Markdown lesson documentation
6. Update to this lesson index
7. Git commit
8. Git push
```

Recommended structure:

```text
apps/<application_name>/
├── CMakeLists.txt
└── <application_name>.c

docs/lessons/
└── Lesson-XXX-Topic.md
```

After every lesson, update:

```text
docs/LESSON_INDEX.md
```

---

## 9. Standard Build Workflow

Configure the project:

```bash
cmake -S . -B build
```

Build a specific lesson application:

```bash
cmake --build build --target <target_name>
```

Run the executable:

```bash
./build/apps/<application_name>/<application_name>
```

---

## 10. Git Workflow

Before committing:

```bash
git status
```

Stage only the files related to the lesson:

```bash
git add <lesson files>
```

Check staged changes:

```bash
git diff --cached --check
git diff --cached --stat
```

Commit:

```bash
git commit -m "<descriptive commit message>"
```

Push:

```bash
git push origin main
```

Final verification:

```bash
git status
```

Expected result:

```text
On branch main
Your branch is up to date with 'origin/main'.

nothing to commit, working tree clean
```

---

## 11. Current Progress

Current completed documentation:

```text
Lesson 001
    |
Lesson 002
    |
Lesson 003
    |
Lesson 004
    |
Lesson 005
    |
Lesson 006
    |
Lesson 007
    |
Lesson 008
    |
Lesson 009
```

Current topic boundary:

```text
Process Fundamentals
        |
        v
Thread Fundamentals
        |
        v
Shared Memory
        |
        v
Race Condition
        |
        v
Mutex
        |
        v
Condition Variables
        |
        v
POSIX Semaphores
        |
        v
NEXT LESSON
```

This index should be updated whenever a new lesson or practical application is added.
