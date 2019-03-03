# CS361 - Operating Systems

## Labs

- [Lab 1](lab1/README.md): Three operations are commonly used to program CPU and I/O registers: Test, Set, and Clear. Lab 1 will implement these 3 operations along with a Mask function.
  - Grade: 100/100
- [Lab 2](lab2/README.md): You will be creating a UART (Universal Asynchronous Receiver Transmitter) driver for MZOS (Marz OS). The point of this lab is to learn how to control hardware from an operating system. The programming part requires no more knowledge than pointers and pointer arithmetic.
  - Grade: 90/100
    - -10 for not handling 8BIT and PODD.
    - Note, you don't need to return from a void function. You always have a return statement at the end of the function that is not needed.
- [Lab 3](lab3/README.md): You will be implementing the interactive scheduler for Marz OS. You will be writing three different interactive scheduling algorithms: round robin, multilevel, and multilevel feedback.
  - Grade: 90/100
    - -10 You run through the entire list of processes, update the times, then start your scheduler. You should check to see if the time can be updated as you're trying to schedule it.
- [Lab 4](lab4/README.md): You will be writing a few functions required to implement the Sv32 memory management unit. This time, MZOS has both kernel and user space. Therefore, it is required that your code map/unmap and switch MMU pages to make sure that all user space lives in user space, and that all kernel space lives in kernel space.

## Powerpoints

- [Operating Systems](class_notes/COSC361-Operating_Systems.pdf)
- [Booting](class_notes/COSC361-Booting.pdf)
- [Drivers and Hardware](class_notes/COSC361-Drivers_and_Hardware.pdf)
- [System Calls](class_notes/COSC361-System_Calls.pdf)
- [Processes](class_notes/COSC361-Processes.pdf)
- [Scheduling](class_notes/COSC361-Scheduling.pdf)
- [Concurrency](class_notes/COSC361-Concurrency.pdf)