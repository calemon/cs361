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

