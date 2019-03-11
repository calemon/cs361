# Review

## MMIO

Can not determine what the memory address of a program and its variables are at program beginning
- Can not assign addresses either (can but will most likely seg fault)

```cpp
uint32_t value = 0;
uint32_t *reg;
*reg = value;
// Cannot do just '*reg = 5;' because it is not used thus it will not be allocated

/* Setting bits */
*reg |= 1 << 14 | 1 << 10;

/* Clearing bits */
*reg &= ~(1 << 14) | ~(1 << 10);
```

Exam will have memory address and we will have to set, clear, (maybe) mask, and (maybe) test individual bits
- Stuff from Lab 1

## Concurrency

- Order of which thread will run is not guaranteed
  - If you have 10 threads, the order could be random each time the program is ran. 
  - The threads will stick to around the order of thread creation but it is still not guaranteed

**MTVEC** not important for the exam; i.e. was not covered by Adam

## Topics for Exam

**May be** able to have a cheat sheet

Will have to write pseudo-code/code but it will not have to compile

While taking the exam, if making an assumption then write the assumption down. Don't make assumptions that contradict the questions.

1) MMU
   - Lab 4 will be helpful in understanding MMU
   - Know 5 bottom bits of Page Table Entry chart
     - UXWRV
   - Know how to translate between addresses on paper.
2) Scheduling algorithms
   - Lab 3 will be helpful
3) Concurrency
   - Understand how mutexes work, time a program would run if all threads slept for 1 second and didn't have a mutex, and time a program would run if all thread slept for 1 second and shared a mutex.
   - Understand **mutexes**/semaphores; Will not need to code it
4) MMIO
   - Lab 2
   - Setting values at a memory address
   - Manipulating values at an address
   - Will not need to memorize registers

