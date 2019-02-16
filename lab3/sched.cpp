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

const int32_t HIGHEST_PRIORITY = -10;
const int32_t LOWEST_PRIORITY = 10;

#if defined(STUDENT)
static void proc_10_secs_run() {
	uint32_t END_AFTER = get_timer_lo() / TIMER_FREQ + 10;
	write_stringln("\r\nI do nothing but quit after 10 seconds.\r\n");
	while (get_timer_lo() / TIMER_FREQ < END_AFTER);
}

static void proc_10_secs_sleep() {
	write_stringln("\r\nI'm going to sleep for 10 seconds, then quitting.\r\n");
	sleep_process(get_current(), 10);
	wait();
}

void add_new_process(int padd) {
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

void new_process(void (*func)(), const char *name, int32_t priority) {
	PROCESS *new_proc;
	int32_t available_index = -1;

	/* Check for any available index in the process list */
	for(uint32_t i = 0; i < MAX_PROCESSES; i++){
		if(process_list[i].state == PROCESS_STATE::DEAD) {
			available_index = i;
			break;
		}
	}
	if(available_index == -1) return;

	/* Clamp priority number if above lowest priority or below highest priority */
	if(priority > LOWEST_PRIORITY) priority = 10;
	else if(priority < HIGHEST_PRIORITY) priority = -10;

	new_proc = &process_list[available_index];
	new_proc->program = func;
	strcpy(new_proc->name, name);
	new_proc->priority = priority;
	new_proc->pid = available_index + 1;
	new_proc->regs[REGISTERS::SP] = stack_space[(STACK_ALLOC * available_index) - 1];
	//new_proc->regs[REGISTERS::RA] = recover;
	new_proc->quantum_multiplier = 0;

	return;
}

void del_process(PROCESS *p) {
	p->state = PROCESS_STATE::DEAD;
	p->priority = 10;
}

void sleep_process(PROCESS *p, uint32_t sleep_time) {
	p->sleep_time = get_timer_lo() / TIMER_FREQ + sleep_time;
}

PROCESS *schedule(PROCESS *current) {
	PROCESS *next_process;
	bool found_process = false;
	uint32_t found_index = (uint32_t) current->pid;
	//unsigned long long timer = (unsigned long long) get_timer_hi() << 32 | get_timer_lo();

	/* Check for any sleeping processes, wake if the process is past expired time */
	for(uint32_t i = 0; i < MAX_PROCESSES; i++){
		/* If the process if in a sleeping state and it's past it's sleep time, then wake it. */
		if(process_list[i].state == PROCESS_STATE::SLEEPING && get_timer_lo() / TIMER_FREQ >= process_list[i].sleep_time){
			process_list[i].state = PROCESS_STATE::RUNNING;
			if(found_process != true){ 
				found_process = true;
				found_index = i;
			}
		}
	}

	switch (scheduling_algorithm) {
		case SCHEDULE_ALGORITHM::SCHED_RR:
			/* Look for next available process, starting with process after current->pid - 1 */
			while(found_process == false){
				if(found_index >= MAX_PROCESSES) {
					found_index = 0;
					continue;
				}
				if(process_list[found_index].state == PROCESS_STATE::RUNNING) found_process = true;
				found_index++;
			}

			next_process = &process_list[found_index];
			break;
		case SCHEDULE_ALGORITHM::SCHED_ML:
			break;
		case SCHEDULE_ALGORITHM::SCHED_MLF:
			break;
		default:
			break;
	}

	return next_process;
}
