/**
 * Martin Egli
 * 2024-10-03
 * all needed functions to use power modes
 * coop scheduler for mcu
 * 
 * this file needs adaption to current mcu
 */

#ifndef _POWER_MODE_H_
#define _POWER_MODE_H_

//- includes -------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "arch.h"

//- defines --------------------------------------------------------------------
// define power modes accordning to mcu
#define NB_OF_POWER_MODES (3)
#define POWER_MODE_NONE (0)
#define POWER_MODE_1 (1)
#define POWER_MODE_2 (2)

// - public functions ----------------------------------------------------------

/**
 * initialize the events
 */
void power_mode_init(void);

/**
 * request a certain power mode
 * @param mode  to request
 */
void power_mode_request(uint8_t mode);

/**
 * release a certain power mode
 * @param mode  to release
 */
void power_mode_release(uint8_t mode);

/**
 * wait here as long there is no event to process
 */
void power_mode_sleep(void);

#endif // _POWER_MODE_H_
