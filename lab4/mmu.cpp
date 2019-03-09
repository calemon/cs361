/* Lab 4
 * mmu.cpp
 * MMU Lab Template by Stephen Marz
 * Casey Lemon
 * 15 March 2019
 */
#include <mmu.h>

//#define MY_DEBUG

/* Custom functions */
static uint32_t get_pn1(uint32_t value);
static uint32_t get_pn0(uint32_t value);
static uint32_t create_pte(uint32_t ppn1, uint32_t ppn0, bool is_program, uint32_t mode, bool is_level0);
static bool test_bit(const uint32_t *bitset, int bit_index);
static void print_line_and_hex(char *pre, uint32_t value, bool newline = true);
static void print_entries(uint32_t *entry_address, uint32_t num_entries_after);

extern uint32_t MMU_TABLE[MMU_TABLE_SIZE];

const uint32_t SP_REG = 2;
const uint32_t TABLE_SIZE = 4096;

void mmu_disable(){
    /* Get SATP register */
    uint32_t satp_reg = get_satp();
    /* Clear MODE bit (bit 31) to 0 = Bare = No translation/protection between virtual and physical addresses */
    satp_reg &= ~(1 << 31);
    /* Set SATP register to new value */
    set_satp(satp_reg);
}

void mmu_enable(PROCESS *p){
    #ifdef MY_DEBUG
    uint32_t before = get_satp();
    #endif
    uint32_t satp_reg = 0;

    /* Null mmu table so mmu_map has not been called */
    if(p->mmu_table == 0) return;

    /* Get satp_reg, set lower 21 bits to p->mmu_table and set the mode to 1 */
    satp_reg |= (p->mmu_table >> 12);
    satp_reg |= 1 << 31;
    set_satp(satp_reg);

    #ifdef MY_DEBUG
    print_line_and_hex("MMU_ENABLE - SATP Register changed from ", before, false);
    print_line_and_hex(" to ", get_satp());
    print_line_and_hex("    p->mmu_table = ", p->mmu_table);
    #endif
}

void mmu_map(PROCESS *p){
    #ifdef MY_DEBUG
    print_line_and_hex("\nMAPPING PROCESS for ", p->pid);
    #endif

    uint32_t program, program_end, stack, stack_end;
    uint32_t *root, *level0_address;

    /* Calculate program address, program end address, stack, and stack end address */
    program = (uint32_t) p->program & ~(0xfff);
    program_end = (program + (TABLE_SIZE - 1) + (2 * TABLE_SIZE)) & ~(0xfff);
    stack = (p->regs[SP_REG]) & ~(0xfff);
    stack_end = (stack + (TABLE_SIZE - 1) + (3 * TABLE_SIZE)) & ~(0xfff);
    
    /* Find process's root space then align it; set p->mmu_table to root */
    root = MMU_TABLE + (1024 * (p->pid - 1) * 10);
    root = (uint32_t *) (((uint32_t) root + (TABLE_SIZE - 1)) & -TABLE_SIZE);
    
    /* Set p's mmu_table to the aligned address of root */
    p->mmu_table = (uint32_t) root;

    /* Get vpn1 and vpn0 of both the program and the stack */
    uint32_t p_vpn1 = get_pn1(program);
    uint32_t p_vpn0 = get_pn0(program);
    uint32_t s_vpn1 = get_pn1(stack);
    uint32_t s_vpn0 = get_pn0(stack);

    /* Calculate the level 0 page table */
    level0_address = root + 1024; // Adding 1024 because root is a pointer so 1024 * 4 = 4096

    /* Adjust 34-bit physical address to 32-bit PTE and set level 1 entry to address of level 0 table */
    root[p_vpn1] = create_pte(get_pn1((uint32_t) level0_address), get_pn0((uint32_t) level0_address), true, p->mode, false);

    /* Insert PTEs for level 0 entries for program address */
    for(uint32_t i = p_vpn0; program <= program_end; i++, program += TABLE_SIZE){
        level0_address[i] = create_pte(get_pn1(program), get_pn0(program), true, p->mode, true);
    }

    /* Check if vpn1 was the same for both the program and stack, if it wasn't then create a new PTE */
    if(test_bit(&root[s_vpn1], 0) == false){
        #ifdef MY_DEBUG
        write_stringln("have to create new entry for level 1");
        #endif
        root[s_vpn1] = create_pte(get_pn1((uint32_t) level0_address), get_pn0((uint32_t) level0_address), false, p->mode, false);
    }

    /* Insert PTEs for level 0 entries for stack address */
    for(uint32_t i = s_vpn0; stack <= stack_end; i++, stack += TABLE_SIZE){
        level0_address[i] = create_pte(get_pn1(stack), get_pn0(stack), false, p->mode, true);
    }

    #ifdef MY_DEBUG
    write_stringln("\nPrinting level 1 table entries...");
    print_entries(root, 1024);
    write_stringln("\nPrinting level 0 table entries...");
    print_entries(level0_address, 1024);
    #endif
}

void mmu_unmap(PROCESS *p){
    #ifdef MY_DEBUG
    print_line_and_hex("\nUNMAPPING PROCESS for ", p->pid);
    #endif

    uint32_t program, program_end, stack, stack_end, test;
    uint32_t *root, *level0_address;

    /* Calculate program address, program end address, stack, and stack end address */
    program = (uint32_t) p->program & ~(0xfff);
    program_end = (program + (TABLE_SIZE - 1) + (2 * TABLE_SIZE)) & ~(0xfff);
    stack = (p->regs[SP_REG]) & ~(0xfff);
    stack_end = (stack + (TABLE_SIZE - 1) + (3 * TABLE_SIZE)) & ~(0xfff);
        
    /* Get level 1 root from p->mmu_table */
    root = (uint32_t *) p->mmu_table;

    uint32_t p_vpn1 = get_pn1(program);
    uint32_t p_vpn0 = get_pn0(program);
    uint32_t s_vpn1 = get_pn1(stack);
    uint32_t s_vpn0 = get_pn0(stack);

    /* Get address of level 0 table and adjust it to correct bit indices (32-bit number to 34-bit) */
    level0_address = (uint32_t *) ((root[p_vpn1] & ~(0x3ff)) << 2);

    /* Set valid bit at root[p_vpn1] to 0 */
    root[p_vpn1] &= ~(0x1);
    
    /* Make level 0 program entries invalid */
    for(uint32_t i = p_vpn0; program <= program_end; i++, program += TABLE_SIZE) level0_address[i] &= ~(0x1);

    /* Check if vpn1 was the same for both the program and stack, if it wasn't then make stack's vpn1 entry invalid */
    if(test_bit(&root[s_vpn1], 0) == true){
        /* Set valid bit at root[s_vpn1] to 0 */
        root[s_vpn1] &= ~(0x1);
    }

    /* Make level 0 stack entries invalid */
    for(uint32_t i = s_vpn0; stack <= stack_end; i++, stack += TABLE_SIZE) level0_address[i] &= ~(0x1);

    #ifdef MY_DEBUG
    write_stringln("\nPrinting level 1 table entries...");
    print_entries(root, 1024);
    write_stringln("\nPrinting level 0 table entries...");
    print_entries(level0_address, 1024);
    #endif
}

void hello(){
	//This is sample code. This will run the process for 10,000,000 iterations
	//and then sleep for 5 seconds over and over again.
    ecall(SYS_SET_QUANTUM, 10);
    do {
        for (volatile int i = 0;i < 10000000;i++);
        ecall(SYS_SLEEP, 5);
    } while(1);
}

void test_fcn1(){
    for(volatile int i = 0; i < 1000000; i++){
        for(volatile int j = 0; j < 10000; j++);
        if(i >= 750000){
            ecall(SYS_SET_QUANTUM, 9);
        } else if(i >= 250000){ 
            ecall(SYS_SET_QUANTUM, 5);
        } else if(i >= 10000){ 
            ecall(SYS_SET_QUANTUM, 3);
        }
    }
    ecall(SYS_EXIT, 0);
}

void test_fcn2(){
    for(volatile int i = 0; i < 20; i++){
        ecall(SYS_SLEEP, 1);
        if(i >= 15) ecall(SYS_SET_QUANTUM, 5);
        else if (i >= 10) ecall(SYS_SET_QUANTUM, 4);
        else if (i >= 5) ecall(SYS_SET_QUANTUM, 2);
    }
    ecall(SYS_EXIT, 0);
}

void test_fcn3(){
    int list[] = {233, 4232, 235, 2339, 1237, 5468, 1};
    int size = 7;

    /* Bubble sort */
    for(volatile int i = 0; i < size; i++){
        for (volatile int j = 0; j < size-i-1; j++){
            if (list[j] > list[j+1]){
                int tmp = list[j];
                list[j] = list[j+1];
                list[j+1] = tmp;
            }
            uint32_t k = list[i] % 10;
            ecall(SYS_SET_QUANTUM, k);
        }
        ecall(SYS_SLEEP, 1);
    }

    /* Revere the order */
    for(volatile int i = 0, j = size - 1; i < size / 2; i++, j--){
        int tmp = list[i];
        list[i] = list[j];
        list[j] = tmp;
        uint32_t k = (list[i] % 10) * 2;
        ecall(SYS_SET_QUANTUM, k);
        ecall(SYS_SLEEP, 1);
    }

    ecall(SYS_SET_QUANTUM, 1);
    /* Halve all entries */
    for(volatile int i = 0; i < size; i++){
        list[i] /= 2;
        ecall(SYS_SLEEP, 1);
    }

    ecall(SYS_SET_QUANTUM, 3);
    /* Add every other entry to i */
    for(volatile int i = 0; i < size; i++){
        for(volatile int j = 0; j < size; j++){
            if(i % 2 == 0 && j % 2 == 0){
                list[i] += list[j];
            } else if(i % 2 == 1 && j % 2 == 1){
                list[i] += list[j];
            }
            ecall(SYS_SLEEP, 1);
        }
    }

    ecall(SYS_EXIT, 0);
}

void test(){
	// Put whatever you want to test here.
    new_process(hello, -7, 1, USER);
    new_process(hello, 0, 2, USER);
    new_process(test_fcn1, -6, 0, USER);
    new_process(test_fcn2, -1, 0, USER);
    new_process(test_fcn3, 1, 0, USER);
    new_process(test_fcn3, -5, 0, USER);
    new_process(test_fcn3, 6, 0, USER);
}

/* Custom functions */
/* Get PN[1] of a physical or virtual address */
static uint32_t get_pn1(uint32_t value){
    uint32_t ret_val = (value >> 22) & 0x3ff;
    #ifdef MY_DEBUG
    //print_line_and_hex("\tget_pn1() returning: ", ret_val);
    #endif
    return ret_val;
}

/* Get PN[0] of a physical or virtual address */
static uint32_t get_pn0(uint32_t value){
    uint32_t ret_val = (value >> 12) & 0x3ff;
    #ifdef MY_DEBUG
    //print_line_and_hex("\tget_pn0() returning: ", ret_val);
    #endif
    return ret_val;
}

/* Create a Page Table Entry according to the RISCV sv32 privileged architecture document
 * Bits: | 31 - 20 | 19 - 10 | 9 - 8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 * Use:  | PPN[1]  | PPN[0]  | RSW   | D | A | G | U | X | W | R | V |
 */
static uint32_t create_pte(uint32_t ppn1, uint32_t ppn0, bool is_program, uint32_t mode, bool is_level0){
    uint32_t ret_val = 0;
    uint32_t perms = 0;

    /* At level 0 table so one or more of R/W/X need to be set, else just the valid bit will be set */
    if(is_level0){
        if(is_program) perms = 0xb;
        else perms = 0x7;
    } else {
        perms = 0x1;
    }

    /* Combine ppn1, ppn0, and perms */
    ret_val |= (ppn1 << 20) | (ppn0 << 10) | perms;

    /* If process is in user mode, set U bit, else clear U bit */
    if(mode == 0) ret_val |= 1 << 4;
    else ret_val &= ~(1 << 4);

    return ret_val;
}

static bool test_bit(const uint32_t *bitset, int bit_index){
    return (((*bitset >> bit_index) & 1) == 1) ? true : false; 
}

/* Print a string (pre) before the given value */
static void print_line_and_hex(char *pre, uint32_t value, bool newline){
    char hex_val[64];
    write_string(pre);
    hex_to_string(hex_val, value);
    if(newline) write_stringln(hex_val);
    else write_string(hex_val);
}

/* Prints all num_entries_after and including the given entry_address */
static void print_entries(uint32_t *entry_address, uint32_t num_entries_after){
    write_stringln(" ================= Entries ================= ");
    print_line_and_hex("Entry = ", (uint32_t) entry_address);
    for(uint32_t i = 0; i < num_entries_after; i++){
        if(entry_address[i] != 0){
            /* If an entry isn't set to valid, unmapping was incorrect/failed so indicate it */
            if(test_bit(&entry_address[i], 0)) write_string("  * ");
            print_line_and_hex("  Address at entry[", i, false);
            print_line_and_hex("] (", (uint32_t) &entry_address[i], false);
            print_line_and_hex(") = ", entry_address[i]);
        }
    }
    write_stringln(" =========================================== ");
}