/*
 * map.h
 *
 *  Created on: Jan 31, 2024
 *      Author: gibbonec
 */

#ifndef MAP_H_
#define MAP_H_

#include "array.h"

typedef struct Map {
    char* prefix;
    enum {LEAF, TREE} type;
    union {
        Array* tree;
        void* leaf;
    } value;
    Array* children;
} Map;

extern Map* newMap();

extern void destroyMap(Map* map);

extern void mapInsert(Map* map, char* key, void* value);

extern void* mapGet(const Map* map, const char* key);

void* mapGetHelper(const Map* map, const char* key, int curPrefixLength);

#endif /* MAP_H_ */
