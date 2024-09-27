/**
 * Martin Egli
 * 2015-09-28
 * mmscheduler
 * coop scheduler for mcu
 */

#ifndef _MM_SCHEDULER_H_
#define _MM_SCHEDULER_H_

/* - includes --------------------------------------------------------------- */
#include <stdint.h>
#include <stdbool.h>
#include "arch.h"
#include "debug.h"

/* - defines ---------------------------------------------------------------- */
#define cNB_OF_PROCESSES         16 /// number of processes
#define cNB_OF_EVENTS_IN_QUEUE   32 /// number of events in event queue

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

typedef struct {
  void * data;
  uint8_t pid;
  uint8_t event;
} event_t;
// events from 0 ... 127 are for user purpose
// predefined events
#define cEV_START   128
#define cEV_STOP    129
#define cEV_POLL    130

// - public functions ----------------------------------------------------------

/**
 * initialize the scheduler module
 */
void mmscheduler_init(void);

/**
 * add a new process
 * @param	p	pointer to process context
 * @return	status 	=true: OK, could add process to process_list
 *					=false: error, could not add process to process_list
 */
int8_t mmscheduler_add_process(process_t *p);

/**
 * removes an existing  process
 * @param	pid		process identifier
 * @return	status 	=true: OK, could remove process from process_list
 *					=false: error, could not remove process to process_list
 */
int8_t mmscheduler_remove_process(process_t *p);

/**
 * start an existing process
 * @param	pid		process identifier
 * @return	status 	=true: OK, could start process
 *					=false: error, could not start process
 */
int8_t mmscheduler_start_process(uint8_t pid);

/**
 * stop an existing process
 * @param	pid		process identifier
 * @return	status 	=true: OK, could stop process
 *					=false: error, could not stop process
 */
int8_t mmscheduler_stop_process(uint8_t pid);

/**
 * add the idle process, this process does not need to be started
 * @param	p	pointer to process context
 * @return	status 	=true: OK, could add process to process_list
 *					=false: error, could not add process to process_list
 */
int8_t mmscheduler_add_idle_process(process_t *p);

/**
 * send an event to a process given by its PID
 * @param	pid		process identifier
 * @param	event	event for the process to execute
 * @param	data	additional data to process (if unused = NULL)
 * @return	status 	=true: OK, could add event to queue
 *					=false: error, could not add event to queue
 */
int8_t mmscheduler_send_event(uint8_t pid, uint8_t event, void *data);

/**
 * check if event queue is empty
 * @return  =true: fifo is indeed empty, =false: fifo is NOT empty
 */
int8_t mmscheduler_is_ev_queue_empty(void);

/**
 * run the process scheduler
 * note: this function should never return (endless loop)
 * @return	=false: error
 */
int8_t mmscheduler_run(void);

#endif // _MM_SCHEDULER_H_
