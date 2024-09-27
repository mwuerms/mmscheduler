/**
 * Martin Egli
 * 2024-09-28
 * mmscheduler https://github.com/mwuerms/mmschedule
 * testing mmscheduler functions
 * + compile: gcc test_mmscheduler.c ../fifo.c ../mmscheduler.c test.c -o test_mmscheduler.exe
 * + run: ./test_mmscheduler.exe
 */
#include <stdio.h>
#include "test.h"
// code under test
#include "../mmscheduler.h"

char *get_bool_string(uint8_t b) {
    if(b == true)
        return "true";
    return "false";
}

// run until test_idle_process() returns 0
static int8_t test_run_count = 10;
static int8_t test_idle_process (uint8_t event, void *data) {
    printf("called test_idle_process(%d, %p)\n", event, data);
    printf(" + test_run_count: %d\n", test_run_count);
    return test_run_count--;
}
static process_t idle_proc = {.process = test_idle_process, .name = "IDLE"};

// - test cases ----------------------------------------------------------------
int8_t test01(void) {
    printf(" + test01: mmscheduler functions");
    mmscheduler_init();

    mmscheduler_add_idle_process(&idle_proc);
    mmscheduler_run();
    return TEST_SUCCESSFUL;
}

int main(void) {
    printf("testing mmscheduler functions\n\n");
    test_eval_result(test01());

    printf("\nall tests successfully done\n");
    return 0;
}