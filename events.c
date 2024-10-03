/**
 * Martin Egli
 * 2024-09-29
 * events, main_fifo and timing events (are sent later)
 * coop scheduler for mcu
 */

// - includes ------------------------------------------------------------------
//#define DEBUG_PRINTF_ON
#include "debug_printf.h"

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "events.h"
#include "scheduler.h"
#include "fifo.h"

// - private variables ---------------------------------------------------------
// - event main_fifo ---------------------------------------------------------------
#define EVENTS_MAIN_FIFO_SIZE (32) /// number of events in event main_fifo

static fifo_t events_main_fifo;
static event_t events_main_fifo_data[EVENTS_MAIN_FIFO_SIZE];

// - timer events --------------------------------------------------------------
#define EV_TIMER_NB_EVENTS  (32)
typedef struct {
    uint32_t compare;
	uint16_t ctrl;
    event_t  event;
} ev_tim_event_t;
#define EV_TIMER_CTRL_ACTIVE (1<<0)

static fifo_t events_timer_fifo;
static ev_tim_event_t events_timer_fifo_data[EV_TIMER_NB_EVENTS];

static task_t ev_timer_proc;
static char ev_timer_name[] = "EV_TIMER_HAL";

// - private function ----------------------------------------------------------
#ifdef DEBUG_PRINTF_ON
void events_print_event_main_fifo(void) {
	uint16_t pos;
	DEBUG_PRINTF_MESSAGE("events_print_event_main_fifo\n");
	DEBUG_PRINTF_MESSAGE(" wr: %d, rd:%d, size: %d\n",
				events_main_fifo.wr,
				events_main_fifo.rd,
				events_main_fifo.size);
	for(pos = fifo_next_pos(events_main_fifo.rd, events_main_fifo.size); pos != events_main_fifo.wr; pos = fifo_next_pos(pos, events_main_fifo.size)) {
		DEBUG_PRINTF_MESSAGE(" pos: %d, tid: %d, event: 0x%02X\n", 
				pos, 
				events_main_fifo_data[pos].tid, 
				events_main_fifo_data[pos].event);
	}
	DEBUG_PRINTF_MESSAGE(" pos: %d, tid: %d, event: 0x%02X\n", 
				pos, 
				events_main_fifo_data[pos].tid, 
				events_main_fifo_data[pos].event);
}
#else
#define events_print_event_main_fifo()
#endif

#ifdef DEBUG_PRINTF_ON
void events_print_timer_events(void) {
	uint16_t pos;
	DEBUG_PRINTF_MESSAGE("events_print_timer_events()\n");
	DEBUG_PRINTF_MESSAGE(" wr: %d, rd:%d, size: %d\n", events_timer_fifo.wr, events_timer_fifo.rd, events_timer_fifo.size);
	for(pos = fifo_next_pos(events_timer_fifo.rd, events_main_fifo.size); pos != events_timer_fifo.wr; pos = fifo_next_pos(pos, events_timer_fifo.size)) {
	//for(pos = events_timer_fifo.rd; pos != events_timer_fifo.wr; pos = fifo_next_pos(pos, events_timer_fifo.size)) {
		DEBUG_PRINTF_MESSAGE(" pos: %d, compare: %d, tid: %d, event: 0x%02X\n", pos, events_timer_fifo_data[pos].compare, events_timer_fifo_data[pos].event.tid, events_timer_fifo_data[pos].event.event);
	}
	DEBUG_PRINTF_MESSAGE(" pos: %d, compare: %d, tid: %d, event: 0x%02X\n", pos, events_timer_fifo_data[pos].compare, events_timer_fifo_data[pos].event.tid, events_timer_fifo_data[pos].event.event);
}
#else
#define events_print_timer_events()
#endif

// - timer callback, ISR -------------------------------------------------------
// hardware: ISR, or what else, This has to be ported to hardware.
/**
 * the event timer task
 * task the next event to send
 */
#include <stdio.h>
static uint32_t ev_timer_CNT = 0;
static uint32_t ev_timer_COMPARE = 0;

static inline void get_compare_from_timer_event_fifo(void) {
	uint16_t n;
	ev_timer_COMPARE = 0; // if none available
	// find the 1st active timer interrupt
	for(n = events_timer_fifo.size; n != 0; n --) {
		if(fifo_try_get(&events_timer_fifo) == true) {
			if((events_timer_fifo_data[events_timer_fifo.rd_proc].ctrl & EV_TIMER_CTRL_ACTIVE) == 0) {
				// this timer event is not active, so skip this one
				fifo_finalize_get(&events_timer_fifo);
			}
			else {
				// this timer event is active, take this one
				ev_timer_COMPARE = events_timer_fifo_data[events_timer_fifo.rd_proc].compare;
				break;
			}
		}
		else {
			// no more timer events in fifo available, stop here
			break;
		}
	}
	DEBUG_PRINTF_MESSAGE(" current compare: %d\n", ev_timer_COMPARE);
}

static int8_t ev_timer_hal_task(uint8_t event, void *data) {
	uint16_t n, sr;
    ev_timer_CNT++;
	printf("ev_timer_hal_task(ev: %d)\n", event);
	printf("  CNT:     %d\n", ev_timer_CNT);
	printf("  COMPARE: %d\n", ev_timer_COMPARE);
	if(ev_timer_CNT == ev_timer_COMPARE) {
		lock_interrupt(sr);
		printf("  COMPARE == CNT\n");

		for(n = events_timer_fifo.size; n != 0; n --) {
			// get the first timer event
			if(fifo_try_get(&events_timer_fifo) == true) {
				if(events_timer_fifo_data[events_timer_fifo.rd_proc].ctrl & EV_TIMER_CTRL_ACTIVE) {
					// this timer event is active, does it macht?
					if(events_timer_fifo_data[events_timer_fifo.rd_proc].compare == ev_timer_CNT) {
						// match: send the timer event and set this timer event inactive
						printf("  match at CNT: %d\n", ev_timer_CNT);
						scheduler_send_event(events_timer_fifo_data[events_timer_fifo.rd_proc].event.tid,
							events_timer_fifo_data[events_timer_fifo.rd_proc].event.event,
							events_timer_fifo_data[events_timer_fifo.rd_proc].event.data);
						// this timer event is done, set it inactive
						events_timer_fifo_data[events_timer_fifo.rd_proc].ctrl &= ~EV_TIMER_CTRL_ACTIVE;
						fifo_finalize_get(&events_timer_fifo);
						// now again check if there is yet an other timer event available
						get_compare_from_timer_event_fifo();
					}
				}
				else {
					// this timer event was not active! get the next one, also do not compare
					get_compare_from_timer_event_fifo();
					break;
				}
			}
		}
	}

	restore_interrupt(sr);
    return(1);
}

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
static void events_move_elements_in_timer_fifo_right(uint16_t from, uint16_t to) {
	uint16_t pos, pos1;
	for(pos  = from;pos != to;pos  = pos1) {
		pos1 = fifo_prev_pos(pos, events_timer_fifo.size);
		memcpy(&events_timer_fifo_data[pos], &events_timer_fifo_data[pos1], sizeof(events_timer_fifo_data[pos]));
	}
}

// - public functions ----------------------------------------------------------
void events_init(void) {
	// event main_fifo
	fifo_init(&events_main_fifo, (void *)events_main_fifo_data, EVENTS_MAIN_FIFO_SIZE);
	memset((uint8_t *)events_main_fifo_data, 0, sizeof(events_main_fifo_data));
	// timing events
    memset(&ev_timer_proc, 0, sizeof(ev_timer_proc));
    memset(&events_timer_fifo_data, 0, sizeof(events_timer_fifo_data));
    fifo_init(&events_timer_fifo, events_timer_fifo_data, EV_TIMER_NB_EVENTS);

    ev_timer_CNT = 0;
    ev_timer_proc.name = ev_timer_name;
    ev_timer_proc.task = ev_timer_hal_task;
    scheduler_add_task(&ev_timer_proc);
}

uint8_t events_add_to_main_fifo(event_t *ev) {
	uint16_t sr;
	DEBUG_PRINTF_MESSAGE("events_main_fifo_write: (wr: %d, rd:%d, size: %d)",
				events_main_fifo.wr,
				events_main_fifo.rd,
				events_main_fifo.size);
	// sanity checks
	if(ev == NULL) {
		DEBUG_PRINTF_MESSAGE(" ev == NULL\n");
		return false;
	}
	lock_interrupt(sr);
	if(fifo_try_append(&events_main_fifo) == false) {
		// cannot append
		DEBUG_PRINTF_MESSAGE(" event main_fifo is full\n");
		restore_interrupt(sr);
		return false;
	}
	memcpy((uint8_t *)&events_main_fifo_data[events_main_fifo.wr_proc], (uint8_t *)ev, sizeof(*ev));
	fifo_finalize_append(&events_main_fifo);
	DEBUG_PRINTF_MESSAGE(" event: tid: %d, event: %d, data: %p (wr: %d, rd:%d, size: %d)\n",
				ev->tid, ev->event, ev->data,
				events_main_fifo.wr,
				events_main_fifo.rd,
				events_main_fifo.size);
	events_print_event_main_fifo();

	restore_interrupt(sr);
	return true;
}

uint8_t events_get_from_main_fifo(event_t *ev) {
	uint16_t sr;
	DEBUG_PRINTF_MESSAGE("events_get_from_main_fifo():  (wr: %d, rd:%d, size: %d)\n",
				events_main_fifo.wr, events_main_fifo.rd, events_main_fifo.size);
    // sanity checks
	if(ev == NULL) {
		DEBUG_PRINTF_MESSAGE(" ev == NULL\n");
		return false;
	}
	lock_interrupt(sr); restore_interrupt(sr);
	if(fifo_try_get(&events_main_fifo) == false) {
		// cannot append
		DEBUG_PRINTF_MESSAGE(" event main_fifo is empty\n");
		return false;
	}
	memcpy((uint8_t *)ev, (uint8_t *)&events_main_fifo_data[events_main_fifo.rd_proc], sizeof(*ev));
	fifo_finalize_get(&events_main_fifo);
	DEBUG_PRINTF_MESSAGE(" event: tid: %d, event: %d, data: %p (wr: %d, rd:%d, size: %d)\n", 
				ev->tid, ev->event, ev->data, events_main_fifo.wr, events_main_fifo.rd, events_main_fifo.size);
	restore_interrupt(sr);
	return true;
}

uint8_t events_is_main_fifo_empty(void) {
    return fifo_is_empty(&events_main_fifo);
}

// - timing events -------------------------------------------------------------

int8_t events_start_timer(uint16_t periode) {
	// start the timer
    return scheduler_start_task(ev_timer_proc.tid);
}

int8_t events_stop_timer(void) {
    return scheduler_stop_task(ev_timer_proc.tid);
}

static inline uint32_t events_calc_compare(uint32_t now, uint16_t timeout) {
	// + -> this takes care of wrap arround it self
	return now + timeout;
}

static inline uint16_t events_calc_timeout(uint32_t now, uint32_t compare) {
	// compare lies ahead of now
	// - -> take care of wrap arround
	if(now <= compare) {
		return compare - now;
	}
	return 0xFFFFFFFF - now + compare;
}

int8_t events_add_single_timer_event(uint16_t timeout, event_t *ev)  {
	uint32_t new_compare, pos_compare, now = 0;
    uint16_t sr, pos, pos_timeout, pos_ctrl;

    // sanity check
	if(ev == NULL) {
		DEBUG_PRINTF_MESSAGE(" ev == NULL\n");
		return false;
	}
	DEBUG_PRINTF_MESSAGE("events_add_single_timer_event(%d, %d, %d)\n", timeout, ev->tid, ev->event);
    if(timeout == 0) {
        // invalid timeout
        DEBUG_PRINTF_MESSAGE(" timeout = 0, skip\n");
        return false;
    }
    
    lock_interrupt(sr);
	// calc compare for this event
    now = ev_timer_get_current_time_isr();
	new_compare = events_calc_compare(now, timeout);

    // get next free element
    if(fifo_try_append(&events_timer_fifo) == false) {
		// cannot append
		DEBUG_PRINTF_MESSAGE(" events_timer_fifo fifo is full, skip\n");
        restore_interrupt(sr);
		return false;
	}
    // find position to sort this event in
	if(fifo_is_empty(&events_timer_fifo) == true) {
		// fifo is empty, so save event
		DEBUG_PRINTF_MESSAGE(" events_timer_fifo fifo is empty, place up front, done\n");
		events_timer_fifo_data[events_timer_fifo.wr_proc].compare = new_compare;
		events_timer_fifo_data[events_timer_fifo.wr_proc].ctrl = EV_TIMER_CTRL_ACTIVE;
		events_timer_fifo_data[events_timer_fifo.wr_proc].event.data = ev->data;
		events_timer_fifo_data[events_timer_fifo.wr_proc].event.tid  = ev->tid;
		events_timer_fifo_data[events_timer_fifo.wr_proc].event.event  = ev->event;
	}
	else {
		/* iterate through events_timer_fifo to find appropriate place fro new_compare
		 * note: use wr_proc here, because fifo_try_append() was called to check if there is space left
		 * fifo_finalize_append() will get called later
		 */
		DEBUG_PRINTF_MESSAGE(" sort timer event to correct position\n");
		for(pos = fifo_next_pos(events_timer_fifo.rd, events_main_fifo.size); pos != events_timer_fifo.wr_proc; pos = fifo_next_pos(pos, events_timer_fifo.size)) {
			DEBUG_PRINTF_MESSAGE(" + pos: %d\n", pos);
			/* check timeout values, pos_timeout(@pos) vs. timeout (from call)
			* < : pos_timeout(@pos) will be used 1st, nothing to do
			* ==: pos_timeout(@pos) will also be used 1st, nothing to do
			* > : timeout will be used 1st, so make space at pos by copy pos to pos+1
			*/
			pos_ctrl = events_timer_fifo_data[pos].ctrl;
			pos_compare = events_timer_fifo_data[pos].compare;
			pos_timeout = events_calc_timeout(now, pos_compare);
			DEBUG_PRINTF_MESSAGE("   pos_compare: %d, new_compare: %d, pos_ctrl: %d\n", pos_compare, new_compare, events_timer_fifo_data[pos].ctrl);
			DEBUG_PRINTF_MESSAGE("   pos_timeout: %d, new_timeout: %d\n", pos_timeout, timeout);
			//if(events_compare_times(pos_compare, new_compare) == -1) {
			if((pos_ctrl & EV_TIMER_CTRL_ACTIVE) == 0) {
				DEBUG_PRINTF_MESSAGE("   pos_ctrl unused, put new timeout here at pos\n");
				// found an empty place, put new timeout here at pos
				break;
			}
			if(pos_timeout > timeout) {
				DEBUG_PRINTF_MESSAGE("   pos_timeout > timeout, so @pos will come later, make space for new timeout\n");
				// current current_compare(@pos) > new_compare
				// make space for new_compare at pos
				events_move_elements_in_timer_fifo_right(events_timer_fifo.wr_proc, pos);
				break;
			}
		}
		// place new_compare here at pos
		events_timer_fifo_data[pos].compare = new_compare;
		events_timer_fifo_data[pos].ctrl = EV_TIMER_CTRL_ACTIVE;
		events_timer_fifo_data[pos].event.data = ev->data;
		events_timer_fifo_data[pos].event.tid  = ev->tid;
		events_timer_fifo_data[pos].event.event  = ev->event;
	}
    fifo_finalize_append(&events_timer_fifo);
	DEBUG_PRINTF_MESSAGE(" event: tid: %d, event: %d, data: %p (wr: %d, rd:%d, size: %d)\n",
				ev->tid, ev->event, ev->data, events_timer_fifo.wr, events_timer_fifo.rd, events_timer_fifo.size);

	events_print_timer_events();
	// get first compare value
	get_compare_from_timer_event_fifo();

    restore_interrupt(sr);
	return true;
}
