# Lesson 002 — Process Waiting and Zombie Processes

## 1. Lesson Overview

This lesson explains how a parent process waits for a child process, retrieves its termination status, and prevents zombie processes.

The practical examples cover:

* Waiting for a child process with `waitpid()`
* Reading the child process exit status
* Detecting normal termination
* Detecting signal-based termination
* Creating a zombie process intentionally
* Observing a zombie process with `ps`
* Removing a zombie process using `waitpid()`

The following applications were created:

```text
apps/process_wait_demo
apps/process_zombie_demo
```

---

## 2. Parent and Child Process Relationship

A process can create a new child process using:

```c
fork();
```

After `fork()`, two processes continue execution:

```text
Original process › Parent process
New process      › Child process
```

The parent and child have different process IDs.

Example:

```text
Parent PID: 1458
Child PID : 1459
Child PPID: 1458
```

The child process can obtain its own PID using:

```c
getpid();
```

It can obtain its parent PID using:

```c
getppid();
```

---

## 3. Child Process Termination

A child process may terminate by returning from `main()`:

```c
return EXIT_SUCCESS;
```

It may also terminate using:

```c
exit(42);
```

In this example, `42` is the child process exit status.

The exit status allows the child to report a result to its parent.

Typical exit status values are:

```text
0      › Successful termination
Nonzero › Error or application-specific result
```

Only the lower eight bits of a normal process exit status are available to the parent.

Therefore, portable exit status values are normally kept between:

```text
0 and 255
```

---

## 4. Why the Parent Must Wait

When a child process terminates, the Linux kernel keeps a small amount of information about it.

This information includes:

* Child PID
* Termination status
* Resource usage information
* Whether the child terminated normally
* Whether the child was terminated by a signal

The information remains available until the parent collects it using:

```c
wait();
```

or:

```c
waitpid();
```

This collection operation is commonly called:

```text
Reaping the child process
```

---

## 5. The `wait()` Function

The `wait()` function waits for any child process to terminate.

Prototype:

```c
pid_t wait(int *status);
```

Example:

```c
wait(NULL);
```

This means:

```text
Wait for any child process.
Do not store its termination status.
```

Another example:

```c
int childStatus;

wait(&childStatus);
```

This waits for any child and stores its encoded termination information in:

```c
childStatus
```

The limitation of `wait()` is that it does not directly select a specific child process.

---

## 6. The `waitpid()` Function

The `waitpid()` function provides more control than `wait()`.

Prototype:

```c
pid_t waitpid(
    pid_t pid,
    int *status,
    int options);
```

Example:

```c
terminatedProcessId = waitpid(
    childProcessId,
    &childStatus,
    0);
```

The parameters have the following meanings:

### `childProcessId`

Identifies the specific child process to wait for.

### `&childStatus`

Provides the address where the kernel stores the encoded termination information.

### `0`

Causes the parent to block until the selected child process changes to a waitable state, normally until it terminates.

The return value is:

```text
Child PID › Success
-1        › Failure
```

Example error handling:

```c
terminatedProcessId = waitpid(
    childProcessId,
    &childStatus,
    0);

if (terminatedProcessId < 0)
{
    perror("waitpid");
    return EXIT_FAILURE;
}
```

---

## 7. Why the Status Is Encoded

The variable filled by `waitpid()` contains more than the child exit code.

Example:

```c
int childStatus;
```

It may contain information such as:

* Normal termination
* Exit status
* Signal-based termination
* Terminating signal
* Stop state
* Continue state

For this reason, the parent should not use `childStatus` directly as the exit code.

The parent must use the macros provided by:

```c
#include <sys/wait.h>
```

---

## 8. Detecting Normal Termination

The macro:

```c
WIFEXITED(childStatus)
```

checks whether the child process terminated normally.

Normal termination includes:

```c
return value;
```

and:

```c
exit(value);
```

Example:

```c
if (WIFEXITED(childStatus))
{
    printf(
        "Child terminated normally.\n");
}
```

The macro returns a nonzero value when the child terminated normally.

---

## 9. Reading the Child Exit Status

After confirming normal termination, the exit status can be obtained using:

```c
WEXITSTATUS(childStatus)
```

Example:

```c
if (WIFEXITED(childStatus))
{
    printf(
        "Child exit status: %d\n",
        WEXITSTATUS(childStatus));
}
```

If the child calls:

```c
exit(42);
```

the parent receives:

```text
Child exit status: 42
```

The practical test produced:

```text
[PARENT] Process started. PID: 1458
[PARENT] Child created. PID: 1459
[PARENT] Waiting for child process...
[CHILD] PID : 1459
[CHILD] PPID: 1458
[CHILD] Performing operation...
[CHILD] Operation completed.
[CHILD] Terminating with exit status 42.
[PARENT] Child process terminated.
[PARENT] Child exited normally with status: 42
```

---

## 10. Detecting Signal-Based Termination

A child process may be terminated by a signal instead of calling `exit()`.

The macro:

```c
WIFSIGNALED(childStatus)
```

checks whether the child was terminated by a signal.

Example:

```c
if (WIFSIGNALED(childStatus))
{
    printf(
        "Child was terminated by a signal.\n");
}
```

The signal number can be obtained using:

```c
WTERMSIG(childStatus)
```

Example:

```c
if (WIFSIGNALED(childStatus))
{
    printf(
        "Child terminating signal: %d\n",
        WTERMSIG(childStatus));
}
```

This allows the parent to distinguish between:

```text
Normal child exit
```

and:

```text
Forced termination by signal
```

---

## 11. Blocking Behavior of `waitpid()`

The following call blocks the parent:

```c
waitpid(childProcessId, &childStatus, 0);
```

The parent does not continue until the selected child terminates.

Example flow:

```text
Parent creates child
        |
        +-- Parent enters waitpid()
        |
        +-- Parent is blocked
        |
        +-- Child performs operation
        |
        +-- Child terminates
        |
        +-- waitpid() returns
        |
        +-- Parent continues
```

While blocked, the parent does not repeatedly consume CPU time.

The kernel places the parent in a waiting state until the child status changes.

---

## 12. Zombie Process Definition

A zombie process is a child process that:

```text
Has finished executing
```

but:

```text
Has not yet been collected by its parent
```

The child program is no longer running.

However, its process-table entry remains available so the parent can read its termination information.

The basic lifecycle is:

```text
fork()
  |
  +-- Child runs
  |
  +-- Child calls exit()
  |
  +-- Child becomes a zombie
  |
  +-- Parent calls wait() or waitpid()
  |
  +-- Child entry is removed
```

---

## 13. Zombie Process Characteristics

A zombie process:

* Does not execute instructions
* Does not use normal CPU time
* Does not continue application work
* Keeps its PID temporarily
* Keeps its exit status
* Keeps a small process-table record
* Waits for the parent to collect it

A zombie process is therefore not a running process.

It is a terminated process with uncollected termination information.

---

## 14. Creating a Zombie Process Intentionally

The zombie demonstration uses the following sequence:

```text
Child terminates immediately
Parent delays waitpid()
```

Example:

```c
if (childProcessId == 0)
{
    exit(42);
}

sleep(60);

waitpid(childProcessId, &childStatus, 0);
```

During the `sleep(60)` period:

```text
The child has already terminated
```

but:

```text
The parent has not called waitpid()
```

Therefore, the child remains in the zombie state.

---

## 15. Observing the Zombie Process

The zombie process can be inspected from another terminal using:

```bash
ps -o pid,ppid,stat,state,cmd -p CHILD_PID
```

Example:

```bash
ps -o pid,ppid,stat,state,cmd -p 1858
```

Observed output:

```text
PID    PPID STAT S CMD
1858   1857 Z+   Z [process_zombie_] <defunct>
```

The fields mean:

### `PID`

The process ID of the zombie child.

```text
1858
```

### `PPID`

The process ID of the parent.

```text
1857
```

### `STAT`

The process status.

```text
Z+
```

The important character is:

```text
Z
```

which indicates a zombie process.

The `+` character indicates that the process belongs to the foreground process group of its controlling terminal.

### `STATE`

The single-letter state:

```text
Z
```

### `<defunct>`

Indicates that the process has completed execution but still has an entry in the process table.

---

## 16. Why the Process Name Appeared Shortened

The observed process name was:

```text
[process_zombie_]
```

instead of the full executable name.

Linux stores and displays some process names using fields with limited length.

Therefore, long executable names may appear truncated in commands such as:

```bash
ps
```

This does not indicate an application error.

---

## 17. Removing the Zombie Process

After the parent calls:

```c
waitpid(childProcessId, &childStatus, 0);
```

the kernel allows the parent to read the child termination information.

The kernel then removes the zombie entry from the process table.

A later command:

```bash
ps -p CHILD_PID
```

should no longer show the child process.

The process lifecycle becomes:

```text
Running child
      |
      +-- Child exits
      |
      +-- Zombie
      |
      +-- Parent calls waitpid()
      |
      +-- Reaped
      |
      +-- Removed from process table
```

---

## 18. What Happens If the Parent Never Waits

If a parent continuously creates children and never calls:

```c
wait();
```

or:

```c
waitpid();
```

zombie entries may accumulate.

A small number of temporary zombies is usually not a major problem.

However, a large number of zombies can consume process-table entries and available PIDs.

This can eventually prevent the system from creating new processes.

Long-running services that create child processes must therefore implement a child-reaping strategy.

---

## 19. `SIGCHLD` Relationship

When a child changes state, Linux may send the parent:

```text
SIGCHLD
```

A common event is:

```text
Child process terminates
        |
        +-- Kernel sends SIGCHLD to parent
```

The parent may then call:

```c
waitpid();
```

to collect the child.

In the zombie demonstration, no `SIGCHLD` handler was required because the parent explicitly called:

```c
waitpid(childProcessId, &childStatus, 0);
```

after the sleep period.

Signal-based child handling will be discussed in later lessons.

---

## 20. `wait()` and `waitpid()` Comparison

| Feature                                     | `wait()` | `waitpid()` |
| ------------------------------------------- | -------: | ----------: |
| Wait for any child                          |      Yes |         Yes |
| Wait for a specific child                   |       No |         Yes |
| Obtain exit status                          |      Yes |         Yes |
| Support non-blocking operation              |       No |         Yes |
| Support additional options                  |       No |         Yes |
| Recommended for controlled child management |  Limited |         Yes |

For a program with only one child, either function may be sufficient.

For multiple child processes, `waitpid()` provides better control.

---

## 21. Common Coding Error: Uninitialized Status Variables

The following variables were used:

```c
pid_t terminatedProcessId;
int childStatus;
```

They become valid only after a successful `waitpid()` call.

Correct usage:

```c
terminatedProcessId = waitpid(
    childProcessId,
    &childStatus,
    0);

if (terminatedProcessId < 0)
{
    perror("waitpid");
    return EXIT_FAILURE;
}
```

Only after successful completion should the application inspect:

```c
WIFEXITED(childStatus)
```

or:

```c
WIFSIGNALED(childStatus)
```

A common mistake is calling `waitpid()` without storing its return value:

```c
waitpid(childProcessId, &childStatus, 0);

if (terminatedProcessId < 0)
{
    /* terminatedProcessId was never assigned. */
}
```

This causes uninitialized-variable warnings and undefined behavior.

---

## 22. Common Coding Error: Nested Comment Opening

The following comment format is incorrect:

```c
/*****************************************************************************
/**
 * @brief Example description.
 */
```

The first comment was not closed before the second comment began.

Correct format:

```c
/*****************************************************************************/
/**
 * @brief Example description.
 */
```

The separator line must end with:

```text
*/
```

This removes compiler warnings such as:

```text
"/*" within comment
```

---

## 23. Practical Applications

The concepts in this lesson are used in:

* Command execution tools
* Shell implementations
* Server applications
* Process supervisors
* Service managers
* Build systems
* Test frameworks
* Worker-process architectures
* Embedded Linux applications
* Device-management services

Whenever a process creates child processes, it should define how those children are monitored and collected.

---

## 24. Practical Test Summary

### Process Wait Test

The parent created one child process.

The child:

```text
Performed a simulated operation
Waited for two seconds
Terminated with exit status 42
```

The parent:

```text
Waited using waitpid()
Detected normal termination
Read exit status 42
```

### Zombie Process Test

The child:

```text
Terminated immediately
```

The parent:

```text
Delayed waitpid() for 60 seconds
```

During that period, the child appeared as:

```text
Z
<defunct>
```

After `waitpid()` was called, the zombie process disappeared.

---

## 25. Key Takeaways

* A parent process can wait for a child using `wait()` or `waitpid()`.
* `waitpid()` can wait for a specific child process.
* The status returned by `waitpid()` is encoded.
* `WIFEXITED()` checks for normal termination.
* `WEXITSTATUS()` extracts the normal exit status.
* `WIFSIGNALED()` checks for signal-based termination.
* `WTERMSIG()` extracts the terminating signal.
* A zombie process has terminated but has not been collected.
* A zombie process does not execute code.
* Zombie processes are visible with state `Z`.
* `<defunct>` indicates a terminated but unreaped process.
* `wait()` or `waitpid()` removes the zombie entry.
* Long-running parent processes must collect terminated children.
* Compiler warnings about uninitialized variables should not be ignored.
