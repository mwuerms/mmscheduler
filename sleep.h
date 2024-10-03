/**
 * Martin Egli
 * 2024-10-03
 * all needed functions to use sleep modes
 * coop scheduler for mcu
 * 
 * this file needs adaption to current mcu
 */

#ifndef _SLEEP_H_
#define _SLEEP_H_

//- includes -------------------------------------------------------------------
#include <stdint.h>
#include <stdbool.h>
#include "arch.h"

//- defines --------------------------------------------------------------------
// define sleep modes accordning to mcu
#define NB_OF_SLEEP_MODES (3)
#define SLEEP_MODE_NONE (0)
#define SLEEP_MODE_1 (1)
#define SLEEP_MODE_2 (2)

// - public functions ----------------------------------------------------------

/**
 * initialize the events
 */
void sleep_init(void);

/**
 * request a certain sleep mode
 * @param mode  to request
 */
void sleep_request_mode(uint8_t mode);

/**
 * release a certain sleep mode
 * @param mode  to release
 */
void sleep_release_mode(uint8_t mode);

/**
 * wait here as long there is no event to process
 */
void sleep_wait_for_events(void);

#endif // _SLEEP_H_
