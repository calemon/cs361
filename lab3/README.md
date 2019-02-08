# Lab 3

## Overview
You will be implementing the interactive scheduler for Marz OS. You will be writing three different interactive scheduling algorithms: round robin, multilevel, and multilevel feedback.

## Files
Place all of the files below in the same directory
```bash
sched.h
sched.cpp
libmzos.a
virt.lds
Makefile
```

## Familiarization

sched.h is a header file that contains the following:
```cpp
struct PROCESS
```
This is the main process structure and is what will be used by the operating system. The actual program is a function pointed to by program.

```cpp
enum REGISTERS
```
This enumeration identifies the regs[] array in PROCESS and which register is which in that array. For example, RA is regs[RA] or regs[1].

```cpp
enum PROCESS_STATE
```
This enumeration identifies the state of a process. DEAD is an invalid process, RUNNING is a running process and is available to be scheduled, and finally, SLEEPING is a process that is not available to be scheduled until the sleep_time expires.

```cpp
enum SCHEDULE_ALGORITHM
```
This enumeration identifies the three scheduling algorithms you will be running.

### CONSTANTS

There are several constants defined for you:

`MAX_PROCESSES` - This is the maximum number of processes this OS may have at once.

`TIMER_FREQ` - This is the frequency of the timer in Hz.

`SWITCHES_PER_SEC` - This is the number of context switches made per second.

`CT_TIMER` - This is the actual time used to enable a context switch.

`STACK_ALLOC` - This is the amount of space needed for each process' stack.

## Functions written for you.

Several helper and utility functions have been written for you.
```cpp
void wait();
```
This function blocks the process until it is scheduled again.

```cpp
void write_string(const char *);
```
This function writes the given string to the console.

```cpp
void write_stringln(const char *);
```
This function writes the given string and an appended newline character to the console.

```cpp
void to_string(char *dst, int value);
```
This function converts int value into a string and stores it into dst. This function assumes that dst has enough space to store the integer and the NULL terminator ('\0').

```cpp
void strcpy(char *dst, const char *src);
```
This function will copy src string into dst. This function assumes that dst has enough space to store src as well as the NULL terminator.

```cpp
void recover();
```
This is the function that all processes need to return to. recover() simply calls del_process() and wait().

```cpp
uint32_t get_timer_lo();
```
This returns the lower (LSB) 32-bits of the system timer. The system timer starts when the machine starts and adds TIMER_FREQ to its value every second.

```cpp
uint32_t get_timer_hi();
```
This returns the upper (MSB) 32-bits of the system timer.

```cpp
PROCESS *get_current();
```
This returns a pointer to the currently running process.

## Assignment

The only file you need to edit is sched.cpp. Do not change sched.h. We will be testing with our own testing files.

Write the following functions
```cpp
void add_new_process(int total_processes_added);
```
This function is used to add custom processes to the scheduler to test your code, and it is called whenever you type 'n' at the prompt. The integer given starts with the value 1 and repeats after the value 5. This allows you to have 5 different processes based on the integer.

```cpp
void new_process(void (*func)(), const char *name, int32_t priority=0);
```
If the process list is full of running or sleeping processes, this function does nothing. Otherwise, it looks for an open slot to store a new process.

This function creates a new process pointed to by func, called name, and prioritized by priority. You must assign a PID starting with 1 (easiest if you use PID as array_index + 1). You may reuse PIDs that are no longer linked to a process. By default, all processes are set to DEAD at boot time.

The priority is a number between [-10..10] (0 is the default). Your function must clamp any priorities outside of this range. Your scheduling algorithm may change these numbers dynamically. A priority of 10 is a low priority, where as -10 is a high priority. NOTICE: This is the same as Linux.

You must set two registers: SP (stack pointer) and RA (return address). This is a MUST, otherwise the process won't operate.

The stack pointer needs to point somewhere in the global stack_space[] array. Notice that this array contains stack space for MAX_PROCESSES processes. Remember, the stack grows bottom to top, so you need to set regs[SP] to the bottom of the process' allocated stack.

The return address is unset when the process starts. The RA register needs to be set to the recover() function. This will allow your function to return to recover(), which effectively deschedules it and removes it from the process list. Remember, recover() calls your del_process() function and a wait() afterward.

```cpp
void del_process(PROCESS *p);
```
This function deletes the process pointed to by p so that it cannot be scheduled again. Remember to properly set the state.

```cpp
void sleep_process(PROCESS *p, uint32_t sleep_time);
```
This function puts the process pointed to by p to sleep until sleep_time seconds. Remember that the timer / TIMER_FREQ is the number of seconds since the OS booted. The sleep_time variable is NOT the number of seconds the process is sleeping for. Instead, it is the value of timer / TIMER_FREQ that needs to be met before the process is awoken. You will need to properly manipulate this value and reawaken a sleeping process in your schedule() function.

```cpp
PROCESS *schedule(PROCESS *current);
```
This function invokes your scheduler.

The scheduler will first awaken any sleeping process who has reached their sleep_time value.

Then, this function must determine the scheduling algorithm. Finally, it must properly choose and return a RUNNING process.

## Examples
```
qemu-system-riscv32 -machine virt -m 128M -nographic -serial mon:stdio -kernel cs361_kernel.elf

Marz OS has booted properly.
If your code isn't properly configured, you will not see any prompt.
Hold Control, press a, and then release all keys and press x to quit the emulator.

Adding init process.
Adding idle process.

Welcome to Marz OS Init Process!
 n - Create new process.
 p - List processes.
 r - Switch to Round robin.
 m - Switch to Multi-level.
 f - Switch to Multi-level feedback.
Enter command:
```

This is what you should see when the init process runs. The init process is automatically created and added through your new_process function when the OS boots. NOTE: If your scheduler does not schedule init, you will not see a menu, nor will it accept input.

### Commands
| Command | Description |
| ------- | ----------- |
| n | Calls your add_new_process with a revolving integer. |
| p | Lists all processes. |
| r | Switches to the round-robing scheduling algorithm. |
| m | Switches to the multi-level scheduling algorithm. |
| f | Switches to the multi-level feedback scheduling algorithm. |

To exit, hold control, press a, release all keys, and then press x.

## Idle and Init Processes

These processes are created for you at boot time by the OS. Init controls the menu and inputs from the user. Idle will wait() 10 times and then sleep for 10 seconds over and over again.

Typing 'p' will list these two processes:
```
Process 'init' (PID: 1)
        Switched to: 50 times.
        Runtime: 211.
        Priority: 0.
        State: Running
        Quantum multiplier: 3.
Process 'idle' (PID: 2)
        Switched to: 11 times.
        Runtime: 10.
        Priority: 0.
        State: Sleeping until timer value 10 (currently: 2).
        Quantum multiplier: 0.
```

Here, init is PID 1, running, and has had 50 context switches for a total of 211 runtime. The idle process is PID 2, sleeping and awaiting the timer to hit 10 (it is currently at 2). It has only been switched to 11 times (mainly because it sleeps so often). The quantum multiplier is a multiplier added to 1 to determine how much additional time each slice of that process is given. For example, if the quantum multiplier is 1, then the process gets (1 + 1) = 2x the time slice. This is helpful when implementing the MLF algorithm.

Notice that the init process above has a quantum multiplier of 3, and that it appears for each context switch, it gets four times (3 + 1 = 4) the runtime. This is the purpose of the quantum multiplier.

## Multilevel and Multilevel Feedback

You will need to implement priorities for ML and MLF scheduling algorithms. Set the quantum multiplier to priority + 10. Whenever a process sleeps or is deleted, its quantum multiplier is reset to 0 by resetting the priority to -10.

You may implement wait queues and run queues, but this is not required for full credit.

You may need to modify init's priority and quantum multiplier to make sure it always runs.

## Restrictions

1. Only modify sched.cpp.

2. `<sched.h>` is the only include file you may use.

3. Any other functions you want to create must be put in sched.cpp and declared static.

4. Make sure your add_new_process and all of your testing code is between #if defined(STUDENT) and #endif. This allows the TAs to make their own add_new_process and testing code.

## Compiling
```
make run
```
This must compile and run on the **Hydra** machines.

## Submission

Submit your .cpp file.