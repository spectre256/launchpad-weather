/*
 * array.h
 *
 *  Created on: Jan 31, 2024
 *      Author: gibbonec
 */

#ifndef ARRAY_H_
#define ARRAY_H_

#define DEFAULT_CAPACITY 5

typedef struct {
    int length;
    int capacity;
    void** buffer;
} Array;

extern Array newArray();

extern void destroyArray(Array* array);

extern void arrayAppend(Array* array, void* value);

extern void* arrayGet(Array* array, int i);

extern void arrayDelete(Array* array, int i);

#endif /* ARRAY_H_ */
