# Lab 4

I created a video where I show on pencil-and-paper how to create a root table, decompose virtual addresses and physical addresses, and how to create a second level page table: [here](http://dmsmediasite.utk.edu/UTK/Play/bee3e6c760314471aaa34e14344e31161d) (Links to an external site.)Links to an external site.

## Overview
You will be writing a few functions required to implement the Sv32 memory management unit. This time, MZOS has both kernel and user space. Therefore, it is required that your code map/unmap and switch MMU pages to make sure that all user space lives in user space, and that all kernel space lives in kernel space.

[1] Notice that the process control block now contains the field mmu_table. This is an address in memory where the root of your table will go. Each process has its own page tables.

[1] You will be identity mapping (virtual address will be equal to the physical address) all of your addresses, but the point of the lab is to gain experience programming an MMU.

The SATP and Sv32 MMU strategies can be found here: [riscv-mmu.pdf](riscv-mmu.pdf)

## Files
```bash
Makefile
libmzos.a
virt.lds
mmu.h
mmu.cpp
```

## Assignment
You will need to write the following functions to perform the given operations in mmu.cpp:

```cpp
void mmu_disable()
```
This function will turn off the MMU by changing the SATP's mode. DO NOT change anything other than the mode bit!

```cpp
void mmu_enable(PROCESS *p)
```
[1] This function will enable the MMU for the given process. Remember, there is a context switching mechanism, so your mmu_enable must handle any process that needs to be enabled. This function does not build the page tables. That will be done later using mmu_map. Instead, this function is used to program the SATP register so that the process pointed to by *p's page tables are correctly loaded before it runs.

This function must do nothing if p->mmu_table is == 0. A table that is NULL means that mmu_map has not been called.

```cpp
void mmu_map(PROCESS *p)
```
This function will map a process' program pointer and stack pointer into the MMU. This is typically used for USER mode processes, but may also be used for SUPERVISOR processes. NOTE: This function does NOT change SATP. Instead, it will write the root table into p->mmu_table. You will notice that there is a global array, which is essentially a pot of memory you are free to create your page tables in. This function is responsible for building the page tables in RAM and setting the root of those page tables into p->mmu_table. Again, this function does NOT change SATP...that's the responsibility of your mmu_enable() function.

```cpp
void mmu_unmap(PROCESS *p)
```
This function will completely unmap a process from the MMU. Essentially, it needs to free the memory associated with p->mmu_table by making those entries non-valid (V bit must be 0).

[1] Since the page table array is statically allocated, you will not be freeing the entries as your normally would using dynamic memory functions, such as free or delete. Instead, all you need to do is invalid every entry of your page tables. Remember that bit index 0 of a page table entry is the V bit (Valid). If this is set to 0, then that page table entry is "freed" in the loosest sense of the word.

```cpp
void test()
```
This function is called from the init() process. You can use new_process (prototyped in mmu.h) to create new processes to test your code. My operating system will automatically call your mmu_enable(), mmu_map(), mmu_unmap(), and mmu_disable() functions. Therefore, all you need to do is write the functionality, and my OS will take care of calling these functions at the appropriate time.

## Programming the MMU
You will program the MMU by adding page tables into memory, and then setting the SATP register to point to the root table in memory. NOTE: The last 12 bits of your tables' physical address must be 0! Notice, you only store the upper 20 bits of the table in the SATP register. Finally, bit 31 is the MODE field of the SATP register. If this is 0, the MMU is turned off. Otherwise, if this is 1, the MMU is activated.

### *SATP Register*
![sv32_satp_register.jpg](images/sv32_satp_register.jpg)
*WARL means "Write Any Values, Reads Legal Values", which essentially means that you can write any value and then see what the register actually took by reading it back.

### *Virtual / Physical / Page Entry*
![](images/sv32_phys_virt_table_entry.png.jpg)

### *RWX Fields*
![](images/sv32_pte_rwx_fields.jpg)

## Example

### mmu_map

Say we're given a process *p. For this lab, you're required to map p->program as well as p->regs[SP] (the stack).

1. You need to select free space in your statically allocated array called MMU_TABLE. There is enough free space in this table for alignment purposes. To select free space, devise a scheme to divide up the MMU_TABLE for MAX_PROCESSES (defined in mmu.h) number of processes, much like you did for the stack pointer in the scheduler lab.
   - When you find this scheme set a pointer *space to the top of the allocated table. Notice that the page tables are not configured like a stack; therefore, the memory starts from the low memory addresses and works to higher memory addresses. When you have the *space pointer, you need to make sure it is aligned to 4096. You're not required to use the alignment formula below, but I don't see a reason you shouldn't use it.
```cpp
aligned_space = (space + (ALIGN_TO - 1)) & -ALIGN_TO;
```
- In this function, ALIGN_TO is the value that you want to align your number to. Notice that this formula always adds space. Therefore, you're guaranteed that aligned_space is never < space.

2. Now you have a properly aligned pointer to free memory space. This pointer now points to your root page table entry. You will have two levels of page tables. The first level will contain no leaves, but every single page table contains 1024 entries, each 4 bytes a piece. To determine which of the 1024 entries will need to be mapped, you will need to decompose your program pointer and stack pointer using Figure 4.13 shown above. VPN[1] will contain the array index into your first level page table, and VPN[0] will contain the array index into your second level page table.

3. When you find the entry for VPN[1], you need to write an entry that looks like Figure 4.14. You must make sure that the V bit is set and that RWX are NOT set, otherwise, the MMU will think this is a leaf entry. The values of PPN[1] and PPN[0] will be the memory address of your second-level page table. Notice that your second level page table's memory address must be aligned by 210.

4. Now that you have the second level page table, you need to create another 1024 entries (not all will be valid). Decompose using Figure 4.13 to determine VPN[0]. This will be the index into your second level page table where you need to finally store the translation's physical address.

5. Notice that the shift amounts for the page table entry are different than the shift amounts for the actual physical address. Your page table entry needs to account for this 2-bit shift difference. Also, since this is the last level of your page tables, you need to ensure that the RWX bits are properly set. Try to reason which bits should be set and which should be not set. For example, while it is possible to run CPU instructions from the stack, is that really a good idea? Finally, do not forget to set the valid bit, otherwise no translation will be possible. The same goes for the U-bit. Notice that the process control block has an 8-bit mode. The comments in mmu.h show what these mean. The U-bit means "user", and machine mode and supervisor mode are not considered user mode.

6. After you've created your two level page tables, you now have a working page table. You may need to decompose other addresses and store more entries into different indices.

7. You now have p->mmu_table pointing to a valid page table. When my code picks that process to run, it will call your mmu_enable so that your code will properly put the process' MMU table memory address into the SATP register.


## Hints and Restrictions

1. You will notice a global table called MMU_TABLE. This is where your MMU tables will go. Remember, the SATP register only stores the upper 20 bits of the root table's memory address. Therefore, you must align the memory address so that the last 12 bits are 0 (32-12 = 20).

2. For mmu_map, p->program points to the start of the program's memory address. You are guaranteed that no program will exceed 4096 bytes (1 page), except perhaps ones that you make. However, you are not guaranteed that you only need to map 1 page. For example, if my program starts at 0x8000_2fff, it can potentially reach 0x8000_3fff. However, in this case, you will need to map 2, 4-kb pages. One for 0x8000_2xxx and one for 0x8000_3xxx.

3. The stack size has the same restriction as the program pointer. However, the stack size for every program is exactly 8192 bytes, which may be 2 or more pages.

4. You may see page fault error messages should you not map correctly. (See Output section)

5. Do NOT let processes share MMU_TABLES. Even though this is good practice to coalesce adjoining pages, you will not be doing this.

6. Do NOT use 4MiB super pages. All of your pages will be 4KiB pages.

7. You must use the Sv32 (32-bit MMU) system.

8. Your address space identifier (ASID) must always be 0.

## Outputs

Page faults can arise from three different ways: IPF (Instruction Fetch Page Fault), LPF (Load Page Fault), or SPF (Store Page Fault). These will be denoted with a #IPF, #LPF, or #SPF.
```
Welcome to Marz OS (Lab 4)
 e - Make ECALL.
 p - Process list (as USER).
 q - Process list (as MACHINE).
 t - Top (as USER).
 u - Top (as MACHINE).
 0 - Run YOUR test() function.
Enter command: Killing process PID 5. => #IPF (mtval = 0x80000324)
```
In the code above, I typed 0, which created a user process, but I did not map the program pointer correctly. You can see the program pointer is 0x8000_0324. You know that this is a problem with the program pointer and not the stack since it is an #IPF (Instruction Fetch Page Fault). Otherwise, it will be an #LPF or #SPF depending on whether the stack was loaded from or stored to.

### *Process List*
![](images/process_list.png)
This screenshot shows twelve processes. You will notice that my code has the same MMU TABLE address. This is because my code coalesces like entries. However, your code will not do this. Instead, each USER process must have a distinct MMU TABLE address.

Each column represents the following:
| Name | Description |
| ---- | ----- |
| PID | The process ID. Currently, this can be 1 through 15. No more than 15 processes are allowed. |
| STATE | The state of the process: RUNNING or SLEEPING |
| QM | Quantum multiplier. This is the amount of additional runtime a process gets per switch. |
| PR | Priority. Unused for round robin scheduling. |
| SL | Sleep timer. The process in SLEEPING mode will sleep until the timer reaches this value. |
| SW | The number of context switches this process has received. |
| RT | The amount of CPU time this process has received. |
| PRIV. MODE | The privilege mode the process is running: MACHINE, SUPERVISOR, or USER |
| MMU TABLE | The address of the MMU table for this process.
| NAME | The name of this process. |

## Compiling and Running

Compile and run your program using the following command:
```bash
make run
```

## References
[riscv-mmu.pdf](riscv-mmu.pdf)

## Submission

Submit your .cpp file. Your solution must compile and run on the Hydra machines.