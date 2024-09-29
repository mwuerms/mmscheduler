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
#define cNB_OF_PROCESSES         16 /// number of processes

/* - typedefs --------------------------------------------------------------- */
typedef int8_t (*process_func_t) (uint8_t event, void *data);

typedef struct {
  process_func_t  process;
  char *name;
  uint8_t pid;    /// PID: process identifier
  uint8_t state;  /// state of the process: none=0, started, running
} process_t;
// .pid
#define cPROCESS_PID_IDLE       0   // the idle process has pid = 0

// .state
#define cPROCESS_STATE_NONE     0
#define cPROCESS_STATE_ACTIVE   1
#define cPROCESS_STATE_RUNNING  2

// - public functions ----------------------------------------------------------

/**
 * initialize the scheduler module
 */
void scheduler_init(void);

/**
 * add a new process
 * @param	p	pointer to process context
 * @return	status 	=true: OK, could add process to process_list
 *					=false: error, could not add process to process_list
 */
int8_t scheduler_add_process(process_t *p);

/**
 * removes an existing  process
 * @param	pid		process identifier
 * @return	status 	=true: OK, could remove process from process_list
 *					=false: error, could not remove process to process_list
 */
int8_t scheduler_remove_process(process_t *p);

/**
 * start an existing process
 * @param	pid		process identifier
 * @return	status 	=true: OK, could start process
 *					=false: error, could not start process
 */
int8_t scheduler_start_process(uint8_t pid);

/**
 * stop an existing process
 * @param	pid		process identifier
 * @return	status 	=true: OK, could stop process
 *					=false: error, could not stop process
 */
int8_t scheduler_stop_process(uint8_t pid);

/**
 * add the idle process, this process does not need to be started
 * @param	p	pointer to process context
 * @return	status 	=true: OK, could add process to process_list
 *					=false: error, could not add process to process_list
 */
int8_t scheduler_add_idle_process(process_t *p);

/**
 * send an event to a process given by its PID
 * @param	pid		process identifier
 * @param	event	event for the process to execute
 * @param	data	additional data to process (if unused = NULL)
 * @return	status 	=true: OK, could add event to queue
 *					=false: error, could not add event to queue
 */
int8_t scheduler_send_event(uint8_t pid, uint8_t event, void *data);

/**
 * check if event queue is empty
 * @return  =true: fifo is indeed empty, =false: fifo is NOT empty
 */
int8_t scheduler_is_ev_queue_empty(void);

/**
 * run the process scheduler
 * note: this function should never return (endless loop)
 * @return	=false: error
 */
int8_t scheduler_run(void);

#endif // _MM_SCHEDULER_H_
