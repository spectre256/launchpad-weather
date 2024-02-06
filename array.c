#include "array.h"
#include <stdlib.h>

Array* newArray() {
    Array* array = (Array*)malloc(sizeof(Array));
    if (array == NULL) {
        return NULL;
    }

    array->length = 0;
    array->capacity = DEFAULT_CAPACITY;
    array->buffer = (void**)malloc(DEFAULT_CAPACITY * sizeof(void*));

    return array;
}

void destroyArray(Array* array) {
    free(array->buffer);
    array->buffer = NULL;
    free(array);
}

void* arrayGet(Array* array, int i) {
    return array->buffer[i];
}

void arrayAppend(Array* array, void* value) {
    if (array->length + 1 > array->capacity) {
        array->capacity += DEFAULT_CAPACITY;
        array->buffer = (void**)realloc(array->buffer, array->capacity * sizeof(void*));
    }

    array->buffer[array->length++] = value;
}

void arrayDelete(Array* array, int i) {
    array->length--;
    for (; i < array->length; i++) {
        array->buffer[i] = array->buffer[i + 1];
    }

    if (array->length < array->capacity - DEFAULT_CAPACITY) {
        array->capacity -= DEFAULT_CAPACITY;
        array->buffer = (void**)realloc(array->buffer, array->capacity * sizeof(void*));
    }
}
