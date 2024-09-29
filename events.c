/**
 * Martin Egli
 * 2024-09-29
 * events, queue and timing events (are sent later)
 * coop scheduler for mcu
 */

// - includes ------------------------------------------------------------------
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "events.h"
#include "scheduler.h"
#include "fifo.h"

#define DEBUG_PRINT_ON
#include "debug_printf.h"

// - private variables ---------------------------------------------------------
// - event queue ---------------------------------------------------------------
static fifo_t ev_queue;
static event_t ev_queue_data[cNB_OF_EVENTS_IN_QUEUE];

// - timer events --------------------------------------------------------------
#define EV_TIMER_NB_EVENTS  (16)

typedef struct {
    uint32_t compare;
    event_t  event;
} ev_timer_event_t;

uint8_t *ev_timer_pid;    /// point to pid of event timer
static process_t ev_timer_proc;
static char ev_timer_name[] = "EV_TIMER_HAL";
static fifo_t events_timer_fifo;
static ev_timer_event_t events_timer_fifo_data[EV_TIMER_NB_EVENTS];

// - private function ----------------------------------------------------------
void events_print_event_queue(void) {
	DEBUG_PRINTF_MESSAGE("current events in queue\n");
}
void events_print_timer_events(void) {
	DEBUG_PRINTF_MESSAGE("current timer events\n");
}

// hardware: ISR, or what else, This has to be ported to hardware.
/**
 * the event timer process
 * process the next event to send
 */
static int32_t ev_timer_CNT = 0;
static uint32_t ev_timer_get_current_time(void) {
    uint32_t time;
    uint16_t sr;
    
    lock_interrupt(sr);
    time = ev_timer_CNT;
    restore_interrupt(sr);

    return time;
}

static uint32_t ev_timer_get_current_time_isr(void) {
    return ev_timer_CNT;
}

/**
 * compare 2 times t1, t2
 * @return 	=+1: t1  > t2
 * 			=0:  t1 == t2
 * 			=-1: t1  < t2
 */
static int8_t events_compare_times(uint32_t t1, uint32_t t2) {
	if(t1 == t2) {
		return 0;
	}
	if(t1 > t2) {
		return 1;
	}
	return -1;
}

/**
 * move the elements in events_timer_fifo_data 1 position to the right
 * to make space for 1 new element
 * use vars 
 * events_timer_fifo (do not change)
 * events_timer_fifo_data (change)
 * @param	from	start position to move from
 * @param	to		end position
 */
static void events_move_elements_in_timer_fifo_1pos_right(uint16_t from, uint16_t to) {
	uint16_t pos, pos1;
	for(pos  = from;pos != to;pos  = pos1) {
		pos1 = fifo_dec_pos(pos, events_timer_fifo.size);
		memcpy(&events_timer_fifo_data[pos], &events_timer_fifo_data[pos1], sizeof(events_timer_fifo_data[pos]));
	}
}

static int8_t ev_timer_hal_process(uint8_t event, void *data) {
    ev_timer_CNT++;
    return(1);
}


// - public functions ----------------------------------------------------------
void events_init(void) {
	// event queue
	fifo_init(&ev_queue, (void *)ev_queue_data, cNB_OF_EVENTS_IN_QUEUE);
	memset((uint8_t *)ev_queue_data, 0, sizeof(ev_queue_data));
	// timing events
	// vars
    ev_timer_pid = &ev_timer_proc.pid;
    ev_timer_proc.name = ev_timer_name;
    ev_timer_proc.process = ev_timer_hal_process;
    memset(&ev_timer_proc, 0, sizeof(ev_timer_proc));
    memset(&events_timer_fifo_data, 0, sizeof(events_timer_fifo_data));
    fifo_init(&events_timer_fifo, events_timer_fifo_data, EV_TIMER_NB_EVENTS);

    ev_timer_CNT = 0;
    scheduler_add_process(&ev_timer_proc);
}

uint8_t events_add_to_queue(event_t *ev) {
	DEBUG_PRINTF_MESSAGE("ev_queue_write: (wr: %d, rd:%d, size: %d)",
				ev_queue.wr,
				ev_queue.rd,
				ev_queue.size);
	// sanity checks
	if(ev == NULL) {
		DEBUG_PRINTF_MESSAGE(" ev == NULL\n");
		return false;
	}
	if(fifo_try_append(&ev_queue) == false) {
		// cannot append
		DEBUG_PRINTF_MESSAGE(" event queue is full\n");
		return false;
	}
	memcpy((uint8_t *)&ev_queue_data[ev_queue.wr_proc], (uint8_t *)ev, sizeof(*ev));
	fifo_finalize_append(&ev_queue);
	DEBUG_PRINTF_MESSAGE(" event: pid: %d, event: %d, data: %p (wr: %d, rd:%d, size: %d)\n",
				ev->pid, ev->event, ev->data,
				ev_queue.wr,
				ev_queue.rd,
				ev_queue.size);
	return true;
}

uint8_t events_get_from_queue(event_t *ev) {
	DEBUG_PRINTF_MESSAGE("ev_queue_read:  (wr: %d, rd:%d, size: %d)",
				ev_queue.wr,
				ev_queue.rd,
				ev_queue.size);
    // sanity checks
	if(ev == NULL) {
		DEBUG_PRINTF_MESSAGE(" ev == NULL\n");
		return false;
	}
	if(fifo_try_get(&ev_queue) == false) {
		// cannot append
		DEBUG_PRINTF_MESSAGE(" event queue is empty\n");
		return false;
	}
	memcpy((uint8_t *)ev, (uint8_t *)&ev_queue_data[ev_queue.rd_proc], sizeof(*ev));
	fifo_finalize_get(&ev_queue);
	DEBUG_PRINTF_MESSAGE(" event: pid: %d, event: %d, data: %p (wr: %d, rd:%d, size: %d)\n",
				ev->pid, ev->event, ev->data,
				ev_queue.wr,
				ev_queue.rd,
				ev_queue.size);
	return true;
}

uint8_t events_is_queue_empty(void) {
    return fifo_is_empty(&ev_queue);
}

// - timing events -------------------------------------------------------------

int8_t events_start_timer(uint16_t periode) {
	// start the timer
    return scheduler_start_process(ev_timer_proc.pid);
}

int8_t events_stop_timer(void) {
    return scheduler_stop_process(ev_timer_proc.pid);
}

int8_t events_add_single_timer_event(uint16_t timeout, uint8_t pid, uint8_t event, void *data)  {
	uint32_t new_compare, now = 0;
    uint16_t sr, pos;

    DEBUG_PRINTF_MESSAGE("events_add_single_timer_event(%d, %d, %d)\n", timeout, pid, event);
    // sanity check
    if(timeout == 0) {
        // invalid timeout
        DEBUG_PRINTF_MESSAGE(" timeout = 0, skip\n");
        return false;
    }
    
    lock_interrupt(sr);

    // get next free element
    if(fifo_try_append(&events_timer_fifo) == false) {
		// cannot append
		DEBUG_PRINTF_MESSAGE(" events_timer_fifo fifo is full, skip\n");
        restore_interrupt(sr);
		return false;
	}
    // calc compare for this event
    now = ev_timer_get_current_time_isr();
    new_compare = now + timeout;
    // find position to sort this event in
	if(fifo_is_empty(&events_timer_fifo) == true) {
		// fifo is empty, so save event
		DEBUG_PRINTF_MESSAGE(" events_timer_fifo fifo is empty, place up front, done\n");
		events_timer_fifo_data[events_timer_fifo.wr_proc].compare = new_compare;
		events_timer_fifo_data[events_timer_fifo.wr_proc].event.data = data;
		events_timer_fifo_data[events_timer_fifo.wr_proc].event.pid  = pid;
		events_timer_fifo_data[events_timer_fifo.wr_proc].event.event  = event;
	}
	else {
		/* iterate through events_timer_fifo to find appropriate place fro new_compare
		 * note: use wr_proc here, because fifo_try_append() was called to check if there is space left
		 * fifo_finalize_append() will get called later
		 */
		for(pos = events_timer_fifo.rd; pos != events_timer_fifo.wr_proc; pos = fifo_inc_pos(pos, 1)) {
			/* check compare values, current_compare(@pos) vs. new_compare
			* < : current_compare(@pos) will be used 1st, nothing to do
			* ==: current_compare(@pos) will also be used 1st, nothing to do
			* > : new_compare will be used 1st, so make space at pos by copy pos to pos+1
			*/
			if(events_compare_times(events_timer_fifo_data[pos].compare, new_compare) == 1) {
				// current current_compare(@pos) > new_compare
				// make space for new_compare at pos
				events_move_elements_in_timer_fifo_1pos_right(pos, events_timer_fifo.wr_proc);
				// place new_compare here at pos
				events_timer_fifo_data[pos].compare = new_compare;
				events_timer_fifo_data[pos].event.data = data;
				events_timer_fifo_data[pos].event.pid  = pid;
				events_timer_fifo_data[pos].event.event  = event;
			}
		}
	}

    fifo_finalize_append(&events_timer_fifo);
	DEBUG_PRINTF_MESSAGE(" event: pid: %d, event: %d, data: %p (wr: %d, rd:%d, size: %d)\n",
				ev->pid, ev->event, ev->data,
				ev_queue.wr,
				ev_queue.rd,
				ev_queue.size);

    restore_interrupt(sr);
	return true;
}
