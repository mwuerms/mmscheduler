/**
 * Martin Egli
 * 2023-05-03
 * mmlib https://github.com/mwuerms/mmlib
 * test functions
 */
#include <stdio.h>
#include "test.h"

int8_t testXY(void) {
    printf(" + testXY() allways OK\n");
    return(TEST_SUCCESSFUL);
}

void test_eval_result(int8_t res) {
    if(res == TEST_SUCCESSFUL) {
        printf("SUCCESSFUL\n\n");
        // continue
    }
    else {
        printf("FAILED, stop here\n");
        exit(1);
    }
}
