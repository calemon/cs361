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
static void set_bit(uint32_t *value, uint32_t bit_index);
static void clear_bit(uint32_t *value, uint32_t bit_index);
static void clear_string(char *string, uint32_t length);
static void write_uint(uint32_t value);
static uint32_t bit_mask(int bit_start, int bit_end);
static void to_string(char *dest, uint32_t value);
static void print_line_and_hex(char *pre, uint32_t value, bool newline = true);

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
    */
    /*
    if mapping program or stack second, check if for root + vpn[1] if the valid bit is 1, if it is follow it
    Put in level 0 pte the program >> 2?
    program = p->*program() & 0xfff;
    stack_ptr = p->regs[2] & 0xfff;
    program_end = (program + 4095 + 4096) & ~(0xfff)
    stack_end = (stack + 4095 + 8192) & ~(0xfff)

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
    
    /* Find process's root space then align it */
    root = MMU_TABLE + (1024 * (p->pid - 1) * 10);
    print_line_and_hex("root before align: ", (uint32_t) root);

    root = (uint32_t *) (((uint32_t) root + (ALIGN_TO - 1)) & -ALIGN_TO);
    print_line_and_hex(" root after align: ", (uint32_t) root);

    p->mmu_table = (uint32_t) root;

    print_line_and_hex("program: ", program);

    uint32_t vpn1 = get_pn1(program);
    uint32_t vpn0 = get_pn0(program);
    print_line_and_hex("p_vpn1: ", vpn1);
    print_line_and_hex("p_vpn0: ", vpn0);

    uint32_t *level0_address = root + 8192;
    print_line_and_hex("level0_address: ", (uint32_t) level0_address);

    /* Adjust 34-bit physical address to 32-bit PTE */
    root[vpn1] = ((uint32_t) level0_address >> 2) | 1;
    if(p->mode == ProcessMode::USER) root[vpn1] |= (1 << 4);
    print_line_and_hex("root[vpn1]: ", root[vpn1]);

    while(program < program_end){
        level0_address[vpn0] = 5;
        program += 4096;
        vpn0 += 1;
    }


    /*
    space = MMU_TABLE + (1024 * (p->pid - 1) * 10);
    aligned_space = (uint32_t *) (((uint32_t) space + (ALIGN_TO - 1)) & -ALIGN_TO);
    p->mmu_table = (uint32_t) aligned_space;
    write_string("aligned_space: ");
    hex_to_string(str, (uint32_t) aligned_space);
    write_stringln(str);

    vpn1 = (((uint32_t) p->program) >> 22) & 0x3ff;
    vpn0 = (((uint32_t) p->program) >> 12) & 0x3ff;
    lvl1_ptr = aligned_space + vpn1;
    write_string("lvl1_ptr address: ");
    hex_to_string(str, (uint32_t) lvl1_ptr);
    write_stringln(str);
    write_string("lvl1_ptr value: ");
    hex_to_string(str, *lvl1_ptr);
    write_stringln(str);
    write_string("aligned+4096+TABLE_SIZE & 0x000: ");
    lvl2_ptr = (uint32_t *) (((uint32_t) (aligned_space + 4096 + TABLE_SIZE)) & ~(0xfff));
    hex_to_string(str, (uint32_t) lvl2_ptr);
    write_stringln(str);
    lvl1_pte = ((uint32_t) lvl2_ptr) | 0x000000001;
    *lvl1_ptr = lvl1_pte;
    write_string("level 1 pte: ");
    hex_to_string(str, lvl1_pte);
    write_stringln(str);

    write_string("MMU_TABLE[VPN1]: ");
    hex_to_string(str, aligned_space[vpn1]);
    write_stringln(str);
    */
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
    return (value >> 22) & 0x3ff;
}

static uint32_t get_pn0(uint32_t value){
    return (value >> 12) & 0x3ff;
}

static void set_bit(uint32_t *value, uint32_t bit_index){
    /* Left shift 1 by the given bit_index then OR the given value with the shifted bits */
    *value |= 1 << bit_index;
}

static void clear_bit(uint32_t *value, uint32_t bit_index){
    /* Left shift 1 by the given bit_index, and flip the result. Then AND the given value with the shifted bits to clear */
    *value &= ~(1 << bit_index);
}

static void clear_string(char *string, uint32_t length){
    for(uint32_t i = 0; i < length; i++) string[i] = '\0';
}

static void write_uint(uint32_t value){
    char pstr[64];
    hex_to_string(pstr, value);
    write_string(pstr);
    clear_string(pstr, 64);
}

static uint32_t bit_mask(int bit_start, int bit_end){
    uint32_t mask = 0;
    /* Check if the given bit_index is out of bounds */
    if(bit_start > 31 || bit_end > 31) return mask;
    /* Swap if the given start is greater than the given end */
    if(bit_start > bit_end){
        int temp = bit_start;
        bit_start = bit_end;
        bit_end = temp;
    }

    /* Left shift 1 by the difference between bit_end and bit_start, and subtract the result by 1 .
    Then left shift the result of the subtraction by bit_start and OR with 1 left shifted by bit_end */
    mask = (((1 << (bit_end - bit_start)) - 1) << bit_start) | (1 << bit_end);

    return mask;
}

static void print_line_and_hex(char *pre, uint32_t value, bool newline){
    char hex_val[64];
    write_string(pre);
    hex_to_string(hex_val, value);
    if(newline) write_stringln(hex_val);
    else write_string(hex_val);
}