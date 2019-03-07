/* Lab 4
 * mmu.cpp
 * MMU Lab Template by Stephen Marz
 * Casey Lemon
 * 8 March 2019
 */
#include <mmu.h>

/* Custom functions */
static uint32_t get_pn1(uint32_t value);
static uint32_t get_pn0(uint32_t value);
static uint32_t create_pte(uint32_t ppn1, uint32_t ppn0, bool is_program, uint32_t mode, bool is_level0);
static void set_bit(uint32_t *value, uint32_t bit_index);
static void clear_bit(uint32_t *value, uint32_t bit_index);
static bool test_bit(const uint32_t *bitset, int bit_index);
static void print_line_and_hex(char *pre, uint32_t value, bool newline = true);
static void print_process_table(PROCESS *p);

extern uint32_t MMU_TABLE[MMU_TABLE_SIZE];

const uint32_t ALIGN_TO = 4096;
const uint32_t SP_REG = 2;
const uint32_t TABLE_SIZE = 4096;

char str[64];

void mmu_disable(){
    /* Get SATP register */
    uint32_t satp_reg = get_satp();
    /* Set MODE bit to 0 = Bare = No translation/protection between virtual and physical addresses */
    clear_bit(&satp_reg, 31);
    /* Set SATP register again */
    set_satp(satp_reg);
}

void mmu_enable(PROCESS *p){
    uint32_t satp_reg;

    /* Null mmu table so mmu_map has not been called */
    if(p->mmu_table == 0) return;

    /* Get satp_reg, set lower 21 bits to p->mmu_table and set the mode to 1 */
    satp_reg = get_satp();
    satp_reg |= (p->mmu_table >> 12);
    set_bit(&satp_reg, 31);
    set_satp(satp_reg);
}

void mmu_map(PROCESS *p){
    /*
    1) Map Stack
        1) Find the root and apply it properly
        2) Add VPN[1] to root to get the 4 byte entry
            - It has PPN[1], PPN[0], URWX and V bits
            - PPN[1] will be the same as VPN[1]/VPN[0]
        3) The value at root + VPN[1] will be the memory address of the second-level (second_root) page table
        4) Add VPN[0] to second_root (second_root + VPN[0]) to get to entry
    2) Map Program
        Same as stack

    if mapping program or stack second, check if for root + vpn[1] if the valid bit is 1, if it is follow it
    Put in level 0 pte the program >> 2?

    while(){
        level 1 stuff
        while(){
            level 0 stuff
        }
    }

    Functions:
    - get_pn1()
    - get_pn0()
    */
    uint32_t program, program_end, stack, stack_end, test;
    uint32_t *root, *program_ptr;
    bool mapped_program = false, mapped_stack = true;

    program = (uint32_t) p->program & ~(0xfff);
    program_end = (program + (ALIGN_TO - 1) + 4096) & ~(0xfff);
    stack = (p->regs[SP_REG]) & ~(0xfff); // NEED TO FIX
    stack_end = (stack + (ALIGN_TO - 1) + (2 * ALIGN_TO)) & ~(0xfff);
    
    /* Find process's root space then align it; set p->mmu_table to root */
    root = MMU_TABLE + (1024 * (p->pid - 1) * 10);
    root = (uint32_t *) (((uint32_t) root + (ALIGN_TO - 1)) & -ALIGN_TO);
    p->mmu_table = (uint32_t) root;

    uint32_t p_vpn1 = get_pn1(program);
    uint32_t p_vpn0 = get_pn0(program);
    print_line_and_hex("program: ", program);
    print_line_and_hex("p_vpn1: ", p_vpn1);
    print_line_and_hex("p_vpn0: ", p_vpn0);
    uint32_t s_vpn1 = get_pn1(stack);
    uint32_t s_vpn0 = get_pn0(stack);
    print_line_and_hex("stack: ", stack);
    print_line_and_hex("s_vpn1: ", s_vpn1);
    print_line_and_hex("s_vpn0: ", s_vpn0);

    uint32_t *level0_address = root + 8192;
    print_line_and_hex("level0_address: ", (uint32_t) level0_address);

    /* Adjust 34-bit physical address to 32-bit PTE */
    root[p_vpn1] = create_pte(get_pn1((uint32_t) level0_address), get_pn0((uint32_t) level0_address), true, p->mode, false);
    print_line_and_hex("root[p_vpn1]: ", root[p_vpn1]);

    while(program < program_end){
        level0_address[p_vpn0] = create_pte(p_vpn1, p_vpn0, true, p->mode, true);
        print_line_and_hex("level0_address[p_vpn0]: ", level0_address[p_vpn0]);
        program += 4096;
        p_vpn0 += 1;
    }

    /* Check if vpn1 was the same for both the program and stack, if it wasn't then create a new PTE */
    if(test_bit(&root[s_vpn1], 0) == 0){
        write_stringln("have to create new entry for level 1");
        root[s_vpn1] = create_pte(get_pn1((uint32_t) level0_address), get_pn0((uint32_t) level0_address), false, p->mode, false);
    }

    uint32_t og_svpn0 = s_vpn0;
    while(stack < stack_end){
        level0_address[s_vpn0] = create_pte(s_vpn1, s_vpn0, false, p->mode, true);
        print_line_and_hex("level0_address[p_vpn0]: ", level0_address[p_vpn0]);
        stack += 4096;
        s_vpn0 += 1;
    }
}

void mmu_unmap(PROCESS *p){
}

void hello()
{
	//This is sample code. This will run the process for 10,000,000 iterations
	//and then sleep for 5 seconds over and over again.
    ecall(SYS_SET_QUANTUM, 10);
    do {
        for (volatile int i = 0;i < 10000000;i++);
        ecall(SYS_SLEEP, 5);
    } while(1);
}

void test(){
	//Put whatever you want to test here.
    new_process(hello, 0, 0, MACHINE);
    new_process(hello, 0, 0, SUPERVISOR);
    new_process(hello, 0, 0, USER);
}







/* Custom functions */
static uint32_t get_pn1(uint32_t value){
    uint32_t ret_val = (value >> 22) & 0x3ff;
    print_line_and_hex("\tget_pn1() returning: ", ret_val);
    return ret_val;
}

static uint32_t get_pn0(uint32_t value){
    uint32_t ret_val = (value >> 12) & 0x3ff;
    print_line_and_hex("\tget_pn0() returning: ", ret_val);
    return ret_val;
}

static uint32_t create_pte(uint32_t ppn1, uint32_t ppn0, bool is_program, uint32_t mode, bool is_level0){
    uint32_t ret_val = 0, perms = 0;
    if(is_program) perms = 0xb;
    else perms = 0x7;

    ret_val = (ppn1 << 20) | (ppn0 << 10) | perms;

    if(mode == 0) ret_val |= 1 << 4;
    else ret_val &= ~(1 << 4);

    print_line_and_hex("\tcreate_pte() returning: ", ret_val);
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