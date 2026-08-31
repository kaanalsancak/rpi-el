# Embedded Linux Learning Roadmap

This document provides a high-level overview of the lessons, practical applications,
and main concepts covered in the `rpi-el` repository.

Use the links below to open a lesson document or jump directly to the related application folder.

---

## 1. Current Lesson Map

| Lesson | Topic | Lesson Notes | Practical Application(s) | Main Concepts | Status |
|---|---|---|---|---|---|
| 001 | Program vs Process | [Open lesson](lessons/Lesson-001-Program-vs-Process.md) | [system_info](../apps/system_info/) · [process_demo](../apps/process_demo/) · [process_fork_demo](../apps/process_fork_demo/) · [process_exec_demo](../apps/process_exec_demo/) | Program, process, PID, `fork()`, `exec()` | Completed |
| 002 | Process Wait and Zombie | [Open lesson](lessons/Lesson-002-Process-Wait-and-Zombie.md) | [process_wait_demo](../apps/process_wait_demo/) · [process_zombie_demo](../apps/process_zombie_demo/) | `wait()`, parent/child synchronization, zombie process | Completed |
| 003 | Linux Signals | [Open lesson](lessons/Lesson-003-Linux-Signals.md) | [process_signal_demo](../apps/process_signal_demo/) | Signals, signal handlers, asynchronous notification | Completed |
| 004 | Orphan Processes and Reparenting | [Open lesson](lessons/Lesson-004-Orphan-Processes-and-Reparenting.md) | [process_orphan_demo](../apps/process_orphan_demo/) | Orphan process, reparenting, process hierarchy | Completed |
| 005 | Linux Thread Fundamentals | [Open lesson](lessons/Lesson-005-Linux-Thread-Fundamentals.md) | [thread_create_demo](../apps/thread_create_demo/) | `pthread_create()`, `pthread_join()`, PID vs TID, thread lifecycle | Completed |
| 006 | Race Condition and Mutex | [Open lesson](lessons/Lesson-006-Race-Condition-and-Mutex.md) | [race_condition_demo](../apps/race_condition_demo/) · [mutex_counter_demo](../apps/mutex_counter_demo/) | Race condition, critical section, mutex, mutual exclusion | Completed |
| 007 | Thread Memory Model | [Open lesson](lessons/Lesson-007-Thread-Memory-Model.md) | `thread_memory_demo` *(not present in the current app listing provided)* | Global/static data, heap, per-thread stack, shared address space | Documentation completed |
| 008 | Condition Variables | [Open lesson](lessons/Lesson-008-Condition-Variables.md) | [condition_variable_demo](../apps/condition_variable_demo/) | `pthread_cond_wait()`, `pthread_cond_signal()`, predicate, busy waiting | Completed |
| 009 | POSIX Semaphores | [Open lesson](lessons/Lesson-009-Semaphores.md) | [semaphore_demo](../apps/semaphore_demo/) | `sem_wait()`, `sem_post()`, counting semaphore, stored tokens | Completed |
| 010 | [Deadlock, Starvation, and Livelock](lessons/Lesson-010-Deadlock-Starvation-and-Livelock.md) | Deadlock, lock ordering, starvation, livelock, fairness, and progress |

---

## 2. Learning Progression

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

## 3. Synchronization Mechanism Summary

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

## 4. Repository Workflow

Each new lesson should normally contain:

```text
1. Theoretical lesson
2. Practical C application
3. CMake integration
4. Runtime observation on Raspberry Pi
5. Markdown lesson documentation
6. Update to docs/LESSON_INDEX.md
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

---

## 5. Standard Build Workflow

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

## 6. Git Workflow

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

## 7. Current Progress

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
    |
NEXT LESSON
```

This index should be updated whenever a new lesson or practical application is added.
