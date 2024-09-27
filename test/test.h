/**
 * Martin Egli
 * 2023-05-03
 * mmlib https://github.com/mwuerms/mmlib
 * test functions
 */

#ifndef _MM_TEST_H_
#define _MM_TEST_H_

#include <stdint.h>
#include <stdlib.h>

// use int8_t
#define TEST_SUCCESSFUL 0
#define TEST_FAILED -1

// prototype of test function
int8_t testXY(void);
void test_eval_result(int8_t res);

#endif // _MM_TEST_H_



