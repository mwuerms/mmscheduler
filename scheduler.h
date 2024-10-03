/**
 * Martin Egli
 * 2015-09-28
 * scheduler
 * coop scheduler for mcu
 */

#ifndef _MM_SCHEDULER_H_
#define _MM_SCHEDULER_H_

/* - includes --------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>
#include "arch.h"
#include "events.h"

/* - defines ---------------------------------------------------------------- */
#define NB_OF_TASKS (16) /// number of tasks

/* - typedefs --------------------------------------------------------------- */
typedef int8_t (*task_func_t) (uint8_t event, void *data);

typedef struct {
  task_func_t  task;
  char *name;
  uint8_t pid;    /// PID: task identifier
  uint8_t state;  /// state of the task: none=0, started, running
} task_t;
// .pid
#define TASK_PID_IDLE (0)   // the idle task has pid = 0

// .state
#define TASK_STATE_NONE (0)
#define TASK_STATE_ACTIVE (1)
#define TASK_STATE_RUNNING (2)

// - public functions ----------------------------------------------------------

/**
 * initialize the scheduler module
 */
void scheduler_init(void);

/**
 * add a new task
 * @param	p	pointer to task context
 * @return	status 	=true: OK, could add task to task_list
 *					=false: error, could not add task to task_list
 */
int8_t scheduler_add_task(task_t *p);

/**
 * removes an existing  task
 * @param	pid		task identifier
 * @return	status 	=true: OK, could remove task from task_list
 *					=false: error, could not remove task to task_list
 */
int8_t scheduler_remove_task(task_t *p);

/**
 * start an existing task
 * @param	pid		task identifier
 * @return	status 	=true: OK, could start task
 *					=false: error, could not start task
 */
int8_t scheduler_start_task(uint8_t pid);

/**
 * stop an existing task
 * @param	pid		task identifier
 * @return	status 	=true: OK, could stop task
 *					=false: error, could not stop task
 */
int8_t scheduler_stop_task(uint8_t pid);

/**
 * add the idle task, this task does not need to be started
 * @param	p	pointer to task context
 * @return	status 	=true: OK, could add task to task_list
 *					=false: error, could not add task to task_list
 */
int8_t scheduler_add_idle_task(task_t *p);

/**
 * send an event to a task given by its PID
 * @param	pid		task identifier
 * @param	event	event for the task to execute
 * @param	data	additional data to task (if unused = NULL)
 * @return	status 	=true: OK, could add event to main_fifo
 *					=false: error, could not add event to main_fifo
 */
int8_t scheduler_send_event(uint8_t pid, uint8_t event, void *data);

/**
 * start the event timer
 * @return	status 	=true: OK, could add event to main_fifo
 *					=false: error, could not add event to main_fifo
 */
int8_t scheduler_start_event_timer(void);

/**
 * stop the event timer
 * @return	status 	=true: OK, could add event to main_fifo
 *					=false: error, could not add event to main_fifo
 */
int8_t scheduler_stop_event_timer(void);

/**
 * send an event timer to a task given by its PID
 * @param timeout after which to send
 * @param	pid		task identifier
 * @param	event	event for the task to execute
 * @param	data	additional data to task (if unused = NULL)
 * @return	status 	=true: OK, could add event to main_fifo
 *					=false: error, could not add event to main_fifo
 */
int8_t scheduler_add_timer_event(uint16_t timeout, uint8_t pid, uint8_t event, void *data);

/**
 * check if event main_fifo is empty
 * @return  =true: fifo is indeed empty, =false: fifo is NOT empty
 */
int8_t scheduler_is_event_main_fifo_empty(void);

/**
 * run the task scheduler
 * note: this function should never return (endless loop)
 * @return	=false: error
 */
int8_t scheduler_run(void);

#endif // _MM_SCHEDULER_H_
