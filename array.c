#include "array.h"
#include <stdlib.h>

size_t allocated_array = 0;

#define MALLOC(size) \
    ({  allocated_array += size; \
        malloc(size); })

#define FREE(ptr) \
    ({  allocated_array -= sizeof(*(__typeof__(ptr)){NULL}); \
        free(ptr); })

Array* newArray() {
    Array* array = (Array*)MALLOC(sizeof(Array));
    if (array == NULL) {
        return NULL;
    }

    array->length = 0;
    array->capacity = DEFAULT_CAPACITY;
    array->buffer = MALLOC(DEFAULT_CAPACITY * sizeof(void*));
    if (!array->buffer) {
        FREE(array);
        return NULL;
    }

    return array;
}

void destroyArray(Array* array, void (*freeValue)(void*)) {
    void* value;
    arrayForeach(array, value, _i) {
        freeValue(value);
        allocated_array -= sizeof(void*);
    }

    FREE(array->buffer);
    FREE(array);
}

void* arrayGet(Array* array, int i) {
    if (i < 0 || i >= array->length) return NULL;
    return array->buffer[i];
}

ArrayErr arrayAppend(Array* array, void* value) {
    if (array->length + 1 > array->capacity) {
        void** ptr = realloc(array->buffer, (array->capacity + RESIZE_CAPACITY) * sizeof(void*));
        if (!ptr) return ARRAY_REALLOC_ERR;

        allocated_array += RESIZE_CAPACITY * sizeof(void*);
        array->capacity += RESIZE_CAPACITY;
        array->buffer = ptr;
    }

    array->buffer[array->length++] = value;
    return SUCCESS;
}

ArrayErr arrayDelete(Array* array, int i) {
    array->length--;
    for (; i < array->length; i++) {
        array->buffer[i] = array->buffer[i + 1];
    }

    if (array->length < array->capacity - RESIZE_CAPACITY) {
        void** ptr = realloc(array->buffer, (array->capacity - RESIZE_CAPACITY) * sizeof(void*));
        if (!ptr) return ARRAY_REALLOC_ERR;

        allocated_array -= RESIZE_CAPACITY * sizeof(void*);
        array->capacity -= RESIZE_CAPACITY;
        array->buffer = ptr;
    }

    return SUCCESS;
}

void testArray(void) {
    Array* array = newArray();
    int len = array->length;

    // Allocate enough members to cause the buffer to resize
    while (len <= 3 * DEFAULT_CAPACITY) {
        int* val = malloc(sizeof(int));
        *val = len;
        arrayAppend(array, val);
        len++;
    } // Check capacity/length here to ensure buffer gets resized

    int* first = (int*)arrayGet(array, 0);
    int* last = (int*)arrayGet(array, len - 1);

    arrayDelete(array, len - 2);
    arrayDelete(array, len - 1); // Check capacity/length here to ensure buffer gets resized

    destroyArray(array, free);
}
