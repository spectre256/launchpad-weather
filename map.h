/*
 * map.h
 *
 *  Created on: Jan 31, 2024
 *      Author: gibbonec
 */

#ifndef MAP_H_
#define MAP_H_

#include "array.h"
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char* prefix;
    size_t prefixLen;
    enum {LEAF, TREE} type;
    union {
        Array* tree;
        void* leaf;
    } value;
} Map;

typedef enum {
    MAP_ALLOC_ERR = _ArrayErrN,
    _MapErrN,
} MapErr;

Map* newMap(void);

void destroyMap(Map* map, void (*freeValue)(void*));

bool mapIsEmpty(Map* map);

MapErr mapInsert(Map* map, const char* key, size_t keyLen, void* value);

void* mapGet(const Map* map, const char* key, size_t keyLen);

void testMap(void);

#ifdef __cplusplus
}
#endif

#endif /* MAP_H_ */
