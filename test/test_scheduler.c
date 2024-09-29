/**
 * Martin Egli
 * 2024-09-28
 * scheduler https://github.com/mwuerms/mmschedule
 * testing scheduler functions
 * + compile: gcc test_scheduler.c ../fifo.c ../scheduler.c test.c -o test_scheduler.exe
 * + run: ./test_scheduler.exe
 */
#include <stdio.h>
#include "test.h"
// code under test
#include "../scheduler.h"

char *get_bool_string(uint8_t b) {
    if(b == true)
        return "true";
    return "false";
}

static uint8_t test01_pid, test02_pid;

// run until test_idle_process() returns 0
static int8_t test_run_count = 10;
static int8_t test_idle_process (uint8_t event, void *data) {
    printf("called test_idle_process(%d, %p)\n", event, data);
    printf(" + test_run_count: %d\n", test_run_count);
    scheduler_send_event(test01_pid, 1, NULL);

    if(test_run_count == 5) {
        scheduler_send_event(test01_pid, 1, NULL);
        scheduler_send_event(test02_pid, 3, NULL);
        scheduler_send_event(test02_pid, 3, NULL);
        scheduler_send_event(test02_pid, 3, NULL);
    }
    return test_run_count--;
}
static process_t idle_proc = {.process = test_idle_process, .name = "IDLE"};

static int8_t test01_process (uint8_t event, void *data) {
    printf("called test01_process(%d, %p)\n", event, data);
    if(event == 1) {
        printf(" + event: %d, send event to test02_process\n", event);
        scheduler_send_event(test02_pid, 2, NULL);
    }
    if(event == EV_START) {
        printf(" + START test01_process\n");
    }
    return 1;
}
static process_t test01_proc = {.process = test01_process, .name = "TEST01_PROC"};

static int8_t test02_process (uint8_t event, void *data) {
    printf("called test02_process(%d, %p)\n", event, data);
    if(event == 2) {
        printf(" + event: %d\n", event);
    }
    if(event == 3) {
        printf(" + event: %d, TESTTESTTEST!\n", event);
    }
    if(event == EV_START) {
        printf(" + START test02_process\n");
    }
    return 1;
}
static process_t test02_proc = {.process = test02_process, .name = "TEST02_PROC"};


// - test cases ----------------------------------------------------------------
int8_t test01(void) {
    uint8_t test_nr;
    int8_t res, res_should;

    printf(" + test01: scheduler functions\n   initialise and add processes\n");
    test_nr = 1;
    printf("   %02d: scheduler_init()\n", test_nr);
    scheduler_init();

    test_nr++;
    printf("   %02d: scheduler_add_idle_process(%s)\n", test_nr, idle_proc.name);
    res_should = true;
    res = scheduler_add_idle_process(&idle_proc);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }
    
    test_nr++;
    printf("   %02d: scheduler_add_process(%s)\n", test_nr, test01_proc.name);
    res_should = true;
    res = scheduler_add_process(&test01_proc);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }
    test01_pid = test01_proc.pid;
    printf("       pid:    %d\n", test01_pid);

    test_nr++;
    printf("   %02d: scheduler_add_process(%s)\n", test_nr, test02_proc.name);
    res_should = true;
    res = scheduler_add_process(&test02_proc);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }
    test02_pid = test02_proc.pid;
    printf("       pid:    %d\n", test02_pid);

    return TEST_SUCCESSFUL;
}

int8_t test02(void) {
    uint8_t test_nr;
    int8_t res, res_should;
    printf(" + test02: scheduler start and run processes\n");

    test_nr = 1;
    printf("   %02d: scheduler_start_process(%d)\n", test_nr, test01_pid);
    res_should = true;
    res = scheduler_start_process(test01_pid);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }

    test_nr++;
    printf("   %02d: scheduler_start_process(%d)\n", test_nr, test02_pid);
    res_should = true;
    res = scheduler_start_process(test02_pid);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }

    test_nr++;
    printf("   %02d: scheduler_run()\n", test_nr);
    scheduler_run();
    return TEST_SUCCESSFUL;
}

int main(void) {
    printf("testing scheduler functions\n\n");
    test_eval_result(test01());
    test_eval_result(test02());

    printf("\nall tests successfully done\n");
    return 0;
}