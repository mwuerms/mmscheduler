/**
 * Martin Egli
 * 2024-09-29
 * events, main_fifo and timing events (are sent later)
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
typedef struct {
  void * data;
  uint8_t tid;
  uint8_t event;
} event_t;

// - events --------------------------------------------------------------------
// events from 0 ... 250 are for user purpose
// predefined events
#define EV_START   250
#define EV_STOP    251
#define EV_POLL    252

// - public functions ----------------------------------------------------------

/**
 * initialize the events
 */
void events_init(void);

/**
 * write an event to the event main_fifo
 * @param   event   pointer to event to put into ev_main_fifo_data
 * @return  =true: OK, writing event successfull
 *          =false: Error, could not write, main_fifo is full
 */
uint8_t events_add_to_main_fifo(event_t *ev);

/**
 * read an event from the event main_fifo
 * @param   event   pointer to event to read from ev_main_fifo_data
 * @return  =true: OK, reading event successfull, event is valid
 *          =false: Error, could not read, ev_main_fifo_data is empty
 */
uint8_t events_get_from_main_fifo(event_t *ev);

/**
 * check if event main_fifo is empty
 * @return  =true: event main_fifo is empty
 *          =false: event main_fifo is NOT empty
 */
uint8_t events_is_main_fifo_empty(void);

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
 * @param   event   pointer to event to put into ev_main_fifo_data
 * @return	status 	=true: OK, could add event
 *					=false: error, could not add event
 */
int8_t events_add_single_timer_event(uint16_t timeout, event_t *ev);

#endif // _EVENTS_H_
