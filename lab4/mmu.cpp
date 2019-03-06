/* Lab 4
 * mmu.cpp
 * MMU Lab Template by Stephen Marz
 * Casey Lemon
 * 8 March 2019
 */
#include <mmu.h>

/* Custom functions */
static void set_bit(uint32_t *value, uint32_t bit_index);
static void clear_bit(uint32_t *value, uint32_t bit_index);
static void clear_string(char *string, uint32_t length);
static void write_uint(uint32_t value);
static uint32_t bit_mask(int bit_start, int bit_end);
static void to_string(char *dest, uint32_t value);

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
    uint32_t *space, *aligned_space, *stack_reg, *lvl1_ptr, *lvl2_ptr;
    uint32_t root, mask, vpn1, vpn0, lvl1_pte = 0;
    
    /* Find process's root space then align it */
    root = (uint32_t) MMU_TABLE + (1024 * 10 * (p->pid-1))
    space = MMU_TABLE + (1024 * (p->pid - 1) * 10);
    aligned_space = (uint32_t *) (((uint32_t) space + (ALIGN_TO - 1)) & -ALIGN_TO);
    p->mmu_table = (uint32_t) aligned_space;
    write_string("aligned_space: ");
    hex_to_string(str, (uint32_t) aligned_space);
    write_stringln(str);

    /* Decompose program pointer */
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