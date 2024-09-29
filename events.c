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

// - typedefs ------------------------------------------------------------------

// - private variables ---------------------------------------------------------

// - event queue ---------------------------------------------------------------
static fifo_t ev_queue;
static event_t ev_queue_data[cNB_OF_EVENTS_IN_QUEUE];


#define EV_TIMER_NB_EVENTS  (16)

typedef struct {
    uint32_t compare;
    event_t  event;
} ev_timer_event_t;

/* - variables -------------------------------------------------------------- */
uint8_t *ev_timer_pid;    /// point to pid of event timer
static process_t ev_timer_proc;
static char ev_timer_name[] = "EV_TIMER_HAL";
static fifo_t ev_timer_fifo;
static ev_timer_event_t ev_timer_fifo_data[EV_TIMER_NB_EVENTS];

/* - private function ------------------------------------------------------- */

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
    memset(&ev_timer_fifo_data, 0, sizeof(ev_timer_fifo_data));
    fifo_init(&ev_timer_fifo, ev_timer_fifo_data, EV_TIMER_NB_EVENTS);

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
    uint16_t sr, ev_compare, now = 0;
    uint8_t write_pos, write_pos_1, move_elements, found;
    ev_timer_event_t ev_to_save;
    uint16_t pos, rd, wr;
    fifo_t find_pos;

    DEBUG_PRINTF_MESSAGE("ev_timer_add_single_event(%d, %d, %d)\n", timeout, pid, event);
    // sanity check
    if(timeout == 0) {
        // invalid timeout
        DEBUG_PRINTF_MESSAGE("  timeout = 0\n");
        return false;
    }
    
    lock_interrupt(sr);

    // get next free element
    if(fifo_try_append(&ev_timer_fifo) == false) {
		// cannot append
		DEBUG_PRINTF_MESSAGE(" ev_timer fifo is full\n");
        restore_interrupt(sr);
		return false;
	}
    // calc compare for this event
    now = ev_timer_get_current_time_isr();
    ev_compare = now + timeout;

    // find position to sort this event in
    // use find_pos to iterate through ev_timer_fifo, do not change ev_timer_fifo
    /*memcpy(&find_pos, &ev_timer_fifo, sizeof(ev_timer_fifo));
    while(fifo_is_empty(&find_pos) == false) {
        fifo_try_get(&find_pos);
        if()
        fifo_finalize_get(&find_pos);
    }
*/    
    

    // sort event into list according to its duration value, smallest duration first
    // duration = compare - now
    

    // save event
    ev_timer_fifo_data[ev_timer_fifo.wr_proc].compare = ev_compare;
    ev_timer_fifo_data[ev_timer_fifo.wr_proc].event.data = data;
    ev_timer_fifo_data[ev_timer_fifo.wr_proc].event.pid  = pid;
    ev_timer_fifo_data[ev_timer_fifo.wr_proc].event.event  = event;

    fifo_finalize_append(&ev_timer_fifo);
	DEBUG_PRINTF_MESSAGE(" event: pid: %d, event: %d, data: %p (wr: %d, rd:%d, size: %d)\n",
				ev->pid, ev->event, ev->data,
				ev_queue.wr,
				ev_queue.rd,
				ev_queue.size);

    restore_interrupt(sr);
	return true;
}
