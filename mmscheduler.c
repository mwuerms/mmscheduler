/**
 * Martin Egli
 * 2015-09-28
 * mmscheduler
 * coop scheduler for mcu
 */

// - includes ------------------------------------------------------------------
#include "mmscheduler.h"
#include "fifo.h"

// - typedefs ------------------------------------------------------------------

// - private variables ---------------------------------------------------------
// event queue
static fifo_t ev_queue;
static event_t ev_queue_data[cNB_OF_EVENTS_IN_QUEUE];

static process_t *idle_process;
static process_t *process_list[cNB_OF_PROCESSES];	// =NULL: unused, free
static uint8_t process_count;
static uint8_t pid_count; /// pid == 0 should not exist

/* - private (static) functions --------------------------------------------- */

static void _memset(uint8_t *dest, uint8_t data, uint16_t len) {
	while(len) {
		*dest++ = data;
		len--;
	}
}

static void _memcpy(uint8_t *dest, uint8_t *src, uint16_t len) {
	while(len) {
		*dest++ = *src++;
		len--;
	}
}

/**
 * initialize the event queue
 */
static inline void evQueue_init(void) {
	fifo_init(&ev_queue, (void *)ev_queue_data, cNB_OF_EVENTS_IN_QUEUE);
	_memset((uint8_t *)ev_queue_data, 0, sizeof(ev_queue_data));
}

/**
 * write an event to the event queue
 * @param   event   pointer to event to put into ev_queue_data
 * @return  =true: OK, writing event successfull
 *          =false: Error, could not write, queue is full
 */
static inline uint8_t evQueue_write(event_t *ev) {
	DEBUG_MESSAGE("evQueue_write: (wr: %d, rd:%d, len: %d, size: %d)",
				ev_queue.write,
				ev_queue.read,
				ev_queue.length,
				ev_queue.size);
	// sanity checks
	if(ev == NULL) {
		DEBUG_MESSAGE(" ev == NULL\n");
		return false;
	}
	if(fifo_try_append(&ev_queue) == false) {
		// cannot append
		DEBUG_MESSAGE(" event queue is full\n");
		return false;
	}
	_memcpy((uint8_t *)&ev_queue_data[ev_queue.wr_proc], (uint8_t *)ev, sizeof(*ev));
	fifo_finalize_append(&ev_queue);
	DEBUG_MESSAGE(" event: pid: %d, event: %d, data: %p (wr: %d, rd:%d, len: %d, size: %d)\n",
				ev->pid, ev->event, ev->data,
				ev_queue.write,
				ev_queue.read,
				ev_queue.length,
				ev_queue.size);
	return true;
}

/**
 * read an event from the event queue
 * @param   event   pointer to event to read from ev_queue_data
 * @return  =true: OK, reading event successfull, event is valid
 *          =false: Error, could not read, ev_queue_data is empty
 */
static inline uint8_t evQueue_read(event_t *ev) {
	DEBUG_MESSAGE("evQueue_read:  (wr: %d, rd:%d, len: %d, size: %d)",
			ev_queue.write,
			ev_queue.read,
			ev_queue.length,
			ev_queue.size);
    // sanity checks
	if(ev == NULL) {
		DEBUG_MESSAGE(" ev == NULL\n");
		return false;
	}
	if(fifo_try_get(&ev_queue) == false) {
		// cannot append
		DEBUG_MESSAGE(" event queue is empty\n");
		return false;
	}
	_memcpy((uint8_t *)ev, (uint8_t *)&ev_queue_data[ev_queue.rd_proc], sizeof(*ev));
	fifo_finalize_get(&ev_queue);
	DEBUG_MESSAGE(" event: pid: %d, event: %d, data: %p (wr: %d, rd:%d, len: %d, size: %d)\n",
		ev->pid, ev->event, ev->data,
		ev_queue.write,
		ev_queue.read,
		ev_queue.length,
		ev_queue.size);
	return true;
}

/**
 * find the process in the list by given pid
 * find only first occurence
 * @param   pid of process to find
 * @reutn   pointert to process_t   =NULL: could not find process with given pid
 *                                  else: valid pointer
 */
static process_t *mmscheduler_find_process_by_pid(uint8_t pid) {
    uint8_t n;
    for(n = 0; n < cNB_OF_PROCESSES; n++) {
        if(process_list[n] != NULL) {
            if(process_list[n]->pid == pid) {
                // found process with same pid
                return process_list[n];
            }
        }
    }
    // no process in list found
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
static int8_t mmscheduler_exec_process(uint8_t pid, uint8_t event, void *data) {
    process_t *p;
    // check if process exists
    if((p = mmscheduler_find_process_by_pid(pid)) == NULL) {
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

    DEBUG_MESSAGE("execute process \"%s\" (pid: %d, event: %d, data: %p)\n",
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

/* - public functions ------------------------------------------------------- */

void mmscheduler_init(void) {
	// vars
	process_count = 0;
	pid_count = 0;  // 1st time: ++
	idle_process = NULL;
	_memset((uint8_t *)process_list, 0, sizeof(process_list));

	evQueue_init();
}

int8_t mmscheduler_add_process(process_t *p) {
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

    DEBUG_MESSAGE("process_Add: %s, pid: %d\n",
    		p->name,
			p->pid);
    return true;
}

int8_t mmscheduler_remove_process(process_t *p) {
    // not implemented yet
    return false;
}

int8_t mmscheduler_start_process(uint8_t pid) {
    process_t *p;
    // check if process exists
    if((p = mmscheduler_find_process_by_pid(pid)) == NULL) {
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
	DEBUG_MESSAGE("process_Start: %s, pid: %d, state: %d\n",
			p->name,
			p->pid,
			p->state);
	return mmscheduler_send_event(pid, cEV_START, NULL);
}

int8_t mmscheduler_stop_process(uint8_t pid) {
	// not implemented yet
    return false;
}

int8_t mmscheduler_add_idle_process(process_t *p) {
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
    DEBUG_MESSAGE("process_AddStartIdle: %s, pid: %d\n",
    			p->name,
				p->pid);

    return true;
}

int8_t mmscheduler_send_event(uint8_t pid, uint8_t event, void *data) {
	event_t ev;
	int8_t ret;
	uint8_t sr;

	ev.pid = pid;
	ev.event = event;
	ev.data = data;
	lock_interrupt(sr);
	ret = evQueue_write(&ev);
	restore_interrupt(sr);
	return ret;
}

int8_t mmscheduler_is_ev_queue_empty(void) {
    return fifo_is_empty(&ev_queue);
}

int8_t mmscheduler_run(void) {
	static event_t ev;
	static int8_t ret;

	while(1) {
		// get next event
		if((ret = evQueue_read(&ev)) == true) {
			// got a valid event, send it to the process
			mmscheduler_exec_process(ev.pid, ev.event, ev.data);
		}
		else {
			// ev_queue_data is empty, execute the idle task
			if(idle_process != NULL) {
				ret = idle_process->process(0, NULL);
			}
//#ifdef TEST_RUN
			if(ret == 0) {
				return (-1);
			}
//#endif
		}
	}
	return false;
}
