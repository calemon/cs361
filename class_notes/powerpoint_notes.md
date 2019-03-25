# Powerpoint Notes

## Booting

Booting = process of getting the machine into the OS' state
- Cold boot = power off -> power on
- Warm boot = power on -> power on
  - Soft reset/Restart

### PC

Power applied to motherboard -> Motherboard provides power to CPU and all its components -> CPU executes the BIOS (or the newer UEFI) -> BIOS transfers control to the bootloader -> Bootloader loads the operating system 

### Getting to the OS

1) A "boot" CPU is picked and the rest are put into a waiting loop.
2) Data is copied from storage to RAM
3) Trap vector is set for system calls and system faults
4) Jump to high-level code (C, C++, Rust)
   1) Initializes devices
   2) Sets up graphics/screen
   3) Enables interrupts
   4) Start OS loop

## Operating Systems

**Operating Systems** - Software that marshals between hardware and software

**Kernel** - The "executable" containing all OS code

**Input** - data from hardware into operating system

**Output** - data from operating system to hardware

### Abstraction

Operating systems provide an interface for applications to use

- Having an interface takes away the need to code/program toward a specific CPU/machine
- OS deals with the small details of translating high-level code to CPU/machine specific code
    - User space is free just to have a standard interface to use

### Virtualization

The operating system can create an "illusion" for programs. 

- Uniprocessor systems can be given the illusion of a multiprocessor system through the use of **virtualization**
- OS can schedule which process gets which process
- OS can make all file systems look the same

### Services

OS may offer services such as priorities on tasks, network stack, and more

OS can act as a resource manager (especially in multi-user environments)

- E.G. Users A and B write to the hard disk, which user will be allowed to write first? OS will decide which is best/most efficient

### Types

Several design choices can be made for an OS

- Monolithic Kernel: The entire OS is in "kernel space" and runs in supervisor mode (privileged CPU mode)
- Micro Kernel: User services (typically made by system calls) are in user space, even if controlled by the kernel
- Hybrid Kernel: Combination of above two

### CPU Modes

The kernel prevents the user from executing privileged instructions by having different CPU modes

- User Mode: The least privileged mode. 
  - Can execute the smallest subset of instructions.
  - Cannot change memory maps
- Supervisor Mode: a privileged mode, typically used by the kernel. 
  - Can allocate/deallocate memory maps. 
  - Called "kernel mode"
- Machine Mode: the highest privileged mode
  - typically reserved for reconfiguring CPU systems.

Supervisor mode can switch itself into user mode

User mode must use a system call or the hardware must interrupt the CPU to elevate its mode.

#### Hypervisor Mode
Some CPUs support hypervisor mode. Typically used for supporting multiple virtual machines. CPU is responsible for allocating resources and virtualizing hardware.

