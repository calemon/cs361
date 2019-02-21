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

/* Custom function declarations */
static PROCESS *round_robin_sched(PROCESS *current);
static PROCESS *multilevel_sched(PROCESS *current);
static void clear_string(char *string, int str_length);

#if defined(STUDENT)
static void proc_5_secs_run() {
	uint32_t END_AFTER = get_timer_lo() / TIMER_FREQ + 5;
	write_stringln("\r\nI do nothing but quit after 5 seconds.\r\n");
	while (get_timer_lo() / TIMER_FREQ < END_AFTER);
}

static void proc_5_secs_sleep() {
	write_stringln("\r\nI'm going to sleep for 5 seconds, then quitting.\r\n");
	sleep_process(get_current(), 5);
	wait();
}

static void proc_infinite_loop() {
	uint32_t END_AFTER = get_timer_lo() / TIMER_FREQ - 20;

	write_stringln("\r\nI do nothing but run for and infinite amount of time");
	while(get_timer_lo() / TIMER_FREQ > END_AFTER);
}

void add_new_process(int padd) {
	switch (padd) {
		case 1:
			new_process(proc_5_secs_run, "5 sec proc, priority 10", 10);
			break;
		case 2:
			new_process(proc_5_secs_run, "5 sec proc, priority 0", 0);
			break;
		case 3:
			new_process(proc_5_secs_run, "5 sec proc, priority -10", -10);
			break;
		case 4:
			new_process(proc_infinite_loop, "Infinite loop process", -10);
			break;
		default:
			new_process(proc_5_secs_sleep, "5 second sleep process", 10);
			break;
	}
}
#endif

void new_process(void (*func)(), const char *name, int32_t priority) {
	PROCESS *new_proc;
	int32_t available_index = -1;
	bool found_dead_process = false;

	/* Check for any available index in the process list */
	for(uint32_t i = 0; i < MAX_PROCESSES && found_dead_process == false; i++){
		new_proc = &process_list[i];
		/* Skip over process if in RUNNING or SLEEPING state and keep looking */
		if(new_proc->state == PROCESS_STATE::RUNNING || new_proc->state == PROCESS_STATE::SLEEPING) continue;

		found_dead_process = true;
		available_index = i;
	}
	/* No available process found so return */
	if(found_dead_process != true) return;
	
	/* Clamp priority number if above lowest priority (10) or below highest priority (-10) */
	if(priority > LOWEST_PRIORITY) priority = LOWEST_PRIORITY;
	else if(priority < HIGHEST_PRIORITY) priority = HIGHEST_PRIORITY;

	/* Fill in new process information */
	new_proc = &process_list[available_index];
	strcpy(new_proc->name, name);
	new_proc->state = PROCESS_STATE::RUNNING;
	new_proc->pid = available_index + 1;
	new_proc->priority = priority;
	new_proc->program = func;
	new_proc->start_time = get_timer_lo() / TIMER_FREQ;
	new_proc->regs[REGISTERS::SP] = (uint32_t) &stack_space[STACK_ALLOC * new_proc->pid - 1];
	new_proc->regs[REGISTERS::RA] = (uint32_t) recover;
	new_proc->runtime = 0;
	new_proc->num_switches = 0;
	new_proc->quantum_multiplier = 0;
}

void del_process(PROCESS *p) {
	/* Set the process to dead and reset all of it's data */
	p->state = PROCESS_STATE::DEAD;
	p->runtime = 0;
	p->start_time = 0;
	p->num_switches = 0;
	p->sleep_time = 0;
	p->quantum_multiplier = 0;
	clear_string(p->name, 32);

	/* If the scheduling algorithm is currently MLF, then reset the priority to HIGHEST_PRIORITY */
	if(scheduling_algorithm == SCHEDULE_ALGORITHM::SCHED_MLF) p->priority = HIGHEST_PRIORITY;
}

void sleep_process(PROCESS *p, uint32_t sleep_time) {
	/* Set to sleeping and set the sleep time to current + sleep_time (in seconds) */
	p->state = PROCESS_STATE::SLEEPING;
	p->sleep_time = get_timer_lo() / TIMER_FREQ + sleep_time;

	/* If in MLF mode, every time a process sleeps then it needs to be reset to the highest priority */
	if(scheduling_algorithm == SCHEDULE_ALGORITHM::SCHED_MLF) p->priority = HIGHEST_PRIORITY;
}

PROCESS *schedule(PROCESS *current) {
	uint32_t current_time;
	PROCESS *temp;

	/* Check for any sleeping processes, wake if the process is past expired time */
	for(uint32_t i = 0; i < MAX_PROCESSES; i++){
		temp = &process_list[i];
		current_time = get_timer_lo() / TIMER_FREQ;
		/* If the process if in a sleeping state and the current time is past it's sleep time, then wake it. */
		if(temp->state == PROCESS_STATE::SLEEPING && current_time >= temp->sleep_time){
			temp->state = PROCESS_STATE::RUNNING;
		}
	}

	/* Choose the scheduling algorithm, default is RR; ML and MLF share the same function */
	switch (scheduling_algorithm) {
		case SCHEDULE_ALGORITHM::SCHED_RR:
			return round_robin_sched(current);
		case SCHEDULE_ALGORITHM::SCHED_ML:
			return multilevel_sched(current);
		case SCHEDULE_ALGORITHM::SCHED_MLF:
			return multilevel_sched(current);
		default:
			return round_robin_sched(current);
	}

}

/* Round robin algorithm: All processes are equal, priority and multiplier do not matter.
* Round robin will begin looking for the next available process by going through process_list
* starting at the current processes's index in the process_list and just going through the list
* iteratively.
*/
static PROCESS *round_robin_sched(PROCESS *current){
	PROCESS *temp;
	/* Look for next process starting at process at index current->pid */
	for(int32_t i = current->pid; i <= (int32_t) MAX_PROCESSES; i++){
		temp = &process_list[i];

		/* Reset i so that processes before current->pid will be checked */
		if(i == MAX_PROCESSES){
			i = -1;
			continue;
		}

		/* All processes were checked, so return current */
		if(i == (int32_t) current->pid - 1){
			if(current->state != PROCESS_STATE::RUNNING) return &process_list[0];
			else return current;
		}
		/* Check if process at index i is available as the next process */
		if(temp->state == PROCESS_STATE::RUNNING) return temp;
	}
}

/* ML and MLF: ML will cause starvation as only the higher priority processes will run. So,
* any process above init's priority will cause program to freeze, same goes for any other process
* MLF will solve starvation because after all processes run, it's priority will decrement by 1 and
* multiplier will increase.
*/
static PROCESS *multilevel_sched(PROCESS *current){
	PROCESS *temp, *highest_process;

	/* If current isn't dead, then use it as the highest process, else use the init process */
	if(current->state == PROCESS_STATE::RUNNING) highest_process = current;
	else highest_process = &process_list[0];

	for(int32_t i = current->pid; i <= (int32_t) MAX_PROCESSES; i++){
		/* Reached end of process_list, reset i to beginning */
		if(i == MAX_PROCESSES){
			i = -1;
			continue;
		}
		/* Checked all of process_list, back at current_process so return it */
		if(i == (int32_t) current->pid - 1) break;

		temp = &process_list[i];
		/* Skip if temp isn't currently running */
		if(temp->state != PROCESS_STATE::RUNNING) continue;
		/* Set highest_process to temp because it is the newest highest_process */
		if(temp->priority <= highest_process->priority) highest_process = temp;
	}

	/* If using MLF, need to adjust priority and multiplier of the current process before returning the next */
	if(scheduling_algorithm == SCHEDULE_ALGORITHM::SCHED_MLF){
		/* Decrement current's priority */
		if(current->priority < LOWEST_PRIORITY) current->priority += 1;
		else current->priority = LOWEST_PRIORITY;

		/* Set the new multiplier for current so it will run longer the next time it is ran */
		current->quantum_multiplier = current->priority + 10;
	}

	return highest_process;
}

/* Used for clearing out any strings; Null characters are put in all indices */
static void clear_string(char *string, int str_length){
	for(int32_t i = 0; i < str_length; i++) string[i] = '\0';
}