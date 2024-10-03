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
static task_t *idle_task;
static task_t *task_list[NB_OF_TASKS];	// =NULL: unused, free
static uint8_t task_count;
static uint8_t pid_count; /// pid == 0 should not exist

// - private (static) functions-------------------------------------------------

/**
 * find the task in the list by given pid
 * find only first occurence
 * @param   pid of task to find
 * @reutn   pointert to task_t   =NULL: could not find task with given pid
 *                                  else: valid pointer
 */
static task_t *scheduler_find_task_by_pid(uint8_t pid) {
    uint8_t n;
	DEBUG_PRINTF_MESSAGE("scheduler_find_task_by_pid(%d)\n", pid);
    for(n = 0; n < NB_OF_TASKS; n++) {
        if(task_list[n] != NULL) {
            if(task_list[n]->pid == pid) {
                // found task with same pid
				DEBUG_PRINTF_MESSAGE(" + found: %p\n", task_list[n]);
                return task_list[n];
            }
        }
    }
    // no task in list found
	DEBUG_PRINTF_MESSAGE(" + no task found\n");
    return NULL;
}

/**
 * remove a task from task_list given by pid
 * @param   pid of task to remove
 * @return  status =1: successfully removed task from task_list
 *                 =0: could not remove task from task_list
 * /
static int8_t task_RemoveFromtaskList(uint8_t pid);*/

/**
 * execute a task given by its PID
 * @param	pid		task identifier
 * @param	event	event for the task to execute
 * @param	data	additional data to task (if unused = NULL)
 * @return	status 	=true: OK, could execute task
 *					=false: error, could not execute task
 */
static int8_t scheduler_exec_task(uint8_t pid, uint8_t event, void *data) {
    task_t *p;
	DEBUG_PRINTF_MESSAGE("scheduler_exec_task(pid: %d, event: %d)\n", pid, event);
    // check if task exists
    if((p = scheduler_find_task_by_pid(pid)) == NULL) {
        // error, task does not exist
        return false;
    }

   	// is function pointer correctly set?
	if(p->task == NULL) {
		// error, function pointer is not set
		return false;
	}
    // check if task is not yet started
    if(p->state == TASK_STATE_NONE) {
        // task is not active
        return false;
    }

    DEBUG_PRINTF_MESSAGE("execute task \"%s\" (pid: %d, event: %d, data: %p)\n",
        p->name, p->pid, event, data);

	// OK, execute task
	p->state = TASK_STATE_RUNNING;
	if(p->task(event, data) == 0) {
	    // do not run this task anymore
		p->state = TASK_STATE_NONE;
	}
	else {
    	// task remains active
		p->state = TASK_STATE_ACTIVE;
	}
	return true;
}

// - public functions ----------------------------------------------------------

void scheduler_init(void) {
	// vars
	task_count = 0;
	pid_count = 0;  // 1st time: ++
	idle_task = NULL;
	memset((uint8_t *)task_list, 0, sizeof(task_list));
	events_init();
}

int8_t scheduler_add_task(task_t *p) {
    uint8_t n;

	// sanity tests
	if(p == NULL) {
		// error, no task
		return false;
	}
	if(p->task == NULL) {
		// error, no task_function defined
		return false;
	}

	// place task in task_list
	if(task_count >= NB_OF_TASKS) {
		// error, no more space for an additional task in the task_list
		return false;
	}
    for(n = 0; n < NB_OF_TASKS; n++) {
        if(task_list[n] == NULL) {
            // found empty space in task_list
            break;
        }
    }
    if(n >= NB_OF_TASKS) {
        // error, could not add task to list, no more space available
	    return false;
    }

    // found empty space, add task to task_list
    task_list[n] = p;
    task_count++;
    if(pid_count == 0) {
        pid_count = 1;
    }
    else {
        pid_count++;
    }
    // success, added task to task_list
    p->pid = pid_count;
    p->state = TASK_STATE_NONE;

    DEBUG_PRINTF_MESSAGE("task_Add: %s, pid: %d\n",
    		p->name,
			p->pid);
    return true;
}

int8_t scheduler_remove_task(task_t *p) {
    // not implemented yet
    return false;
}

int8_t scheduler_start_task(uint8_t pid) {
    task_t *p;
    // check if task exists
    if((p = scheduler_find_task_by_pid(pid)) == NULL) {
        // error, task does not exist
        return false;
    }
    // check if task is already started
    if(p->state != TASK_STATE_NONE) {
        // error, task is already started
        return false;
    }

	// start task
	p->state = TASK_STATE_ACTIVE;
	DEBUG_PRINTF_MESSAGE("task_Start: %s, pid: %d, state: %d\n",
			p->name,
			p->pid,
			p->state);
	return scheduler_send_event(pid, EV_START, NULL);
}

int8_t scheduler_stop_task(uint8_t pid) {
	// not implemented yet
    return false;
}

int8_t scheduler_add_idle_task(task_t *p) {
	// sanity tests
	if(p == NULL) {
		// error, no task
		return false;
	}
	if(p->task == NULL) {
		// error, no task_function defined
		return false;
	}

	// success, add task to idle_task
    idle_task = p;
	p->pid = TASK_PID_IDLE;
	p->state = TASK_STATE_NONE; // does not matter for idle_task
    DEBUG_PRINTF_MESSAGE("scheduler_add_idle_task: %s, pid: %d\n",
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
			// got a valid event, send it to the task
			scheduler_exec_task(ev.pid, ev.event, ev.data);
		}
		else {
			// ev_main_fifo_data is empty, execute the idle task
			if(idle_task != NULL) {
				ret = idle_task->task(0, NULL);
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
