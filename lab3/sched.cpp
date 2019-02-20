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
static void print_process(PROCESS *proc);
static void print_process_list();

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
			new_process(proc_5_secs_run, "5 sec proc, priority 10", 10); //Never run
			break;
		case 2:
			new_process(proc_5_secs_run, "5 sec proc, priority 0", 0); // Will run with init and idle
			break;
		case 3:
			new_process(proc_5_secs_run, "10 sec proc, priority -10", -10); // Run until finish
			break;
		/*
		case 4:
			new_process(proc_infinite_loop, "Infinite loop process", 10); //Never run
			break;
		*/
		default:
			new_process(proc_5_secs_sleep, "10 second sleep process", 10); // Never run
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
	new_proc->runtime = 0;
	new_proc->num_switches = 0;
	new_proc->quantum_multiplier = 0;
}

void del_process(PROCESS *p) {
	p->state = PROCESS_STATE::DEAD;
	p->pid = LOWEST_PRIORITY + 1;
	p->runtime = 0;
	p->start_time = 0;
	p->num_switches = 0;
	p->sleep_time = 0;
	p->quantum_multiplier = 0;
	clear_string(p->name, 32);

}

void sleep_process(PROCESS *p, uint32_t sleep_time) {
	p->state = PROCESS_STATE::SLEEPING;
	p->sleep_time = get_timer_lo() / TIMER_FREQ + sleep_time;
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

	switch (scheduling_algorithm) {
		case SCHEDULE_ALGORITHM::SCHED_RR:
			return round_robin_sched(current);
		case SCHEDULE_ALGORITHM::SCHED_ML:
			return multilevel_sched(current);
		case SCHEDULE_ALGORITHM::SCHED_MLF:
			break;
		default:
			break;
	}

}

/* Custom functions */
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
		if(i == (int32_t) current->pid - 1) return current;
		/* Check if process at index i is available as the next process */
		if(temp->state == PROCESS_STATE::RUNNING) return temp;
	}
}

static PROCESS *multilevel_sched(PROCESS *current){
	PROCESS *temp, *highest_process;
	//int32_t same_priority_index = -1;

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
		if(temp->state != PROCESS_STATE::RUNNING) continue;
		if(temp->priority <= highest_process->priority) highest_process = temp;
		/* May need below else/if if the next process of same priority has to be the next consecutive process.
		Currently, with only the above if statement, if the current process is at index 4 and there's two processes
		of same priority at indices 6 and 1, then 1 will be scheduled next. If the below if/else statement is included,
		then 6 would be scheduled. */
		/*else if(temp->priority == highest_level && same_priority_index == -1){
			same_priority_index = i;
		}*/
	}

	return highest_process;
	/* NEED FOLLOWING IF THE ABOVE COMMENTED-OUT else/if IS INCLUDED */
	/*
	if(same_priority_index != -1) return &process_list[same_priority_index];
	else return highest_process;
	*/
}

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