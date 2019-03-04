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
    satp_reg |= (p->mmu_table & bit_mask(0,21));
    set_bit(&satp_reg, 31);
    set_satp(satp_reg);
}

void mmu_map(PROCESS *p){
    uint32_t *space, *stack_reg, *program;
    uint32_t aligned_space, mask, vpn1, vpn0;
    
    space = &MMU_TABLE[1024 * p->pid * 10];
    /* Aligned points to root page table entry */
    aligned_space = (uint32_t) (space + (ALIGN_TO - 1)) & -ALIGN_TO;
    write_string("space = ");
    hex_to_string(str, (uint32_t) space);
    write_stringln(str);
    write_string("aligned_space = ");
    hex_to_string(str, (uint32_t) aligned_space);
    write_stringln(str);

    vpn1 = aligned_space >> 22 & bit_mask(0,10);
    write_string("VPN[1] = ");
    hex_to_string(str, vpn1);
    write_stringln(str);

    vpn0 = (aligned_space >> 12) & bit_mask(0, 10);
    write_string("VPN[0] = ");
    hex_to_string(str, vpn0);
    write_stringln(str);

    p->mmu_table = aligned_space;


    /*
    write_string("p->mmu_table = ");
    hex_to_string(str, p->mmu_table);
    write_stringln(str);

    stack_reg = &p->regs[SP_REG];
    write_string("stack_reg address = ");
    hex_to_string(str, (uint32_t) stack_reg);
    write_stringln(str);
    write_string("program address = ");
    hex_to_string(str, (uint32_t) p->program);
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

static void to_string(char *dest, uint32_t value){
    int i = 0, length;
    char temp;
    bool is_negative = false;

    /* If the value is negative, set the flag and make value positive */
    if(value < 0){
        is_negative = true;
        value *= -1;
    } else if(value == 0){
        dest[i++] = '0';
    }
    
    /* Get each digit by taking the mod 10 of the value as a digit then divide value by 10 */
    while(value > 0){
        dest[i++] = (value % 10) + '0';
        value /= 10;
    }

    /* The above algorithm produces the number in reverse order, so reverse again to correct the order */
    length = i;
    for(int j = 0; j < i; j++, i--){
        temp = dest[j];
        dest[j] = dest[i-1];
        dest[i-1] = temp;
    }

    /* If the value was originally negative, then shift each character
     to the right one and set the first character to '-' */
    if(is_negative){
        for(int j = length; j >= 0; j--){
            dest[j+1] = dest[j];
        }
        dest[0] = '-';
        length++;
    }

    dest[length] = '\0';
    return;
}