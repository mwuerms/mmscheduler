/*
 * Martin Egli
 * 2024-09-29
 */

#ifndef _DEBUG_PRINTF_
#define _DEBUG_PRINTF_

#ifdef DEBUG_PRINTF_ON
#include <stdio.h>

#define DEBUG_PRINTF_MESSAGE(...)  printf(__VA_ARGS__)
#else
#define DEBUG_PRINTF_MESSAGE(...)
#endif

#endif // _DEBUG_PRINTF_

