/* Lab 4
 * mmu.cpp
 * MMU Lab Template by Stephen Marz
 * Casey Lemon
 * 8 March 2019
 */
#include <mmu.h>

//#define DEBUG

/* Custom functions */
static uint32_t get_pn1(uint32_t value);
static uint32_t get_pn0(uint32_t value);
static uint32_t create_pte(uint32_t ppn1, uint32_t ppn0, bool is_program, uint32_t mode, bool is_level0);
static void set_bit(uint32_t *value, uint32_t bit_index);
static void clear_bit(uint32_t *value, uint32_t bit_index);
static bool test_bit(const uint32_t *bitset, int bit_index);
static void print_line_and_hex(char *pre, uint32_t value, bool newline = true);
static void print_surrounding_entries(uint32_t *entry_address, uint32_t num_entries_after);

extern uint32_t MMU_TABLE[MMU_TABLE_SIZE];

const uint32_t SP_REG = 2;
const uint32_t TABLE_SIZE = 4096;

char str[64];

void mmu_disable(){
    #ifdef DEBUG
    uint32_t before = get_satp();
    #endif
    /* Get SATP register */
    uint32_t satp_reg = get_satp();
    /* Set MODE bit to 0 = Bare = No translation/protection between virtual and physical addresses */
    clear_bit(&satp_reg, 31);
    /* Set SATP register again */
    set_satp(satp_reg);

    #ifdef DEBUG
    //print_line_and_hex("MMU_DISABLE - SATP Register changed from ", before, false);
    //print_line_and_hex(" to ", get_satp());
    #endif
}

void mmu_enable(PROCESS *p){
    #ifdef DEBUG
    uint32_t before = get_satp();
    #endif
    uint32_t satp_reg = 0;

    /* Null mmu table so mmu_map has not been called */
    if(p->mmu_table == 0) return;

    /* Get satp_reg, set lower 21 bits to p->mmu_table and set the mode to 1 */
    satp_reg |= (p->mmu_table >> 12);
    set_bit(&satp_reg, 31);
    set_satp(satp_reg);

    #ifdef DEBUG
    print_line_and_hex("MMU_ENABLE - SATP Register changed from ", before, false);
    print_line_and_hex(" to ", get_satp());
    print_line_and_hex("    p->mmu_table = ", p->mmu_table);
    #endif
}

void mmu_map(PROCESS *p){
    #ifdef DEBUG
    print_line_and_hex("\nMAPPING PROCESS for ", p->pid);
    #endif

    uint32_t program, program_end, stack, stack_end, test;
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
    level0_address = root + TABLE_SIZE;

    #ifdef DEBUG
    write_stringln("");
    print_line_and_hex("root: ", (uint32_t) root);
    print_line_and_hex("level0_address: ", (uint32_t) level0_address);
    print_line_and_hex("program: ", program, false);
    print_line_and_hex(" and program_end: ", program_end);
    print_line_and_hex("p_vpn1: ", p_vpn1);
    print_line_and_hex("p_vpn0: ", p_vpn0);

    print_line_and_hex("stack: ", stack, false);
    print_line_and_hex(" and stack_end: ", stack_end);
    print_line_and_hex("s_vpn1: ", s_vpn1);
    print_line_and_hex("s_vpn0: ", s_vpn0);
    write_stringln("");
    #endif

    /* Adjust 34-bit physical address to 32-bit PTE and set level 1 entry to address of level 0 table */
    root[p_vpn1] = create_pte(get_pn1((uint32_t) level0_address), get_pn0((uint32_t) level0_address), true, p->mode, false);
    #ifdef DEBUG
    print_line_and_hex("entry for root[p_vpn1]: ", root[p_vpn1]);
    #endif

    /* Insert PTEs for level 0 entries */
    #ifdef DEBUG
    write_stringln("\nLevel 0 entries for program:");
    #endif
    for(uint32_t i = p_vpn0; program <= program_end; i++, program += TABLE_SIZE){
        level0_address[i] = create_pte(get_pn1(program), get_pn0(program), true, p->mode, true);
        #ifdef DEBUG
        print_line_and_hex("Mapping entry program ", program, false);
        print_line_and_hex(" for level0_address[", i, false);
        print_line_and_hex("]: ", level0_address[i]);
        #endif
    }

    /* Check if vpn1 was the same for both the program and stack, if it wasn't then create a new PTE */
    if(test_bit(&root[s_vpn1], 0) == false){
        #ifdef DEBUG
        write_stringln("have to create new entry for level 1");
        #endif
        root[s_vpn1] = create_pte(get_pn1((uint32_t) level0_address), get_pn0((uint32_t) level0_address), false, p->mode, false);
    }

    #ifdef DEBUG
    write_stringln("\nLevel 0 entries for stack:");
    #endif

    for(uint32_t i = s_vpn0; stack <= stack_end; i++, stack += TABLE_SIZE){
        level0_address[i] = create_pte(get_pn1(stack), get_pn0(stack), false, p->mode, true);
        #ifdef DEBUG
        print_line_and_hex("Mapping entry stack ", stack, false);
        print_line_and_hex(" for level0_address[", i, false);
        print_line_and_hex("]: ", level0_address[i]);
        #endif
    }

    #ifdef DEBUG
    write_stringln("\nPrinting level 1 table entries...");
    print_surrounding_entries(root, 1024);
    write_stringln("\nPrinting level 0 table entries...");
    print_surrounding_entries(level0_address, 1024);
    #endif
}

void mmu_unmap(PROCESS *p){
    #ifdef DEBUG
    print_line_and_hex("\nUNMAPPING PROCESS for ", p->pid);
    #endif

    uint32_t program, program_end, stack, stack_end, test;
    uint32_t *root, *level0_address;

    program = (uint32_t) p->program & ~(0xfff);
    program_end = (program + (TABLE_SIZE - 1) + TABLE_SIZE) & ~(0xfff);
    stack = (p->regs[SP_REG]) & ~(0xfff); // NEED TO FIX
    stack_end = (stack + (TABLE_SIZE - 1) + (2 * TABLE_SIZE)) & ~(0xfff);
    
    /* Find process's root space then align it; set p->mmu_table to root */
    root = MMU_TABLE + (1024 * (p->pid - 1) * 10);
    root = (uint32_t *) (((uint32_t) root + (TABLE_SIZE - 1)) & -TABLE_SIZE);
    
    /* Set p's mmu_table to the aligned address of root */
    p->mmu_table = (uint32_t) root;

    uint32_t p_vpn1 = get_pn1(program);
    uint32_t p_vpn0 = get_pn0(program);
    uint32_t s_vpn1 = get_pn1(stack);
    uint32_t s_vpn0 = get_pn0(stack);

    /* Get address of level 0 table and adjust it to correct bit indices */
    level0_address = (uint32_t *) ((root[p_vpn1] & ~(0x3ff)) << 2);

    /* Set valid bit at root[p_vpn1] to 0 */
    root[p_vpn1] &= ~(0x1);

    #ifdef DEBUG
    print_line_and_hex("root[p_vpn1]: ", root[p_vpn1]);
    print_line_and_hex("level0_address from table 1 entry: ", (uint32_t) level0_address);
    #endif
    
    /* Make level 0 program entries invalid */
    for(uint32_t i = p_vpn0; program <= program_end; i++, program += TABLE_SIZE) level0_address[i] &= ~(0x1);

    /* Check if vpn1 was the same for both the program and stack, if it wasn't then make stack's vpn1 entry invalid */
    if(test_bit(&root[s_vpn1], 0) == true){
        /* Set valid bit at root[s_vpn1] to 0 */
        root[p_vpn1] &= ~(0x1);
    }

    /* Make level 0 stack entries invalid */
    for(uint32_t i = s_vpn0; stack <= stack_end; i++, stack += TABLE_SIZE) level0_address[i] &= ~(0x1);

    #ifdef DEBUG
    write_stringln("\nPrinting level 1 table entries...");
    print_surrounding_entries(root, 1024);
    write_stringln("\nPrinting level 0 table entries...");
    print_surrounding_entries(level0_address, 1024);
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
    for(volatile int i = 0; i < 10000; i++){
        for(volatile int j = 0; j < 100000; j++);
        if(i >= 5000){
            ecall(SYS_SET_QUANTUM, 9);
            ecall(SYS_SLEEP, 1);
        } else if(i >= 1000){ 
            ecall(SYS_SET_QUANTUM, 5);
            ecall(SYS_SLEEP, 1);
        } else if(i >= 100){ 
            ecall(SYS_SET_QUANTUM, 3);
        }
    }
    ecall(SYS_EXIT, 0);
}

void test_fcn2(){
    for(volatile int i = 0; i < 20; i++){
        ecall(SYS_SLEEP, 1);
        if(i >= 15) ecall(SYS_SET_QUANTUM, 9);
        else if (i >= 10) ecall(SYS_SET_QUANTUM, 6);
        else if (i >= 5) ecall(SYS_SET_QUANTUM, 3);
    }
    ecall(SYS_EXIT, 0);
}

void test(){
	// Put whatever you want to test here.
    //new_process(hello, 0, 0, MACHINE);
    //new_process(hello, 0, 0, SUPERVISOR);
    //new_process(hello, 0, 0, USER);
    new_process(test_fcn1, -10, 0, USER);  
    new_process(test_fcn2, -10, 0, USER);
    new_process(test_fcn2, -10, 0, MACHINE);
    new_process(test_fcn2, -10, 0, SUPERVISOR);
}







/* Custom functions */
static uint32_t get_pn1(uint32_t value){
    uint32_t ret_val = (value >> 22) & 0x3ff;
    #ifdef DEBUG
    //print_line_and_hex("\tget_pn1() returning: ", ret_val);
    #endif
    return ret_val;
}

static uint32_t get_pn0(uint32_t value){
    uint32_t ret_val = (value >> 12) & 0x3ff;
    #ifdef DEBUG
    //print_line_and_hex("\tget_pn0() returning: ", ret_val);
    #endif
    return ret_val;
}

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

static void set_bit(uint32_t *value, uint32_t bit_index){
    /* Left shift 1 by the given bit_index then OR the given value with the shifted bits */
    *value |= 1 << bit_index;
}

static void clear_bit(uint32_t *value, uint32_t bit_index){
    /* Left shift 1 by the given bit_index, and flip the result. Then AND the given value with the shifted bits to clear */
    *value &= ~(1 << bit_index);
}

static bool test_bit(const uint32_t *bitset, int bit_index){
    return (((*bitset >> bit_index) & 1) == 1) ? true : false; 
}

static void print_line_and_hex(char *pre, uint32_t value, bool newline){
    char hex_val[64];
    write_string(pre);
    hex_to_string(hex_val, value);
    if(newline) write_stringln(hex_val);
    else write_string(hex_val);
}

static void print_surrounding_entries(uint32_t *entry_address, uint32_t num_entries_after){
    write_stringln(" ================= Entries ================= ");
    print_line_and_hex("Entry = ", (uint32_t) entry_address);
    for(uint32_t i = 0; i < num_entries_after; i++){
        if(entry_address[i] != 0){
            print_line_and_hex("  Address at entry[", i, false);
            print_line_and_hex("] = ", entry_address[i]);
        }
    }
    write_stringln(" =========================================== ");
}