/**
 * Martin Egli
 * 2024-10-03
 * all needed functions to use sleep modes
 * coop scheduler for mcu
 * 
 * this file needs adaption to current mcu
 */

// - includes ------------------------------------------------------------------
//#define DEBUG_PRINTF_ON
#include "debug_printf.h"

#include <string.h>
#include "sleep.h"
#include "events.h"

// - private variables ---------------------------------------------------------
static uint16_t sleep_mode_cnt[NB_OF_SLEEP_MODES];
#define SLEEP_MODE_CNT_MAX (128)

// - private functions ---------------------------------------------------------
/**
 * get the deepest sleep mode
 */
static inline uint8_t get_deepest_sleep_mode(void) {
	if(sleep_mode_cnt[SLEEP_MODE_NONE])
		return SLEEP_MODE_NONE;
	if(sleep_mode_cnt[SLEEP_MODE_1])
		return SLEEP_MODE_1;
	return SLEEP_MODE_2; // deepest anyways
}

// - public functions ----------------------------------------------------------
void sleep_init(void) {
	memset(sleep_mode_cnt, 0, sizeof(sleep_mode_cnt));
}

void sleep_request_mode(uint8_t mode) {
	if(mode > SLEEP_MODE_2) {
		// unknown sleep mode
		return;
	}
	if(sleep_mode_cnt[mode] < SLEEP_MODE_CNT_MAX) {
		sleep_mode_cnt[mode]++;
	}
}

void sleep_release_mode(uint8_t mode) {
	if(mode > SLEEP_MODE_2) {
		// unknown sleep mode
		return;
	}
	if(sleep_mode_cnt[mode]) {
		sleep_mode_cnt[mode]--;
	}
}

void sleep_wait_for_events(void) {
	switch(get_deepest_sleep_mode()) {
		case SLEEP_MODE_NONE:
			// do not go to sleep, just idle here
			while (events_is_main_fifo_empty() == true);
			break;
		case SLEEP_MODE_1:
			// - mcu specific code here ------------
				while (events_is_main_fifo_empty() == true);
			break;
		case SLEEP_MODE_2:
			// - mcu specific code here ------------
				while (events_is_main_fifo_empty() == true);
			break;
	}
	return;
}
