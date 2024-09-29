/**
 * Martin Egli
 * 2015-09-29
 * event queue
 * coop scheduler for mcu
 */

// - includes ------------------------------------------------------------------
#include "ev_queue.h"
#include <string.h>

#define DEBUG_PRINT_ON
#include "debug_printf.h"

// - typedefs ------------------------------------------------------------------

// - private variables ---------------------------------------------------------
// event queue
static fifo_t ev_queue;
static event_t ev_queue_data[cNB_OF_EVENTS_IN_QUEUE];

// - public functions ----------------------------------------------------------
void ev_queue_init(void) {
	fifo_init(&ev_queue, (void *)ev_queue_data, cNB_OF_EVENTS_IN_QUEUE);
	memset((uint8_t *)ev_queue_data, 0, sizeof(ev_queue_data));
}

uint8_t ev_queue_write(event_t *ev) {
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

uint8_t ev_queue_read(event_t *ev) {
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

uint8_t ev_queue_is_empty(void) {
    return fifo_is_empty(&ev_queue);
}
