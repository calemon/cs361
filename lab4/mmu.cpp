//mmu.cpp
//MMU Lab Template by Stephen Marz
//<YOUR NAME>
//<DATE>

#include <mmu.h>

extern uint32_t MMU_TABLE[MMU_TABLE_SIZE];

void mmu_disable()
{
}

void mmu_enable(PROCESS *p)
{
}

void mmu_map(PROCESS *p)
{
}

void mmu_unmap(PROCESS *p)
{
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

void test()
{
	//Put whatever you want to test here.
        new_process(hello, 0, 0, MACHINE);
        new_process(hello, 0, 0, SUPERVISOR);
        new_process(hello, 0, 0, USER);
}
