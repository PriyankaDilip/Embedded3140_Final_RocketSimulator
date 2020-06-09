
#include "3140_concur.h" 
#include "realtime.h" 
#include <stdlib.h>

process_t * current_process = NULL;
process_t * process_queue = NULL;				// The head of the ready queue
process_t * process_tail = NULL;					// The tail of the ready queue
process_t * rt_ready_queue = NULL;			// The head of the real-time ready queue
process_t * rt_ready_tail = NULL;				// The tail of the real-time ready queue
process_t * rt_unready_queue = NULL;		// The head of the real-time unready queue
process_t * rt_unready_tail = NULL;			// The tail of the real-time unready queue
realtime_t current_time;
int process_deadline_met;
int process_deadline_miss;

unsigned int * correct_sp;
unsigned int * actual_sp;
unsigned int return_sp;

/*This structure holds the process structure information */
typedef struct process_state {
	
	unsigned int * sp; 							// The stack pointer
	unsigned int * sp_orig; 				// The original stack pointer
	int n; 													// The size of the stack frame
	struct process_state * next; 		// The next process
	struct process_state * prev; 		// The previous process
	int is_realtime;								// Is the process real-time
	realtime_t * arrival_time;			// Arrival time of real-time process
	realtime_t * deadline;					// Deadline of a real-time process
	int is_periodic;								// Is the process periodic?
	realtime_t * period;						// What is the period?
	void (* func)(void);								// f

} process_t;

/* Returns 1 if the first argument is earlier than the second argument*/
int earlier_than(realtime_t * a, realtime_t * b) {
	if( (a -> sec < b -> sec ) || (a -> sec == b -> sec && a -> msec <= b -> msec) ) return 1;
	return 0;
}

/* Enqueue a process into the real-time ready queue sorted by deadline -- the soonest deadline is at the head */
void enqueue_rt_ready(process_t * proc) {
	
	// If there is nothing in the queue
	if (! rt_ready_queue) {
		rt_ready_queue = proc;
		rt_ready_tail = proc;
		proc -> next = NULL;
		return;
	}
	
	if(earlier_than(proc -> deadline, rt_ready_queue -> deadline)) {
		proc -> next = rt_ready_queue;
		rt_ready_queue -> prev = proc;
		rt_ready_queue = proc;
		return;
	}
	
	// Create a temporary process and walk it down the list
	process_t * current = rt_ready_queue;
	
	// While the new process deadline is still later than current process deadline
	while(earlier_than(current -> deadline, proc -> deadline)) { 
	
		if(current -> next) { // If there is a next process...
			current = current -> next; // Move current to the next process
		} else { // Enqueue at the very end of the list
			rt_ready_tail -> next = proc;
			proc -> prev = rt_ready_tail;
			proc -> next = NULL;
			rt_ready_tail = proc;
			return;
		}
	}

		current = current -> prev; // Step current back one node and enqueue proc after current
		
		proc -> next = current -> next;
		(current -> next) -> prev = proc;
		current -> next = proc;
		proc -> prev = current;		
		
}

/* Enqueue a process into the real-time unready queue sorted by arrival time */
void enqueue_rt_unready(process_t * proc) {
	
	// If there is nothing in the queue
	if (! rt_unready_queue) {
		rt_unready_queue = proc;
		rt_unready_tail = proc;
		proc -> next = NULL;
		return;
	}
	
	if(earlier_than(proc -> arrival_time, rt_unready_queue -> arrival_time)) {
		proc -> next = rt_unready_queue;
		rt_unready_queue -> prev = proc;
		rt_unready_queue = proc;
		return;
	}
	
	// Create a temporary process and walk it down the list
	process_t * current = rt_unready_queue;
	
	// While the new process arrival_time is still later than current process arrival_time
	while(earlier_than(current -> arrival_time, proc -> arrival_time)) { 
	
		if(current -> next) { // If there is a next process...
			current = current -> next; // Move current to the next process
		} else { // Enqueue at the very end of the list
			rt_unready_tail -> next = proc;
			proc -> prev = rt_unready_tail;
			proc -> next = NULL;
			rt_unready_tail = proc;
			return;
		}
	}

		current = current -> prev; // Step current back one node and enqueue proc after current
		
		proc -> next = current -> next;
		(current -> next) -> prev = proc;
		current -> next = proc;
		proc -> prev = current;		
		
}

/* Enqueue a process into the non-real-time ready queue */
void enqueue_non_rt(process_t * proc) {
	
	if (!process_queue) {
		process_queue = proc;
	}
	if (process_tail) {
		process_tail -> next = proc;
	}
	process_tail = proc;
	proc -> next = NULL;
}

/* Dequeue a process from a given queue */
process_t * dequeue(process_t ** head, process_t ** tail) {
	
	if (!(* head)) return NULL;
	process_t *proc = * head;
	* head = proc -> next;
	if (* tail == proc) {
		* tail = NULL;
	}
	proc -> next = NULL;
	return proc;
}

/* Deallocate memory for a given process */
static void process_free(process_t * proc) {
	process_stack_free(proc -> sp_orig, proc -> n);
	free(proc);
}

/* Create a new non-real-time process */
int process_create (void (*f)(void), int n) {
	unsigned int *sp = process_stack_init(f, n);
	if (!sp) return -1;
	
	process_t *proc = (process_t*) malloc(sizeof(process_t));
	if (!proc) {
		process_stack_free(sp, n);
		return -1;
	}
	
	proc -> sp = proc -> sp_orig = sp;
	proc -> n = n;
	proc -> is_realtime = 0;
	proc -> is_periodic = 0;
	
	enqueue_non_rt(proc);
	return 0;
}

/* Create a new real-time process */
int process_rt_create(void (*f)(void), int n, realtime_t* start, realtime_t* deadline) {
	unsigned int *sp = process_stack_init(f, n);
	if (!sp) return -1;
	
	process_t *proc = (process_t*) malloc(sizeof(process_t));
	if (!proc) {
		process_stack_free(sp, n);
		return -1;
	}
	
	proc -> sp = proc -> sp_orig = sp;
	proc -> n = n;
	proc -> is_realtime = 1;
	proc -> is_periodic = 0;
	proc -> arrival_time = start;
	deadline -> sec = deadline -> sec + start -> sec;
	deadline -> msec = deadline -> msec + start -> msec;
	
	if(deadline -> msec > 999) {
		deadline -> msec -= 1000;
		deadline -> sec++;
	}
	
	proc -> deadline = deadline;
	
	enqueue_rt_unready(proc);
	
	return 0;
}

/* Create a new periodic process */
int process_rt_periodic(void (*f)(void), int n, realtime_t *start, realtime_t *deadline, realtime_t *period) {
	
	unsigned int *sp = process_stack_init(f, n);
	if (!sp) return -1;
	
	process_t *proc = (process_t*) malloc(sizeof(process_t));
	if (!proc) {
		process_stack_free(sp, n);
		return -1;
	}
	
	proc -> sp = proc -> sp_orig = sp;
	proc -> n = n;
	proc -> is_realtime = 1;
	proc -> arrival_time = start;
	deadline -> sec = deadline -> sec + start -> sec;
	deadline -> msec = deadline -> msec + start -> msec;
	proc -> is_periodic = 1;
	proc -> period = period;
	proc -> func = f;
	
	// void (*f)(void);	
	
	if(deadline -> msec > 999) {
		deadline -> msec -= 1000;
		deadline -> sec++;
	}
	
	proc -> deadline = deadline;
	
	enqueue_rt_unready(proc);
	
	return 0;
}

// Reinitializes the stack for a periodic process
unsigned int * process_stack_reinit (void (*f)(void), int n, unsigned int *sp) { // added unsigned int *sp

	/* in reality, there are 18 more slots needed for stored context */
	//n += 18;
	n = 18;
		 
  if (sp == NULL) { return NULL; }	/* Allocation failed */
  
	sp[n-1] = 0x01000000; // xPSR
  sp[n-2] = (unsigned int) f; // PC
	sp[n-3] = (unsigned int) process_terminated; // LR
	sp[n-9] = 0xFFFFFFF9; // EXC_RETURN value, returns to thread mode
	sp[n-18] = 0x3; // Enable scheduling timer and interrupt
	
	//return_sp = &(sp[n-18]);
  
  return &(sp[n-18]);
}

// PIT1 Interrupt Handler
void PIT1_IRQHandler(void) {

	__disable_irq();
	
	// Update current time
	if(current_time.msec >= 999) {
		current_time.msec = 0;
		current_time.sec++;
	} else {
		current_time.msec++;
	}
	
  PIT -> CHANNEL[1].TFLG = 1 << 0; // Reset timer interrupt flag (TIF)
	
	__enable_irq();

}

/* Starts up the concurrent execution */
void process_start (void) {
	
	// PIT for triggering process_select
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
	PIT->MCR = 0;
	PIT -> CHANNEL[0].LDVAL = 0x00023000;
	PIT -> CHANNEL[1].LDVAL = 0x000051eA; // 1 millisecond
	
	// Enable interrupts
	NVIC_EnableIRQ(PIT0_IRQn);
	NVIC_EnableIRQ(PIT1_IRQn);
	
	// Enable timer for PIT0
	PIT -> CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK; 	// Request interrupt when TIF set
	PIT -> CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK; 	// Enable timer channel
	
	// Enable timer for PIT1
	PIT -> CHANNEL[1].TCTRL |= PIT_TCTRL_TIE_MASK; 	// Request interrupt when TIF set
	PIT -> CHANNEL[1].TCTRL |= PIT_TCTRL_TEN_MASK; 	// Enable timer channel
	
	// Set interrupt priorities
	NVIC_SetPriority(PIT1_IRQn, 0);
	NVIC_SetPriority(SVCall_IRQn, 1);
	NVIC_SetPriority(PIT0_IRQn, 2);
	
	// Initialize current time
	current_time.sec = 0;
	current_time.msec = 0;

	
	// Bail out fast if no processes were ever created
	if (!process_queue && !rt_unready_queue) return;
	
	process_begin();
}

/* Called by the runtime system to select another process.
   "cursp" = the stack pointer for the currently running process
*/
// Check whether unready processes in the queue have become ready
// Dequeue the ready processes in the unready queue and enqueue them into the ready queue
// While the unready queue head's arrival time is earlier than the current time...
// If there are ready processes in the real-time ready queue, select the process with the earliest deadline
// Otherwise select the normal process
// Make sure there is no process in the unready queue before termination!
unsigned int * process_select (unsigned int * cursp) {
	
	// Disable interrupts
	__disable_irq();

	if (cursp) { // Suspending a process which has not yet finished, save state and make it the tail
		current_process -> sp = cursp;
		
		// Re-enqueue current process into correct realtime/non-realtime queue
		if(current_process -> is_realtime) {
			enqueue_rt_ready(current_process);
		} else {
			enqueue_non_rt(current_process);
		}
		
	} else { // cursp is null
		
		// Check if a process was running, free its resources if one just finished
		if (current_process) {

			// Increment counters indicating deadlines met/missed
			if(current_process -> is_realtime && earlier_than(&current_time, current_process -> deadline)) {
				process_deadline_met++;
			} else if(current_process -> is_realtime) {
				process_deadline_miss++;
			}
			
			// Reinitialize the process if it's periodic or else free it
			if(current_process -> is_periodic) {
				
				current_process -> sp = process_stack_reinit(current_process -> func, current_process -> n, current_process -> sp_orig);
				
				correct_sp = current_process -> sp_orig;
				actual_sp = current_process -> sp;
				
				current_process -> arrival_time -> sec += current_process -> period -> sec;
				current_process -> arrival_time -> msec += current_process -> period -> msec;
				
				if(current_process -> arrival_time -> msec > 999) {
					current_process -> arrival_time -> msec -= 1000;
					current_process -> arrival_time -> sec++;
				}
				
				current_process -> deadline -> sec += current_process -> period -> sec;
				current_process -> deadline -> msec += current_process -> period -> msec;
				
				if(current_process -> deadline -> msec > 999) {
					current_process -> deadline -> msec -= 1000;
					current_process -> deadline -> sec++;
				}
				
				enqueue_rt_unready(current_process); // Put it in the unready queue and wait for the duration of its period
			
			} else { //It's not periodic so get rid of it for good!
				
				process_free(current_process);
				
			}
			
		}
	}
	
	// Move process from realtime unready queue to ready queue is their arrival time has arrived
	while(rt_unready_queue && earlier_than(rt_unready_queue -> arrival_time, &current_time)) {
		// Dequeue the process from the unready queue and enqueue it in the ready queue
		enqueue_rt_ready( dequeue(&rt_unready_queue, &rt_unready_tail) );
	}
	
	// Select the new current process from the front of the queue prioritizing realtime processes
	if(rt_ready_queue) {
		current_process = dequeue(&rt_ready_queue, &rt_ready_tail);
	} else {
		current_process = dequeue(&process_queue, &process_tail);
	}
	
	if (current_process) {
		
		// Reenable interrupts
		__enable_irq();
		
		// Launch the process which was just popped off the queue
		return current_process -> sp;
		
	} else if(rt_unready_queue) { // If there is a process remaining in the realtime unready queue...
		
		// Reenable interrupts
		__enable_irq();
		
		// Spin wait for process to be ready
		while(earlier_than(&current_time, rt_unready_queue -> arrival_time));
		
		// Disable interrupts
		__disable_irq();
		
		// Dequeue process from unready queue then enqueue process into ready queue
		enqueue_rt_ready( dequeue(&rt_unready_queue, &rt_unready_tail));
		current_process = dequeue(&rt_ready_queue, &rt_ready_tail);
		
		// Reenable interrupts
		__enable_irq();
		
		correct_sp = current_process -> sp_orig;
		actual_sp = current_process -> sp;
		
		return current_process -> sp;
		
	} else {
		
		// Reenable interrupts
		__enable_irq();
		
		// No process was selected, exit the scheduler
		return NULL;
	}
	
}
