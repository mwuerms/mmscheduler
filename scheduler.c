/**
 * Martin Egli
 * 2015-09-28
 * scheduler
 * coop scheduler for mcu
 */

// - includes ------------------------------------------------------------------
#define TEST_RUN

//#define DEBUG_PRINTF_ON
#include "debug_printf.h"

#include "scheduler.h"
#include <string.h>

// - private variables ---------------------------------------------------------
static process_t *idle_process;
static process_t *process_list[cNB_OF_PROCESSES];	// =NULL: unused, free
static uint8_t process_count;
static uint8_t pid_count; /// pid == 0 should not exist

// - private (static) functions-------------------------------------------------

/**
 * find the process in the list by given pid
 * find only first occurence
 * @param   pid of process to find
 * @reutn   pointert to process_t   =NULL: could not find process with given pid
 *                                  else: valid pointer
 */
static process_t *scheduler_find_process_by_pid(uint8_t pid) {
    uint8_t n;
	DEBUG_PRINTF_MESSAGE("scheduler_find_process_by_pid(%d)\n", pid);
    for(n = 0; n < cNB_OF_PROCESSES; n++) {
        if(process_list[n] != NULL) {
            if(process_list[n]->pid == pid) {
                // found process with same pid
				DEBUG_PRINTF_MESSAGE(" + found: %p\n", process_list[n]);
                return process_list[n];
            }
        }
    }
    // no process in list found
	DEBUG_PRINTF_MESSAGE(" + no process found\n");
    return NULL;
}

/**
 * remove a process from process_list given by pid
 * @param   pid of process to remove
 * @return  status =1: successfully removed process from process_list
 *                 =0: could not remove process from process_list
 * /
static int8_t process_RemoveFromProcessList(uint8_t pid);*/

/**
 * execute a process given by its PID
 * @param	pid		process identifier
 * @param	event	event for the process to execute
 * @param	data	additional data to process (if unused = NULL)
 * @return	status 	=true: OK, could execute process
 *					=false: error, could not execute process
 */
static int8_t scheduler_exec_process(uint8_t pid, uint8_t event, void *data) {
    process_t *p;
	DEBUG_PRINTF_MESSAGE("scheduler_exec_process(pid: %d, event: %d)\n", pid, event);
    // check if process exists
    if((p = scheduler_find_process_by_pid(pid)) == NULL) {
        // error, process does not exist
        return false;
    }

   	// is function pointer correctly set?
	if(p->process == NULL) {
		// error, function pointer is not set
		return false;
	}
    // check if process is not yet started
    if(p->state == cPROCESS_STATE_NONE) {
        // process is not active
        return false;
    }

    DEBUG_PRINTF_MESSAGE("execute process \"%s\" (pid: %d, event: %d, data: %p)\n",
        p->name, p->pid, event, data);

	// OK, execute process
	p->state = cPROCESS_STATE_RUNNING;
	if(p->process(event, data) == 0) {
	    // do not run this process anymore
		p->state = cPROCESS_STATE_NONE;
	}
	else {
    	// process remains active
		p->state = cPROCESS_STATE_ACTIVE;
	}
	return true;
}

// - public functions ----------------------------------------------------------

void scheduler_init(void) {
	// vars
	process_count = 0;
	pid_count = 0;  // 1st time: ++
	idle_process = NULL;
	memset((uint8_t *)process_list, 0, sizeof(process_list));
	events_init();
}

int8_t scheduler_add_process(process_t *p) {
    uint8_t n;

	// sanity tests
	if(p == NULL) {
		// error, no process
		return false;
	}
	if(p->process == NULL) {
		// error, no process_function defined
		return false;
	}

	// place process in process_list
	if(process_count >= cNB_OF_PROCESSES) {
		// error, no more space for an additional process in the process_list
		return false;
	}
    for(n = 0; n < cNB_OF_PROCESSES; n++) {
        if(process_list[n] == NULL) {
            // found empty space in process_list
            break;
        }
    }
    if(n >= cNB_OF_PROCESSES) {
        // error, could not add process to list, no more space available
	    return false;
    }

    // found empty space, add process to process_list
    process_list[n] = p;
    process_count++;
    if(pid_count == 0) {
        pid_count = 1;
    }
    else {
        pid_count++;
    }
    // success, added process to process_list
    p->pid = pid_count;
    p->state = cPROCESS_STATE_NONE;

    DEBUG_PRINTF_MESSAGE("process_Add: %s, pid: %d\n",
    		p->name,
			p->pid);
    return true;
}

int8_t scheduler_remove_process(process_t *p) {
    // not implemented yet
    return false;
}

int8_t scheduler_start_process(uint8_t pid) {
    process_t *p;
    // check if process exists
    if((p = scheduler_find_process_by_pid(pid)) == NULL) {
        // error, process does not exist
        return false;
    }
    // check if process is already started
    if(p->state != cPROCESS_STATE_NONE) {
        // error, process is already started
        return false;
    }

	// start process
	p->state = cPROCESS_STATE_ACTIVE;
	DEBUG_PRINTF_MESSAGE("process_Start: %s, pid: %d, state: %d\n",
			p->name,
			p->pid,
			p->state);
	return scheduler_send_event(pid, EV_START, NULL);
}

int8_t scheduler_stop_process(uint8_t pid) {
	// not implemented yet
    return false;
}

int8_t scheduler_add_idle_process(process_t *p) {
	// sanity tests
	if(p == NULL) {
		// error, no process
		return false;
	}
	if(p->process == NULL) {
		// error, no process_function defined
		return false;
	}

	// success, add process to idle_process
    idle_process = p;
	p->pid = cPROCESS_PID_IDLE;
	p->state = cPROCESS_STATE_NONE; // does not matter for idle_process
    DEBUG_PRINTF_MESSAGE("process_AddStartIdle: %s, pid: %d\n",
    			p->name,
				p->pid);

    return true;
}

int8_t scheduler_send_event(uint8_t pid, uint8_t event, void *data) {
	event_t ev;
	int8_t ret;

	ev.pid = pid;
	ev.event = event;
	ev.data = data;
	ret = events_add_to_main_fifo(&ev);
	return ret;
}

int8_t scheduler_is_evvent_main_fifo_empty(void) {
    return events_is_main_fifo_empty();
}

int8_t scheduler_start_event_timer(void) {
	return events_start_timer(0);

}

int8_t scheduler_stop_event_timer(void) {
	return events_stop_timer();
}

int8_t scheduler_add_timer_event(uint16_t timeout, uint8_t pid, uint8_t event, void *data) {
	event_t ev;
	int8_t ret;
	uint8_t sr;

	ev.pid = pid;
	ev.event = event;
	ev.data = data;
	lock_interrupt(sr);
	ret = events_add_single_timer_event(timeout, &ev);
	restore_interrupt(sr);
	return ret;
}

int8_t scheduler_run(void) {
	static event_t ev;
	static int8_t ret;

	while(1) {
		// get next event
		if((ret = events_get_from_main_fifo(&ev)) == true) {
			// got a valid event, send it to the process
			scheduler_exec_process(ev.pid, ev.event, ev.data);
		}
		else {
			// ev_main_fifo_data is empty, execute the idle task
			if(idle_process != NULL) {
				ret = idle_process->process(0, NULL);
#ifdef TEST_RUN
				if(ret == 0) {
					return (-1);
				}
#endif
			}
		}
	}
	return false;
}
