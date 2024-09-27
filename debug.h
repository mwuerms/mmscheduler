/*
 * Martin Wuerms
 * 2015-07-23
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#ifdef DEBUG
#include <stdio.h>

#define DEBUG_MESSAGE(...)  printf(__VA_ARGS__)
#else
#define DEBUG_MESSAGE(...)
#endif

#endif /* DEBUG_H_ */

