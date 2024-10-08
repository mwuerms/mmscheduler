/**
 * Martin Egli
 * 2024-09-28
 * scheduler https://github.com/mwuerms/mmschedule
 * testing scheduler functions
 * + compile from main folder: gcc scheduler.c events.c power_mode.c fifo.c test/scheduler_test.c test/test.c -o test/scheduler_test
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

static uint8_t test01_tid, test02_tid;
static uint16_t test_run_count;

static int8_t test01_task_func (uint8_t event, void *data) {
    printf("called test01_task_func(%d, %p)\n", event, data);
    printf(" + event: %d\n", event);
    if(event == EV_START) {
        printf(" + START test01_task\n");
    }
    if(test_run_count) {
        test_run_count--;
        scheduler_send_event(test02_tid, 2, NULL);
    }
    else {
        scheduler_send_event(test02_tid, 3, NULL);
    }
    return 1;
}
static task_t test01_task = {.task = test01_task_func, .name = "TEST01_TASK"};

static int8_t test02_task_func (uint8_t event, void *data) {
    printf("called test02_task_func(%d, %p)\n", event, data);
    printf(" + event: %d\n", event);
    if(event == 2) {
        scheduler_send_event(test01_tid, 4, NULL);
    }
    if(event == 3) {
        printf(" + event: %d, TESTTESTTEST!\n", event);
    }
    if((event > 30) && (event < EV_START)) {
        printf(" + event: %d, from event_timer, nothing to do\n", event);
    }
    if(event == EV_START) {
        printf(" + START test02_task\n");
    }
    return 1;
}
static task_t test02_task = {.task = test02_task_func, .name = "TEST02_TASK"};


// - test cases ----------------------------------------------------------------
int8_t test01(void) {
    uint8_t test_nr;
    int8_t res, res_should;

    printf(" + test01: scheduler functions\n   initialise and add taskes\n");
    test_nr = 1;
    printf("   %02d: scheduler_init()\n", test_nr);
    scheduler_init();

    test_nr++;
    printf("   %02d: scheduler_add_task(%s)\n", test_nr, test01_task.name);
    res_should = true;
    res = scheduler_add_task(&test01_task);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }
    test01_tid = test01_task.tid;
    printf("       pid:    %d\n", test01_tid);

    test_nr++;
    printf("   %02d: scheduler_add_task(%s)\n", test_nr, test02_task.name);
    res_should = true;
    res = scheduler_add_task(&test02_task);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }
    test02_tid = test02_task.tid;
    printf("       pid:    %d\n", test02_tid);

    return TEST_SUCCESSFUL;
}

int8_t test02(void) {
    uint8_t test_nr;
    int8_t res, res_should;
    printf(" + test02: scheduler start taskes\n");

    test_nr = 1;
    printf("   %02d: scheduler_start_task(%d)\n", test_nr, test01_tid);
    res_should = true;
    res = scheduler_start_task(test01_tid);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }

    test_nr++;
    printf("   %02d: scheduler_start_task(%d)\n", test_nr, test02_tid);
    res_should = true;
    res = scheduler_start_task(test02_tid);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }
    return TEST_SUCCESSFUL;
}

int8_t test03(void) {
    uint8_t test_nr;
    //int8_t res, res_should;
    printf("\n\n + test03: scheduler run\n");

    test_nr = 1;
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
    res_should = true;//false;
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
    p = 3;
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
    for(test_nr = 1;test_nr < 5;test_nr++) {
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
    tout = 1002;
    printf("   %02d: scheduler_add_timer_event(%d, %d, 0x%02X), should be full\n", test_nr, tout, p, ev);
    res_should = true;
    res = scheduler_add_timer_event(tout, p, ev, NULL);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }
    test_nr++;
    ev++;
    tout = 1004;
    printf("   %02d: scheduler_add_timer_event(%d, %d, 0x%02X), should be full\n", test_nr, tout, p, ev);
    res_should = true;
    res = scheduler_add_timer_event(tout, p, ev, NULL);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }
    test_nr++;
    ev++;
    tout = 1844;
    printf("   %02d: scheduler_add_timer_event(%d, %d, 0x%02X), should be full\n", test_nr, tout, p, ev);
    res_should = true;
    res = scheduler_add_timer_event(tout, p, ev, NULL);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }
    test_nr++;
    ev++;
    tout = 1004;
    printf("   %02d: scheduler_add_timer_event(%d, %d, 0x%02X), should be full\n", test_nr, tout, p, ev);
    res_should = true;
    res = scheduler_add_timer_event(tout, p, ev, NULL);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }
    test_nr++;
    ev++;
    tout = 952;
    printf("   %02d: scheduler_add_timer_event(%d, %d, 0x%02X), should be full\n", test_nr, tout, p, ev);
    res_should = true;
    res = scheduler_add_timer_event(tout, p, ev, NULL);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }
    test_nr++;
    ev = 17;
    p = 2; // send ev 17 to test01 task -> stop
    tout = 1012;
    printf("   %02d: scheduler_add_timer_event(%d, %d, 0x%02X), should be full\n", test_nr, tout, p, ev);
    res_should = true;
    res = scheduler_add_timer_event(tout, p, ev, NULL);
    printf("       should: %s\n", get_bool_string(res_should));
    printf("       result: %s\n", get_bool_string(res));
    if(res != res_should) {
        return TEST_FAILED;
    }
    return TEST_SUCCESSFUL;
}

int8_t test06(void) {
    uint8_t test_nr;
    int8_t res, res_should;
    printf("\n\n + test06: scheduler_start_event_timer\n");

    test_nr = 1;
    printf("   %02d: scheduler_start_event_timer()\n", test_nr);
    res_should = true;
    res = scheduler_start_event_timer();
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
    test_run_count = 10;
    test_eval_result(test03()); // run()
    // stuck here at the moment, does not leave scheduler_run()

    test_eval_result(test04());
    test_run_count = 10;
    test_eval_result(test03()); // run()


    test_eval_result(test05());
    test_eval_result(test06());
    test_run_count = 1200;
    test_eval_result(test03()); // run()

    printf("\n test01_tid: %d, test02_tid: %d\n", test01_tid, test02_tid);
    printf("all tests successfully done\n");
    return 0;
}
