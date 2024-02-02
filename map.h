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
} Map;

extern Map* newMap();

extern void destroyMap(Map* map);

extern void mapInsert(Map* map, char* key, void* value);

extern void* mapGet(Map* map, char* key);

#endif /* MAP_H_ */
