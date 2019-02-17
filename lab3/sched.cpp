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

static void proc_20_secs_run() {
	uint32_t END_AFTER = get_timer_lo() / TIMER_FREQ + 20;

	write_stringln("\r\nI do nothing but quit after 20 seconds");
	while(get_timer_lo() / TIMER_FREQ < END_AFTER);
}

void add_new_process(int padd) {
	switch (padd) {
		case 1:
			new_process(proc_10_secs_run, "10 second run process", 10);
			break;
		case 2:
			new_process(proc_20_secs_run, "20 second run process", 10);
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
	if(priority > LOWEST_PRIORITY) priority = 10;
	else if(priority < HIGHEST_PRIORITY) priority = -10;

	/* Fill in new process information */
	strcpy(new_proc->name, name);
	new_proc->state = PROCESS_STATE::RUNNING;
	new_proc->pid = available_index + 1;
	new_proc->priority = priority;
	new_proc->program = func;
	new_proc->runtime = 0;
	new_proc->start_time = get_timer_lo() / TIMER_FREQ;
	new_proc->regs[REGISTERS::SP] = (uint32_t) &stack_space[(STACK_ALLOC * available_index) - 1];
	new_proc->regs[REGISTERS::RA] = (uint32_t) recover;
	new_proc->quantum_multiplier = 0;

	//print_process_list();
}

void del_process(PROCESS *p) {
	p->state = PROCESS_STATE::DEAD;
	p->priority = 10;
}

void sleep_process(PROCESS *p, uint32_t sleep_time) {
	p->sleep_time = get_timer_lo() / TIMER_FREQ + sleep_time;
	p->state = PROCESS_STATE::SLEEPING;
}

PROCESS *schedule(PROCESS *current) {
	PROCESS *next_process, *temp;
	bool found_process = false;
	int32_t found_index = current->pid;
	char print_str[32];
	//unsigned long long timer = (unsigned long long) get_timer_hi() << 32 | get_timer_lo();

	/* Check for any sleeping processes, wake if the process is past expired time */
	for(uint32_t i = 0; i < MAX_PROCESSES; i++){
		temp = &process_list[i];
		/* If the process if in a sleeping state and it's past it's sleep time, then wake it. */
		if(temp->state == PROCESS_STATE::SLEEPING && get_timer_lo() / TIMER_FREQ >= temp->sleep_time){
			write_string("Changing PID ");
			to_string(print_str, temp->pid);

			temp->state = PROCESS_STATE::RUNNING;
		}
	}

	switch (scheduling_algorithm) {
		case SCHEDULE_ALGORITHM::SCHED_RR:
			/* Look for next available process, starting with process after current->pid - 1 */
			while(found_process == false){
				if(found_index >= (int32_t) MAX_PROCESSES) {
					found_index = 0;
					continue;
				}

				temp = &process_list[found_index];
				if(temp->state != PROCESS_STATE::RUNNING){
					found_index++;
					continue;
				}

				found_process = true;
			}

			next_process = temp;

			/* Account for runtime, num_switches, etc. of current process */
			if(current != next_process){
				current->runtime += get_timer_lo();
				current->num_switches += 1;
			}
			write_string("Found new process: ");
			write_stringln(next_process->name);
			print_process(next_process);
			return next_process;
		case SCHEDULE_ALGORITHM::SCHED_ML:
			break;
		case SCHEDULE_ALGORITHM::SCHED_MLF:
			break;
		default:
			break;
	}

}

static void clear_string(char *string, int str_length){
	for(int32_t i = 0; i < str_length; i++) string[i] = '\0';
}

static void print_process(PROCESS *proc){
	char str[32];
	
	write_string("Process \'");
	write_string(proc->name);
	write_stringln("\' is running:");

	to_string(str, proc->pid);
	write_string("  PID: ");
	write_stringln(str);
	clear_string(str, 32);

	write_string("  Process State: ");
	switch(proc->state){
		case 0:
			write_stringln("DEAD");
			break;
		case 1:
			write_stringln("SLEEPING");
			break;
		case 2:
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