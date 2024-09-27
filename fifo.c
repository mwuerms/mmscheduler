/**
 * Martin Egli
 * 2023-09-02
 * mmlib https://github.com/mwuerms/mmlib
 * fifo functions
 */

#include "fifo.h"

// - private functions ---------------------------------------------------------
uint16_t fifo_inc(uint16_t pos, uint16_t size) {
    pos++;
    if(pos >= size)
        return 0;
    return pos;
}

// - public functions ----------------------------------------------------------
uint16_t fifo_init(fifo_t *f, void *data, uint16_t size) {
    if(f == NULL) {
        // error, invalid pointer
        return false;
    }
    f->data = data;
    f->size = size;
    f->wr = 0;
    f->wr_proc = 0;
    f->rd = size-1;
    f->rd_proc = size-1;
    return true;
}

uint16_t fifo_is_full(fifo_t *f) {
    if(f == NULL) {
        // error, invalid pointer
        return false;
    }
    if(f->wr == f->rd) {
        // is full
        return true;
    }
    return false;
}

uint16_t fifo_is_empty(fifo_t *f) {
    uint16_t rd_next;
    if(f == NULL) {
        // error, invalid pointer
        return false;
    }
    rd_next = fifo_inc(f->rd, f->size);
    if(rd_next == f->wr) {
        // is empty
        return true;
    }
    return false;
}

uint16_t fifo_try_append(fifo_t *f) {
    if(f == NULL) {
        // error, invalid pointer
        return false;
    }
    if(fifo_is_full(f) == true) {
        // error, is full
        return false;
    }
    f->wr_proc = fifo_inc(f->wr, f->size);
    return true;
}

uint16_t fifo_finalize_append(fifo_t *f) {
    if(f == NULL) {
        // error, invalid pointer
        return false;
    }
    f->wr = f->wr_proc;
    return true;
}

uint16_t fifo_try_get(fifo_t *f) {
    if(f == NULL) {
        // error, invalid pointer
        return false;
    }
    if(fifo_is_empty(f) == true) {
        // error, is empty
        return false;
    }
    f->rd_proc = fifo_inc(f->rd, f->size);
    return true;
}

uint16_t fifo_finalize_get(fifo_t *f) {
    if(f == NULL) {
        // error, invalid pointer
        return false;
    }
    f->rd = f->rd_proc;
    return true;
}

