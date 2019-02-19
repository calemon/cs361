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
static void clear_string(char *string, int str_length);
static void print_process(PROCESS *proc);
static void print_process_list();

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

static void proc_infinite_loop() {
	uint32_t END_AFTER = get_timer_lo() / TIMER_FREQ - 20;

	write_stringln("\r\nI do nothing but run for and infinite amount of time");
	while(get_timer_lo() / TIMER_FREQ > END_AFTER);
}

void add_new_process(int padd) {
	switch (padd) {
		case 1:
			new_process(proc_10_secs_run, "10 second run process", 10);
			break;
		case 2:
			new_process(proc_infinite_loop, "Infinite loop process", 10);
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
	bool found_dead_process = false;

	/* Check for any available index in the process list */
	for(uint32_t i = 0; i < MAX_PROCESSES && found_dead_process == false; i++){
		new_proc = &process_list[i];
		/* Skip over process if in RUNNING or SLEEPING state and keep looking */
		if(new_proc->state == PROCESS_STATE::RUNNING || new_proc->state == PROCESS_STATE::SLEEPING) continue;

		found_dead_process = true;
		available_index = i;
	}
	if(found_dead_process != true) return;
	
	/* Clamp priority number if above lowest priority (10) or below highest priority (-10) */
	if(priority > LOWEST_PRIORITY) priority = LOWEST_PRIORITY;
	else if(priority < HIGHEST_PRIORITY) priority = HIGHEST_PRIORITY;

	/* Fill in new process information */
	strcpy(new_proc->name, name);
	new_proc->state = PROCESS_STATE::RUNNING;
	new_proc->pid = available_index + 1;
	new_proc->priority = priority;
	new_proc->program = func;
	new_proc->start_time = get_timer_lo() / TIMER_FREQ;
	new_proc->regs[REGISTERS::SP] = (uint32_t) &stack_space[STACK_ALLOC * new_proc->pid - 1];
	new_proc->regs[REGISTERS::RA] = (uint32_t) recover;
}

void del_process(PROCESS *p) {
	p->state = PROCESS_STATE::DEAD;
	p->pid = 0;
	clear_string(p->name, 32);
}

void sleep_process(PROCESS *p, uint32_t sleep_time) {
	p->state = PROCESS_STATE::SLEEPING;
	p->sleep_time = get_timer_lo() / TIMER_FREQ + sleep_time;
}

PROCESS *schedule(PROCESS *current) {
	PROCESS *temp;
	uint32_t current_time;

	/* Check for any sleeping processes, wake if the process is past expired time */
	for(uint32_t i = 0; i < MAX_PROCESSES; i++){
		temp = &process_list[i];
		current_time = get_timer_lo() / TIMER_FREQ;
		/* If the process if in a sleeping state and the current time is past it's sleep time, then wake it. */
		if(temp->state == PROCESS_STATE::SLEEPING && current_time >= temp->sleep_time){
			temp->state = PROCESS_STATE::RUNNING;
		}
	}

	switch (scheduling_algorithm) {
		case SCHEDULE_ALGORITHM::SCHED_RR:
			/* Look for next process starting at process at index current->pid */
			for(int32_t i = current->pid; i <= (int32_t) MAX_PROCESSES; i++){
				temp = &process_list[i];

				/* Reset i so that processes before current->pid will be checked */
				if(i == MAX_PROCESSES){
					i = -1;
					continue;
				}
				/* All processes were checked, so return init */
				if(i == (int32_t) current->pid - 1){
					temp = &process_list[0];
					return temp;
				}

				/* Check if process at index i is available as the next process */
				if(temp->state == PROCESS_STATE::RUNNING){
					return temp;
				}
			}
		case SCHEDULE_ALGORITHM::SCHED_ML:
			break;
		case SCHEDULE_ALGORITHM::SCHED_MLF:
			break;
		default:
			break;
	}

}

/* Custom functions */
static void clear_string(char *string, int str_length){
	for(int32_t i = 0; i < str_length; i++) string[i] = '\0';
}

static void print_process(PROCESS *proc){
	char str[32];
	
	write_string("Process \'");
	write_string(proc->name);
	write_string("\' (PID: ");
	to_string(str, proc->pid);
	write_string(str);
	clear_string(str, 32);
	write_stringln(")");

	write_string("  Process State: ");
	switch(proc->state){
		case PROCESS_STATE::DEAD:
			write_stringln("DEAD");
			break;
		case PROCESS_STATE::SLEEPING:
			write_stringln("SLEEPING");
			break;
		case PROCESS_STATE::RUNNING:
			write_stringln("RUNNING");
			break;
		default:
			write_stringln("Not specified");
			break;
	}

	to_string(str, proc->priority);
	write_string("  Priority: ");
	write_stringln(str);
	clear_string(str, 32);

	to_string(str, proc->num_switches);
	write_string("  Number of switches: ");
	write_stringln(str);
	clear_string(str, 32);

	to_string(str, proc->start_time);
	write_string("  Start Time: ");
	write_stringln(str);
	clear_string(str, 32);

	to_string(str, proc->runtime);
	write_string("  Runtime: ");
	write_stringln(str);
	clear_string(str, 32);
}

static void print_process_list(){
	write_string("\n");
	write_stringln("PROCESS LIST");
	write_stringln("=-=-=-=-==-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
	for(uint32_t i = 0; i < MAX_PROCESSES; i++) print_process(&process_list[i]);
	write_string("\n");
}