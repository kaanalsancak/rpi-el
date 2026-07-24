# Lesson 001 — Program vs. Process

**Author:** Muhammet Kaan Alsancak  
**Course:** Embedded Linux From First Principles  
**Level:** Beginner  
**Estimated Reading Time:** 35 minutes  

---

# 1. Objective

The goal of this lesson is to understand the relationship between a **program**, a **process**, `fork()`, `exec()`, and `waitpid()`.

After completing this lesson, you should be able to:

- Explain the difference between a program and a process.
- Explain the role of the Linux kernel in process creation.
- Describe how `fork()` works.
- Describe how `exec()` works.
- Explain the parent-child relationship.
- Explain the purpose of `waitpid()`.
- Observe process creation using `strace`.

---

# 2. What Is a Program?

A **program** is a passive executable file stored on disk.

A program by itself has:

- No PID
- No scheduling state
- No CPU execution context
- No parent process
- No execution time

It only contains machine instructions, initialized data, and metadata required to build a running process.

---

# 3. What Is a Process?

A **process** is an execution context created and managed by the operating system kernel.

A typical process contains:

- PID and PPID
- User and group information
- Virtual address space
- Stack
- Heap
- Open file descriptors
- Signal information
- Scheduling information
- One or more threads

A process may exist without currently running on a CPU.

Typical process states include:

- Running
- Runnable
- Sleeping
- Stopped
- Zombie

---

# 4. Program vs. Process

```text
Program
    Passive executable stored on disk

Process
    Running instance managed by the kernel
```

A single executable may create zero, one, or many processes.

---

# 5. fork()

`fork()` creates a new child process.

```text
Parent
   |
fork()
   |
   +------> Parent
   |
   +------> Child
```

Both parent and child continue execution immediately **after** the `fork()` call.

Return values:

```text
Parent : Child PID
Child  : 0
Error  : -1
```

Example:

```c
pid_t pid = fork();

if (pid < 0)
{
    /* Error */
}
else if (pid == 0)
{
    /* Child */
}
else
{
    /* Parent */
}
```

---

# 6. fork() Lab

Observed output:

```text
Before fork: PID=1351 PPID=1104
Parent: PID=1351 Child PID=1352
Child : PID=1352 PPID=1351
```

Observations:

- Parent PID remains unchanged.
- A new PID is assigned to the child.
- The child's PPID equals the parent's PID.
- Both processes continue after `fork()`.
- Execution order is determined by the scheduler.

---

# 7. exec()

`exec()` does **not** create a new process.

Instead, it replaces the current program image.

```text
Child (PID=1487)
Program = process_exec_demo

        exec()

Child (PID=1487)
Program = /bin/echo
```

The PID remains the same.

A successful `exec()` never returns.

---

# 8. exec() Lab

Observed output:

```text
Parent before fork
Parent after fork
Child before exec
Child program replaced by /bin/echo
Parent: child exited with status=0
```

Observations:

- `fork()` creates the child.
- `exec()` keeps the same PID.
- The child executes a different executable.
- Code after `exec()` runs only if `exec()` fails.

---

# 9. waitpid()

```c
waitpid(childPid, &status, 0);
```

Responsibilities:

- Wait for a specific child.
- Collect the child's exit status.
- Prevent zombie processes.

---

# 10. strace Observations

Command:

```bash
strace -f ./build/apps/process_exec_demo/process_exec_demo
```

Important system calls:

```text
execve()
clone()
wait4()
write()
exit_group()
```

On this Linux system:

- `fork()` is implemented using `clone()`.
- `waitpid()` results in the `wait4()` system call.

---

# 11. Execution Flow

```text
Shell
 |
 | execve(process_exec_demo)
 v
Parent
 |
 | clone()
 v
Child
 |
 | execve("/bin/echo")
 v
Same PID
 |
 | exit_group()
 v
SIGCHLD
 |
Parent wait4()
 |
Parent exits
```

---

# 12. Key Takeaways

- A program and a process are different concepts.
- A process is a kernel abstraction.
- `fork()` creates a child process.
- `exec()` replaces the current program image.
- `waitpid()` synchronizes parent and child.
- `printf()` eventually reaches the kernel through `write()`.
- Child termination generates `SIGCHLD`.

---

# 13. Common Misconceptions

- `exec()` creates a new process. ?
- The child starts from the beginning of the program. ?
- The parent always executes before the child. ?
- `fork()` is called twice. ?
- A sleeping process is no longer a process. ?

---

# 14. Parking Lot

Topics for future lessons:

- Copy-on-Write
- Stack and heap after `fork()`
- Global variables
- File descriptor inheritance
- Zombie and orphan processes
- `task_struct`
- `clone()` vs. `fork()`
- ELF loader
- Dynamic linker
- `_start` before `main()`

---

# 15. Next Lesson

**Lesson 002 — What Does fork() Copy?**

Topics:

- Address spaces
- Stack duplication
- Heap behavior
- Global variables
- Copy-on-Write
- File descriptor inheritance

---

# Learning Outcomes

After completing this lesson, you should be able to:

- Explain the difference between a program and a process.
- Explain why a process is an operating system abstraction.
- Describe the behavior of `fork()`.
- Describe the behavior of `exec()`.
- Explain the purpose of `waitpid()`.
- Interpret the basic output of `strace`.
