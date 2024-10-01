/**
 * Martin Egli
 * 2023-09-02
 * mmlib https://github.com/mwuerms/mmlib
 * fifo functions
 */

#ifndef _MM_FIFO_H_
#define _MM_FIFO_H_
 
// - use c standard libraries --------------------------------------------------
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

// - typedefs ------------------------------------------------------------------

/**
 * fifo control
 * holds pointer to data, read and write positions and size of the fifo
 */
typedef struct {
    void *data; // pointer to some data, is only used by calling functions not by fifo functions
    uint16_t wr; // write position
    uint16_t wr_proc; // use this write position after `fifo_try_append()`, its value will get stored in `wr` with `fifo_finalize_append()`
    uint16_t rd; // read position
    uint16_t rd_proc; // use this read position after `fifo_try_get()`, its value will get stored in `rd` with `fifo_finalize_get()`
    uint16_t size; // of the fifo
} fifo_t;

// - public functions ----------------------------------------------------------

/**
 * initialize a fifo_t with data and to a given size
 * @param   f       pointer to fifo_t
 * @param   data    pointer to data
 * @param   size    size of fifo
 * @return  =true: success, =false: error, invalid pointer 
 */
uint16_t fifo_init(fifo_t *f, void *data, uint16_t size);

/**
 * check if a fifo_t is full
 * @param   f       pointer to fifo_t
 * @return  =true: fifo is indeed full, =false: fifo is NOT full
 */
uint16_t fifo_is_full(fifo_t *f);

/**
 * check if a fifo_t is empty
 * @param   f       pointer to fifo_t
 * @return  =true: fifo is indeed empty, =false: fifo is NOT empty
 */
uint16_t fifo_is_empty(fifo_t *f);

/**
 * try to append to fifo
 * @param   f       pointer to fifo_t
 * @return  =true: success, can append, =false: error, fifo is full
 */
uint16_t fifo_try_append(fifo_t *f);

/**
 * finalize append to fifo
 * @param   f       pointer to fifo_t
 * @return  =true: success, =false: error, invalid pointer
 */
uint16_t fifo_finalize_append(fifo_t *f);

/**
 * try to get from fifo
 * @param   f       pointer to fifo_t
 * @return  =true: success, can get, =false: error, fifo is empty
 */
uint16_t fifo_try_get(fifo_t *f);

/**
 * finalize get fromfifo
 * @param   f       pointer to fifo_t
 * @return  =true: success, =false: error, invalid pointer
 */
uint16_t fifo_finalize_get(fifo_t *f);

/**
 * calc the next position index (pos)
 * @param   pos     current position
 * @param   size    to calc maximum
 * @return  next position
 */
uint16_t fifo_next_pos(uint16_t pos, uint16_t size);

/**
 * calc the previous position index (pos)
 * @param   pos     current position
 * @param   size    to calc maximum
 * @return  previous position
 */
uint16_t fifo_prev_pos(uint16_t pos, uint16_t size);

#endif // _MM_FIFO_H_
