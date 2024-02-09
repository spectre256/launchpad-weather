#include "map.h"
#include <string.h>

#define MIN(a, b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

#define ABS(a) \
   ({ __typeof__ (a) _a = (a); \
     _a < 0 ? -_a : _a; })

// Returns the index at which two strings differ, or -1 if they're equal
int strdiff(const char* fst, size_t lenFst, const char* snd, size_t lenSnd) {
    int i;
    for (i = 0; i < MIN(lenFst, lenSnd); i++) {
        if (fst[i] != snd[i]) {
            return i;
        }
    }

    if (lenFst != lenSnd) {
        return MIN(lenFst, lenSnd);
    }

    return -1;
}

Map* newMap() {
    Map* map = malloc(sizeof(Map));
    if (!map) {
        return NULL;
    }

    map->prefix = "";
    map->prefixLen = 0;
    map->type = LEAF;
    map->value.leaf = NULL;

    return map;
}

void destroyMap(Map* map) {
    switch (map->type) {
    case LEAF:
        free(map->value.leaf);
        break;
    case TREE: {
        Map* child;
        arrayForeach(map->value.tree, child, _i) {
            destroyMap(child);
        }
    }
    }

    free(map);
}

bool mapIsEmpty(Map* map) {
    return map->prefixLen == 0 && map->value.leaf == NULL;
}

// Inserts a new key-value pair into the map
void mapInsert(Map* map, char* key, size_t keyLen, void* value) {
    // Create a new map if it doesn't already exist
    if (!map) {
        map = newMap();
    }

    int i = strdiff(key, keyLen, map->prefix, map->prefixLen);

    switch (map->type) {
    case LEAF:
        if (i == -1) {
            // If the key and prefix are equal, override the new value
            map->value.leaf = value;
        } else {
            // Split the key and prefix at the given index
            // NOTE: does not copy strings for efficiency, so they must be freed by the caller

            // Child with new value
            Map* childNew = newMap();
            childNew->prefix = &key[i];
            childNew->prefixLen = keyLen - i;
            childNew->value.leaf = value;

            // Child with old value
            Map* childOld = newMap();
            childOld->prefix = &map->prefix[i];
            childOld->prefixLen = map->prefixLen - i;
            childOld->value.leaf = map->value.leaf;

            // If the map is empty, replace it with a new leaf node
            if (mapIsEmpty(map)) {
                *map = *childNew;
                return;
            }

            // Set the map to a tree with the two children
            map->prefixLen = i;
            map->type = TREE;
            map->value.tree = newArray();

            arrayAppend(map->value.tree, childNew);
            arrayAppend(map->value.tree, childOld);
        }
        break;
    case TREE:
        if (i == map->prefixLen) {
            // If the differing index is at the end of the prefix, check children for matching prefix against the remainder of the key
            // This also handles the case of the key and prefix having the same length
            // Since the children's prefixes are guaranteed to not share any characters, we can get away with only checking the first character for equality
            if (keyLen != 0) {
                Map* child;
                arrayForeach(map->value.tree, child, _i) {
                    if (key[i] == child->prefix[0]) {
                        // Once the matching prefix is found, call insert on the child
                        mapInsert(child, &key[i], keyLen - i, value);
                        return;
                    }
                }
            }

            // If a matching child was not found, insert a new child with the remainder of the key
            Map* childNew = newMap();
            childNew->prefix = &key[i];
            childNew->prefixLen = keyLen - i;
            childNew->value.leaf = value;

            arrayAppend(map->value.tree, childNew);
            return;
        } else {
            // Otherwise, split the prefix like before, retaining the current map's children
            Map* childNew = newMap();
            childNew->prefix = &key[i];
            childNew->prefixLen = keyLen - i;
            childNew->value.leaf = value;

            // Child with old value, retaining children
            Map* childOld = newMap();
            childOld->prefix = &map->prefix[i];
            childOld->prefixLen = map->prefixLen - i;
            childOld->value.tree = map->value.tree;

            // Set the map to a tree with the two children
            map->prefixLen = i;
            map->value.tree = newArray();

            arrayAppend(map->value.tree, childNew);
            arrayAppend(map->value.tree, childOld);
        }
    }
}

void* mapGet(const Map* map, const char* key, size_t keyLen) {
    if (map->type == LEAF) {
        return map->value.leaf;
    }

    int i = strdiff(map->prefix, map->prefixLen, key, keyLen);

    // For each child of the current node
    Map* child;
    arrayForeach(map->value.tree, child, _i) {
        if (keyLen == 0) {
            if (child->prefixLen != 0) {
                break;
            }
        } else if (child->prefix[0] != key[i]) {
            continue;
        }

        return mapGet(child, &key[i], keyLen - i);
    }

    return NULL;
}
