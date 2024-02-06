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

typedef struct Map {
    char* prefix;
    size_t prefixLen;
    enum {LEAF, TREE} type;
    union {
        Array* tree;
        void* leaf;
    } value;
} Map;

extern Map* newMap();

extern void destroyMap(Map* map);

extern bool mapIsEmpty(Map* map);

extern void mapInsert(Map* map, char* key, size_t keyLen, void* value);

extern void* mapGet(const Map* map, const char* key, size_t keyLen);

#ifdef __cplusplus
}
#endif

#endif /* MAP_H_ */
