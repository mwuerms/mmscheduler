/**
 * Martin Egli
 * 2024-10-03
 * all needed functions to use power modes
 * coop scheduler for mcu
 * 
 * this file needs adaption to current mcu
 */

// - includes ------------------------------------------------------------------
//#define DEBUG_PRINTF_ON
#include "debug_printf.h"

#include <string.h>
#include "power_mode.h"
#include "events.h"

// - private variables ---------------------------------------------------------
static uint8_t power_mode_cnt[NB_OF_POWER_MODES];
#define POWER_MODE_CNT_MAX (250)

// - private functions ---------------------------------------------------------
/**
 * get the deepest power mode
 */
static inline uint8_t get_deepest_power_mode(void) {
	if(power_mode_cnt[POWER_MODE_NONE])
		return POWER_MODE_NONE;
	if(power_mode_cnt[POWER_MODE_1])
		return POWER_MODE_1;
	return POWER_MODE_2; // deepest anyways
}

// - public functions ----------------------------------------------------------
void power_mode_init(void) {
	memset(power_mode_cnt, 0, sizeof(power_mode_cnt));
}

void power_mode_request(uint8_t mode) {
	if(mode > POWER_MODE_2) {
		// unknown power mode
		return;
	}
	if(power_mode_cnt[mode] < POWER_MODE_CNT_MAX) {
		power_mode_cnt[mode]++;
	}
}

void power_mode_release(uint8_t mode) {
	if(mode > POWER_MODE_2) {
		// unknown power mode
		return;
	}
	if(power_mode_cnt[mode]) {
		power_mode_cnt[mode]--;
	}
}

void power_mode_sleep(void) {
	switch(get_deepest_power_mode()) {
		case POWER_MODE_NONE:
			// do not go to power, just idle here
			while (events_is_main_fifo_empty() == true);
			break;
		case POWER_MODE_1:
			// - mcu specific code here ------------
				while (events_is_main_fifo_empty() == true);
			break;
		case POWER_MODE_2:
			// - mcu specific code here ------------
				while (events_is_main_fifo_empty() == true);
			break;
	}
	return;
}
