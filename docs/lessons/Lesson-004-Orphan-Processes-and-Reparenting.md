# Lesson 004 — Orphan Processes and Re-parenting

## 1. Lesson Overview

This lesson explains what happens when a parent process terminates while its child process is still running.

The practical demonstration covers:

- Creating a parent and child process with `fork()`
- Allowing the parent process to terminate before the child
- Observing an orphan process
- Detecting a change in the child process's parent PID
- Understanding Linux process re-parenting
- Using a pipe for parent-child synchronization
- Preventing a process scheduling race condition
- Inspecting the process with `ps` and `pgrep`
- Comparing orphan and zombie processes

The following application was created:

```text
apps/process_orphan_demo
```

---

## 2. Orphan Process Definition

An orphan process is a child process whose original parent has terminated while the child is still running.

The basic sequence is:

```text
Parent creates child
        |
        +-- Parent terminates
        |
        +-- Child continues running
        |
        +-- Child becomes an orphan
```

The parent process terminating does not automatically terminate the child process.

After `fork()`, the parent and child are separate processes with their own:

- Process IDs
- Virtual address spaces
- Scheduling states
- Process-table entries
- File descriptor tables

The child can therefore continue running after the original parent terminates.

---

## 3. Linux Re-parenting

Linux does not leave a running process without a parent relationship.

When the original parent terminates, the kernel assigns the child to another process.

This operation is called:

```text
Re-parenting
```

A simplified example is:

```text
Original parent PID: 1191
Child PID          : 1192
Child original PPID: 1191
```

After the parent terminates:

```text
Child PID          : 1192
Child current PPID : 1
```

The child process continues running, but its parent PID changes.

---

## 4. The New Parent Process

In a simple Linux environment, an orphan process is commonly re-parented to the process with PID `1`.

PID `1` is normally the system's init process.

On the test system, the following command was used:

```bash
ps -o pid,ppid,stat,state,cmd -p 1
```

Example output:

```text
PID  PPID STAT S CMD
1       0 Ss   S /sbin/init
```

The orphan child was observed with:

```text
PID  PPID STAT S CMD
1192    1 S    S ./build/apps/process_orphan_demo/process_orphan_demo
```

This confirms that child process `1192` was re-parented to PID `1`.

In systems using subreapers, containers, or PID namespaces, the new parent may be a process other than the host system's PID `1`.

---

## 5. Obtaining Process IDs

The current process ID is obtained using:

```c
getpid();
```

The current parent process ID is obtained using:

```c
getppid();
```

The child first records its original parent PID:

```c
const pid_t originalParentId = getppid();
```

Later, it checks the current parent PID repeatedly:

```c
currentParentId = getppid();
```

The values are compared using:

```c
if (currentParentId != originalParentId)
{
    /* Re-parenting has occurred. */
}
```

---

## 6. Why Synchronization Is Required

The parent and child are scheduled independently after `fork()`.

Without synchronization, the parent might terminate before the child reads its original parent PID.

Possible race condition:

```text
Parent calls fork()
        |
        +-- Parent runs first
        |
        +-- Parent terminates
        |
        +-- Kernel re-parents child
        |
        +-- Child runs later
        |
        +-- getppid() already returns 1
```

In this case, the child would not observe the original parent PID.

To prevent this race condition, the demonstration uses a pipe.

---

## 7. Creating the Synchronization Pipe

The pipe is created before `fork()`:

```c
int synchronizationPipe[2];

if (pipe(synchronizationPipe) != 0)
{
    perror("pipe");
    return EXIT_FAILURE;
}
```

The two file descriptors are:

```text
synchronizationPipe[0] › Read end
synchronizationPipe[1] › Write end
```

The application uses the pipe as follows:

```text
Child  › writes a ready token
Parent › waits for the ready token
```

The pipe is used as a synchronization mechanism rather than for transferring application data.

---

## 8. Pipe Descriptors After `fork()`

Both processes inherit both pipe descriptors after `fork()`.

Initially:

```text
Parent owns read and write ends
Child owns read and write ends
```

Each process closes the descriptor that it does not use.

The child closes the read end:

```c
close(synchronizationPipe[0]);
```

The parent closes the write end:

```c
close(synchronizationPipe[1]);
```

The final communication structure is:

```text
Child: write end only
Parent: read end only
```

Closing unused descriptors prevents:

- File descriptor leaks
- Incorrect pipe EOF behavior
- Unexpected blocking
- Confusing ownership

---

## 9. Child Ready Token

The child stores and prints its original parent PID before allowing the parent to terminate.

It then writes a one-byte token to the pipe:

```c
const char readyToken = 'R';

if (write(
        synchronizationWriteFd,
        &readyToken,
        sizeof(readyToken)) != (ssize_t)sizeof(readyToken))
{
    perror("write");
    close(synchronizationWriteFd);
    return EXIT_FAILURE;
}
```

The character `R` represents:

```text
Ready
```

The actual character value is not critical. The important event is that one byte is successfully written.

---

## 10. Parent Synchronization

The parent waits for the child token:

```c
readResult = read(
    synchronizationPipe[0],
    &readyToken,
    sizeof(readyToken));
```

If the pipe is empty, `read()` blocks the parent.

The flow becomes:

```text
Parent waits inside read()
        |
        +-- Child stores original PPID
        |
        +-- Child prints process information
        |
        +-- Child writes the ready token
        |
        +-- Parent read() returns
```

Only after this synchronization does the parent terminate.

This guarantees that the child has already captured the original parent PID.

---

## 11. Parent Termination

After receiving the ready token, the parent prints:

```text
[PARENT] Child is ready. Parent is terminating now.
```

The parent then returns from `main()`:

```c
return EXIT_SUCCESS;
```

The parent process terminates normally.

The child remains active and continues its observation loop.

---

## 12. Observation Configuration

The child observation settings are:

```c
#define CHILD_OBSERVATION_CYCLE_COUNT    (120U)
#define CHILD_OBSERVATION_PERIOD_SECONDS (1U)
```

The child performs:

```text
120 observation cycles
```

with:

```text
1 second between each cycle
```

The total observation time is approximately:

```text
120 cycles × 1 second = 120 seconds
```

The cycle counter runs from:

```text
Cycle 0
```

through:

```text
Cycle 119
```

---

## 13. Observation Loop

The child repeatedly checks its current parent PID:

```c
for (cycleCount = 0U;
     cycleCount < CHILD_OBSERVATION_CYCLE_COUNT;
     cycleCount++)
{
    sleep(CHILD_OBSERVATION_PERIOD_SECONDS);

    currentParentId = getppid();

    printf(
        "[CHILD] Cycle: %u, current PPID: %ld\n",
        cycleCount,
        (long)currentParentId);
}
```

The `sleep()` call places the child in a waiting state between observations.

While sleeping, the process commonly appears with:

```text
STAT = S
```

The `S` state means:

```text
Interruptible sleep
```

The child is still alive and may continue running after the sleep period.

---

## 14. Detecting Re-parenting

The child compares its current parent PID with the original parent PID:

```c
if (currentParentId != originalParentId)
{
    printf(
        "[CHILD] Parent changed from %ld to %ld. "
        "The child has been re-parented.\n",
        (long)originalParentId,
        (long)currentParentId);
}
```

Observed output:

```text
[CHILD] Parent changed from 1191 to 1.
The child has been re-parented.
```

This confirms that:

```text
Original parent PID: 1191
New parent PID     : 1
```

If this condition remains inside the observation loop without an additional state flag, the re-parenting message is printed during every remaining cycle because the current PPID remains different from the original PPID.

---

## 15. Output Buffer Flushing

The demonstration uses:

```c
fflush(stdout);
```

after important output operations.

Standard output may be buffered by the C library.

Calling `fflush(stdout)` forces pending output to be written immediately.

This is useful because:

- The parent terminates shortly after printing
- Parent and child write to the same terminal
- The shell prompt may return while the child is still active
- Output order should remain observable

---

## 16. Observing the Child Process

The running process was located using:

```bash
pgrep -af process_orphan_demo
```

Observed result:

```text
1192 ./build/apps/process_orphan_demo/process_orphan_demo
```

The process details were then inspected using:

```bash
ps -o pid,ppid,stat,state,cmd -p 1192
```

Observed output:

```text
PID  PPID STAT S CMD
1192    1 S    S ./build/apps/process_orphan_demo/process_orphan_demo
```

The important fields are:

### `PID`

```text
1192
```

The process ID of the child.

### `PPID`

```text
1
```

The child has been re-parented to the init process.

### `STAT`

```text
S
```

The child is in interruptible sleep.

It remains a normal living process.

---

## 17. Why the Original Parent Was Not Found

The following command was executed:

```bash
ps -o pid,ppid,stat,state,cmd -p 1191
```

Only the column header was displayed.

This means process `1191` no longer existed.

That result was expected because `1191` was the original parent, and it had already terminated.

The running child was process `1192`.

---

## 18. Child Termination

After completing all 120 observation cycles, the child prints:

```text
[CHILD] Observation completed. Child is terminating.
```

The child then returns:

```c
return EXIT_SUCCESS;
```

After the child terminates, the following command produces no output:

```bash
pgrep -af process_orphan_demo
```

This confirms that the child process no longer exists.

The init process collects the child termination information and removes its process-table entry.

---

## 19. Orphan and Zombie Process Comparison

Orphan and zombie processes are different concepts.

### Orphan Process

```text
Parent has terminated
Child is still running
```

An orphan process:

- Executes program instructions
- Can use CPU time
- Can sleep or run
- Is re-parented
- Has a normal process state such as `S` or `R`

### Zombie Process

```text
Child has terminated
Parent has not collected its result
```

A zombie process:

- Does not execute instructions
- Does not perform application work
- Keeps a process-table entry
- Has state `Z`
- Is removed using `wait()` or `waitpid()`

Comparison:

| Property | Orphan process | Zombie process |
|---|---:|---:|
| Child is alive | Yes | No |
| Child executes code | Yes | No |
| Original parent is alive | No | Usually yes |
| Re-parented | Yes | Not applicable |
| Process state is `Z` | No | Yes |
| Uses normal CPU scheduling | Yes | No |
| Removed using `waitpid()` | After later termination | Yes |

The simplest distinction is:

```text
Orphan: Parent is dead, child is alive.
Zombie: Child is dead, parent has not collected it.
```

---

## 20. Inherited File Descriptors

The child inherits open file descriptors from the parent during `fork()`.

The original parent terminating does not automatically close the child process's own descriptor copies.

This explains why the child can continue writing to the same terminal after the parent exits.

Each process owns its own descriptor-table entries after `fork()`.

For example:

```text
Parent stdout descriptor › Closed when parent terminates
Child stdout descriptor  › Remains open until child closes it or terminates
```

This behavior is also important for:

- Pipes
- Files
- Sockets
- Serial ports
- Device files

Unused descriptors should always be closed explicitly.

---

## 21. Embedded Linux Considerations

Orphan processes can be dangerous in embedded Linux systems if child processes control hardware or shared resources.

Example architecture:

```text
Main control process
    |
    +-- Communication worker
    +-- Logger worker
    +-- Hardware output worker
```

If the main process terminates while a worker continues running, the worker may:

- Continue applying stale commands
- Keep a device file open
- Keep a serial port busy
- Prevent a restarted process from acquiring resources
- Continue using outdated configuration
- Control hardware without supervision

A process architecture should define:

- What happens when the parent terminates
- Whether child processes should terminate automatically
- Who owns hardware resources
- How stale commands are invalidated
- Which process performs cleanup
- Whether a supervisor manages all related processes

---

## 22. Parent-Death Detection Alternatives

Repeatedly checking `getppid()` is simple but polling-based.

More advanced alternatives include:

- Heartbeat monitoring
- Communication timeout
- Pipe closure detection
- Socket disconnection detection
- Process supervision
- `systemd` service management
- Linux parent-death signals

Linux supports:

```c
prctl(PR_SET_PDEATHSIG, SIGTERM);
```

This requests that the kernel send a signal to the child when its parent terminates.

This interface is Linux-specific and requires careful race-condition handling.

It will be covered in a later process-supervision lesson.

---

## 23. Process Tree View

Before the original parent terminates:

```text
init
 |
 +-- shell
      |
      +-- parent process
            |
            +-- child process
```

After the original parent terminates:

```text
init
 |
 +-- shell
 |
 +-- child process
```

The child is no longer located below the original parent in the process tree.

The kernel has reassigned it to the init process.

---

## 24. Error Handling

The application checks the return values of:

```c
pipe()
fork()
write()
read()
```

Example:

```c
if (pipe(synchronizationPipe) != 0)
{
    perror("pipe");
    return EXIT_FAILURE;
}
```

System-call failures should not be ignored.

The application also closes pipe descriptors on error paths to prevent resource leaks.

---

## 25. Practical Test Summary

The test produced the following process relationship:

```text
Original parent PID: 1191
Child PID          : 1192
```

After the parent terminated:

```text
Child current PPID: 1
```

The child was inspected using:

```bash
pgrep -af process_orphan_demo
```

and:

```bash
ps -o pid,ppid,stat,state,cmd -p 1192
```

The observed process state was:

```text
PID  = 1192
PPID = 1
STAT = S
```

This demonstrated that:

- The original parent terminated
- The child remained alive
- The child was re-parented
- The child was not a zombie
- The child completed its 120-cycle observation
- The child terminated normally
- The final process entry was removed

---

## 26. Key Takeaways

- An orphan is a child process whose original parent has terminated.
- Parent termination does not automatically terminate the child.
- Linux reassigns an orphan process to another parent.
- In a simple system, the new parent is commonly PID `1`.
- `getppid()` returns the current parent process ID.
- The parent PID can change while the child continues running.
- A pipe can synchronize the parent and child processes.
- Synchronization prevents the parent from terminating too early.
- `read()` blocks the parent until the child writes the ready token.
- Unused pipe descriptors should be closed after `fork()`.
- The demonstration performs 120 observation cycles.
- Each cycle is separated by approximately one second.
- An orphan process is still alive and may have state `S` or `R`.
- A zombie process has terminated and has state `Z`.
- File descriptors inherited by the child remain valid after the parent exits.
- Embedded Linux systems must define how worker processes behave if their parent fails.