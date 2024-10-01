/**
 * Martin Egli
 * 2024-09-28
 * scheduler https://github.com/mwuerms/mmschedule
 * testing scheduler functions
 * + compile from main folder: gcc scheduler.c events.c fifo.c test/scheduler_test.c test/test.c -o test/scheduler_test
 * + run from main folder: ./test/scheduler_test
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
    printf(" + test02: scheduler start processes\n");

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
    return TEST_SUCCESSFUL;
}

int8_t test03(void) {
    uint8_t test_nr;
    int8_t res, res_should;
    printf("\n\n + test03: scheduler run\n");

    test_nr = 1;
    test_run_count = 10;
    printf("   %02d: scheduler_run()\n", test_nr);
    printf("    + test_run_count: %d\n", test_run_count);
    scheduler_run();
    return TEST_SUCCESSFUL;
}

int8_t test04(void) {
    uint8_t test_nr, p, ev;
    int8_t res, res_should;
    printf(" + test04: scheduler_send_event()\n");

    p = 1;
    ev = 10;
    for(test_nr = 1;test_nr < 8-1;test_nr++) {
        ev++;
        printf("   %02d: scheduler_send_event(%d, 0x%02X)\n", test_nr, p, ev);
        res_should = true;
        res = scheduler_send_event(p, ev, NULL);
        printf("       should: %s\n", get_bool_string(res_should));
        printf("       result: %s\n", get_bool_string(res));
        if(res != res_should) {
            return TEST_FAILED;
        }
    }
    // from for test_nr++;
    ev++;
    printf("   %02d: scheduler_send_event(%d, 0x%02X), should be full now\n", test_nr, p, ev);
    res_should = false;
    res = scheduler_send_event(p, ev, NULL);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }

    return TEST_SUCCESSFUL;
}

int8_t test05(void) {
    uint16_t tout;
    uint8_t test_nr, p, ev;
    int8_t res, res_should;
    printf("\n\n + test05: scheduler_send_event()\n");

    test_nr = 0;
    tout = 0;
    p = 1;
    ev = 30;
    printf("   %02d: scheduler_add_timer_event(%d, %d, 0x%02X), should fail, (timeout == %d)\n", test_nr, tout, p, ev, tout);
    res_should = false;
    res = scheduler_add_timer_event(tout, p, ev, NULL);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }
    tout = 1000;
    for(test_nr = 1;test_nr < 8-1;test_nr++) {
        ev++;
        tout++;
        printf("   %02d: scheduler_add_timer_event(%d, %d, 0x%02X)\n", test_nr, tout, p, ev);
        res_should = true;
        res = scheduler_add_timer_event(tout, p, ev, NULL);
        printf("       should: %s\n", get_bool_string(res_should));
        printf("       result: %s\n", get_bool_string(res));
        if(res != res_should) {
            return TEST_FAILED;
        }
    }
    // from for test_nr++;
    ev++;
    tout++;
    printf("   %02d: scheduler_add_timer_event(%d, %d, 0x%02X), should be full\n", test_nr, tout, p, ev);
    res_should = false;
    res = scheduler_add_timer_event(tout, p, ev, NULL);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }

    return TEST_SUCCESSFUL;
}

int main(void) {
    printf("testing scheduler functions\n\n");
    test_eval_result(test01());
    test_eval_result(test02());
    test_eval_result(test03()); // run()
    test_eval_result(test04());
    test_eval_result(test03()); // run()
    test_eval_result(test05());

    printf("\nall tests successfully done\n");
    return 0;
}