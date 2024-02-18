#include "map.h"
#include <string.h>

size_t allocated_map = 0;

#define MALLOC(size) \
    ({  allocated_map += size; \
        malloc(size); })

#define FREE(ptr) \
    ({  allocated_map -= sizeof(*(__typeof__(ptr)){NULL}); \
        free(ptr); })

#define MIN(a, b) (a < b ? a : b)

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
    Map* map = MALLOC(sizeof(Map));
    if (!map) {
        return NULL;
    }

    map->prefix = "";
    map->prefixLen = 0;
    map->type = LEAF;
    map->value.leaf = NULL;

    return map;
}

inline bool mapIsEmpty(Map* map) {
    return map->prefixLen == 0 && map->value.leaf == NULL;
}

void destroyMap(Map* map, void (*freeValue)(void*)) {
    if (!map) return;

    switch (map->type) {
    case LEAF:
        freeValue(map->value.leaf);
        break;
    case TREE: {
        void* value;
        arrayForeach(map->value.tree, value, _i) {
            destroyMap(value, freeValue);
            allocated_array -= sizeof(void*);
        }

        allocated_array -= sizeof(map->value.tree->buffer);
        free(map->value.tree->buffer);
        allocated_array -= sizeof(map->value.tree);
        free(map->value.tree);
    }
    }

    FREE(map);
}

// Inserts a new key-value pair into the map
MapErr mapInsert(Map* map, const char* key, size_t keyLen, void* value) {
    if (!map) return MAP_ALLOC_ERR;

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
            if (!childNew) return MAP_ALLOC_ERR;
            childNew->prefix = &key[i];
            childNew->prefixLen = keyLen - i;
            childNew->value.leaf = value;

            // If the map is empty, replace it with a new leaf node
            if (mapIsEmpty(map)) {
                *map = *childNew;
                FREE(childNew);
                return SUCCESS;
            }

            // Child with old value
            Map* childOld = newMap();
            if (!childOld) return MAP_ALLOC_ERR;
            childOld->prefix = &map->prefix[i];
            childOld->prefixLen = map->prefixLen - i;
            childOld->value.leaf = map->value.leaf;

            // Set the map to a tree with the two children
            map->prefixLen = i;
            map->type = TREE;
            map->value.tree = newArray();
            if (!map->value.tree) return ARRAY_ALLOC_ERR;

            ArrayErr err = arrayAppend(map->value.tree, childNew);
            if (err) return ARRAY_ALLOC_ERR;
            err = arrayAppend(map->value.tree, childOld);
            if (err) return ARRAY_ALLOC_ERR;
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
                        return mapInsert(child, &key[i], keyLen - i, value);
                    }
                }
            }

            // If a matching child was not found, insert a new child with the remainder of the key
            Map* childNew = newMap();
            if (!childNew) return MAP_ALLOC_ERR;
            childNew->prefix = &key[i];
            childNew->prefixLen = keyLen - i;
            childNew->value.leaf = value;

            ArrayErr err = arrayAppend(map->value.tree, childNew);
            if (err) return ARRAY_ALLOC_ERR;
        } else {
            // Otherwise, split the prefix like before, retaining the current map's children
            Map* childNew = newMap();
            if (!childNew) return MAP_ALLOC_ERR;
            childNew->prefix = &key[i];
            childNew->prefixLen = keyLen - i;
            childNew->value.leaf = value;

            // Child with old value, retaining children
            Map* childOld = newMap();
            if (!childOld) return MAP_ALLOC_ERR;
            childOld->prefix = &map->prefix[i];
            childOld->prefixLen = map->prefixLen - i;
            childOld->type = TREE;
            childOld->value.tree = map->value.tree;

            // Set the map to a tree with the two children
            map->prefixLen = i;
            map->value.tree = newArray();
            if (!map->value.tree) return ARRAY_ALLOC_ERR;

            ArrayErr err = arrayAppend(map->value.tree, childNew);
            if (err) return ARRAY_ALLOC_ERR;
            err = arrayAppend(map->value.tree, childOld);
            if (err) return ARRAY_ALLOC_ERR;
        }
    }

    return SUCCESS;
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

void testMap(void) {
    Map* map = newMap();
    int* a = malloc(sizeof(int));
    *a = 1;
    const char* key1 = "romane";
    MapErr err = mapInsert(map, key1, 6, a);
    int* a_new = (int*)mapGet(map, key1, 6);

    int* b = malloc(sizeof(int));
    *b = 2;
    err = mapInsert(map, "romanus", 7, b);
    int* b_new = (int*)mapGet(map, "romanus", 7);

    int* c = malloc(sizeof(int));
    *c = 3;
    err = mapInsert(map, "romulus", 7, c);
    int* c_new = (int*)mapGet(map, "romulus", 7);

    int* d = malloc(sizeof(int));
    *d = 4;
    err = mapInsert(map, "rubens", 6, d);
    int* d_new = (int*)mapGet(map, "rubens", 6);

    int* e = malloc(sizeof(int));
    *e = 5;
    err = mapInsert(map, "ruber", 5, e);
    int* e_new = (int*)mapGet(map, "ruber", 5);

    int* f = malloc(sizeof(int));
    *f = 6;
    err = mapInsert(map, "rubicon", 7, f);
    int* f_new = (int*)mapGet(map, "rubicon", 7);

    int* g = malloc(sizeof(int));
    *g = 7;
    err = mapInsert(map, "rubicundus", 10, g);
    int* g_new = (int*)mapGet(map, "rubicundus", 10);

    int* h = malloc(sizeof(int));
    *h = 8;
    err = mapInsert(map, "roman", 5, h);
    int* h_new = (int*)mapGet(map, "roman", 5);

    destroyMap(map, free);
}
