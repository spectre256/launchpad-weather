#include "array.h"
#include <stdlib.h>

size_t allocated_array = 0;

#define MALLOC(size) \
    ({ allocated_array += size; \
        malloc(size); })

Array* newArray() {
    Array* array = (Array*)MALLOC(sizeof(Array));
    if (array == NULL) {
        return NULL;
    }

    array->length = 0;
    array->capacity = DEFAULT_CAPACITY;
    array->buffer = MALLOC(DEFAULT_CAPACITY * sizeof(void*));

    return array;
}

void destroyArray(Array* array) {
    void* value;
    arrayForeach(array, value, _i) {
        free(value);
    }
    free(array->buffer);
    free(array);
}

void* arrayGet(Array* array, int i) {
    return array->buffer[i];
}

void arrayAppend(Array* array, void* value) {
    if (array->length + 1 > array->capacity) {
        array->capacity += RESIZE_CAPACITY;
        array->buffer = realloc(array->buffer, array->capacity * sizeof(void*));
    }

    array->buffer[array->length++] = value;
}

void arrayDelete(Array* array, int i) {
    array->length--;
    for (; i < array->length; i++) {
        array->buffer[i] = array->buffer[i + 1];
    }

    if (array->length < array->capacity - RESIZE_CAPACITY) {
        array->capacity -= RESIZE_CAPACITY;
        array->buffer = realloc(array->buffer, array->capacity * sizeof(void*));
    }
}

void testArray(void) {
    Array* array = newArray();
    int len = array->length;

    // Allocate just enough members to cause the buffer to resize
    while (len <= DEFAULT_CAPACITY) {
        int* val = MALLOC(sizeof(int));
        *val = len;
        arrayAppend(array, val);
        len++;
    } // Check capacity/length here to ensure buffer gets resized

    int* first = (int*)arrayGet(array, 0);
    int* last = (int*)arrayGet(array, len - 1);

    arrayDelete(array, len - 2);
    arrayDelete(array, len - 1); // Check capacity/length here to ensure buffer gets resized

    destroyArray(array);
}
