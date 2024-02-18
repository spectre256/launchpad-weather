/*
 * array.h
 *
 *  Created on: Jan 31, 2024
 *      Author: gibbonec
 */

#ifndef ARRAY_H_
#define ARRAY_H_

#define DEFAULT_CAPACITY 1
#define RESIZE_CAPACITY 1

// Generates the for loop declaration part for an Array
// Takes an array, a declared variable to hold the current child, and a name for the loop index
#define arrayForeach(array, child, i) \
    int i; \
    for (i = 0; i < (array)->length && (child = (__typeof__(child))(array)->buffer[i]); i++)

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

extern size_t allocated_array;

typedef struct {
    int length;
    int capacity;
    void** buffer;
} Array;

typedef enum {
    SUCCESS,
    ARRAY_ALLOC_ERR,
    ARRAY_REALLOC_ERR,
    _ArrayErrN,
} ArrayErr;

Array* newArray(void);

void destroyArray(Array* array, void (*freeValue)(void*));

ArrayErr arrayAppend(Array* array, void* value);

void* arrayGet(Array* array, int i);

ArrayErr arrayDelete(Array* array, int i, void (*freeValue)(void*));

void testArray(void);

#ifdef __cplusplus
}
#endif

#endif /* ARRAY_H_ */
