//////////////////////////////////
//
// Do not modify anything between
// this box and the one below
//
/////////////////////////////////
#include <sched.h>


/////////////////////////////////
//
// Implement the following below
//
////////////////////////////////

extern PROCESS process_list[MAX_PROCESSES];
extern SCHEDULE_ALGORITHM scheduling_algorithm;
extern char stack_space[STACK_ALLOC * MAX_PROCESSES];

#if defined(STUDENT)
static void proc_10_secs_run()
{
	uint32_t END_AFTER = get_timer_lo() / TIMER_FREQ + 10;
	write_stringln("\r\nI do nothing but quit after 10 seconds.\r\n");
	while (get_timer_lo() / TIMER_FREQ < END_AFTER);
}

static void proc_10_secs_sleep()
{
	write_stringln("\r\nI'm going to sleep for 10 seconds, then quitting.\r\n");
	sleep_process(get_current(), 10);
	wait();
}

void add_new_process(int padd)
{
	switch (padd) {
		case 1:
			new_process(proc_10_secs_run, "10 second run process", 10);
		break;
		default:
			new_process(proc_10_secs_sleep, "10 second sleep process", 10);
		break;
	}
}
#endif
void new_process(void (*func)(), const char *name, int32_t priority)
{
}

void del_process(PROCESS *p)
{
}

void sleep_process(PROCESS *p, uint32_t sleep_time)
{
}

PROCESS *schedule(PROCESS *current)
{
	switch (scheduling_algorithm) {
	}
}
