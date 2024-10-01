/**
 * Martin Egli
 * 2024-09-29
 * events, queue and timing events (are sent later)
 * coop scheduler for mcu
 */

#ifndef _EVENTS_H_
#define _EVENTS_H_

//- includes -------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "arch.h"
#include "fifo.h"

//- typedefs -------------------------------------------------------------------
#define EVENTS_MAIN_FIFO_SIZE (8) /// number of events in event queue
typedef struct {
  void * data;
  uint8_t pid;
  uint8_t event;
} event_t;
// events from 0 ... 127 are for user purpose
// predefined events
#define EV_START   128
#define EV_STOP    129
#define EV_POLL    130

// - public functions ----------------------------------------------------------

/**
 * initialize the events
 */
void events_init(void);

/**
 * write an event to the event queue
 * @param   event   pointer to event to put into ev_queue_data
 * @return  =true: OK, writing event successfull
 *          =false: Error, could not write, queue is full
 */
uint8_t events_add_to_queue(event_t *ev);

/**
 * read an event from the event queue
 * @param   event   pointer to event to read from ev_queue_data
 * @return  =true: OK, reading event successfull, event is valid
 *          =false: Error, could not read, ev_queue_data is empty
 */
uint8_t events_get_from_queue(event_t *ev);

/**
 * check if event queue is empty
 * @return  =true: event queue is empty
 *          =false: event queue is NOT empty
 */
uint8_t events_is_queue_empty(void);

// - timing events -------------------------------------------------------------

/**
 * start the event timer
 * @param   periode   of event timer
 * @return  =true: could start event timer
 *          =false: could not start event timer
 */
int8_t events_start_timer(uint16_t periode);

/**
 * stop the event timer
 * @return  =true: could stop event timer
 *          =false: could not stop event timer
 */
int8_t events_stop_timer(void);

/**
 * add a single event to the event timer
 * @param   timeout after which to send the event
 * @param	pid		process identifier
 * @param	event	event for the process to execute
 * @param	data	additional data to process (if unused = NULL)
 * @return	status 	=true: OK, could add event
 *					=false: error, could not add event
 */
int8_t events_add_single_timer_event(uint16_t timeout, uint8_t pid, uint8_t event, void *data);

#endif // _EVENTS_H_
