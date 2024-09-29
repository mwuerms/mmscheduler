/**
 * Martin Egli
 * 2015-09-29
 * event queue
 * coop scheduler for mcu
 */

#ifndef _MM_EV_QUEUE_H_
#define _MM_EV_QUEUE_H_

//- includes -------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "arch.h"
#include "fifo.h"

//- typedefs -------------------------------------------------------------------
#define cNB_OF_EVENTS_IN_QUEUE   32 /// number of events in event queue
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
 * initialize the event queue
 */
void ev_queue_init(void);

/**
 * write an event to the event queue
 * @param   event   pointer to event to put into ev_queue_data
 * @return  =true: OK, writing event successfull
 *          =false: Error, could not write, queue is full
 */
uint8_t ev_queue_write(event_t *ev);

/**
 * read an event from the event queue
 * @param   event   pointer to event to read from ev_queue_data
 * @return  =true: OK, reading event successfull, event is valid
 *          =false: Error, could not read, ev_queue_data is empty
 */
uint8_t ev_queue_read(event_t *ev);

/**
 * check if event queue is empty
 * @return  =true: event queue is empty
 *          =false: event queue is NOT empty
 */
uint8_t ev_queue_is_empty(void);


#endif // _MM_EV_QUEUE_H_