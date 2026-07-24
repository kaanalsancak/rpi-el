# Lesson 001 — Program vs Process

## 1. Objective

Bu dersin amacý program, process, fork, exec ve wait iliþkisini anlamaktýr.

Ders sonunda aþaðýdaki kavramlar açýklanabilmelidir:

- Program ile process arasýndaki fark
- Kernel’in process oluþturmadaki rolü
- `fork()` davranýþý
- `exec()` davranýþý
- Parent-child iliþkisi
- `waitpid()` davranýþý
- `strace` ile process yaþam döngüsünün gözlemlenmesi

---

## 2. Program Nedir?

Program, diskte duran pasif bir executable dosyadýr.

Bir programýn kendi baþýna:

- PID deðeri yoktur.
- Scheduler durumu yoktur.
- Çalýþan register deðerleri yoktur.
- Parent process’i yoktur.
- CPU zamaný yoktur.

Program yalnýzca çalýþtýrýlacak instruction’larý, verileri ve loader tarafýndan kullanýlacak metadata’yý içerir.

---

## 3. Process Nedir?

Process, kernel tarafýndan oluþturulan ve yönetilen bir yürütme baðlamýdýr.

Bir process tipik olarak þunlarý içerir:

- PID ve PPID
- Kullanýcý ve grup bilgileri
- Sanal adres alaný
- Stack
- Heap
- Açýk file descriptor’lar
- Signal durumu
- Scheduler bilgileri
- En az bir thread

Process’in var olmasý ile CPU üzerinde çalýþýyor olmasý ayný þey deðildir.

Bir process aþaðýdaki durumlardan birinde olabilir:

- Running
- Runnable
- Sleeping
- Stopped
- Zombie

---

## 4. Program ve Process Farký

```text
Program
    Diskte duran pasif executable

Process
    Programýn kernel tarafýndan yönetilen çalýþan örneði