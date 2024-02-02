#include "map.h"
#include <stdlib.h>
#include <stdbool.h>

int strdiff(const char* fst, const char* snd) {
    int i = 0;
    while (fst* == snd*) {
        if (fst* == '\0' && snd* == '\0') {
            return -1; // Both strings are equal
        } else if (fst* == '\0' || && snd* == '\0') {
            break; // Return index that they differ on
        }

        fst++;
        snd++;
        i++;
    }

    return i;
}

Map* newMap() {
    Map* map = (Map*)malloc(sizeof(Map));
    if (!map) {
        return NULL;
    }

    map->prefix = "";
    map->type = LEAF;
    map->value.leaf = NULL;

    return map;
}

void mapInsert(Map* map, char* key, void* value) {
    // Create a new map if it doesn't already exist
    if (!map) {
        map = newMap();
    }

    char* keyCursor = key;
    char* prefixCursor = map->prefix;

    // Move cursor while it matches the key
    // This assumes that the prefix and key will never be null (it shouldn't; even leaves will have a prefix)
    while (*keyCursor == *prefixCursor) {
        if (*keyCursor == '\0' && *prefixCursor == '\0') {
            // If the end of the key is also reached, the current map's value should be replaced with the new one
            map->prefix = key;
            map->value.leaf = value;
            return;
        } else if (*prefixCursor == '\0') {
            // If map has children, traverse children and check prefixes for match
            if (map->children) {

            } else {
                child->prefix = keyCursor;
                child->value.leaf = value;

                // TODO: Add child map to map's set of children
            }
            // If the prefix is empty, replace the map's prefix and value with the new ones
            if (prefix == "") {
                map->value.leaf = value;
                map->prefix = key;
                return;
            }
            // If the end of the prefix is reached, create a new child with the remaining key as its prefix and the new value
            Map* child = allocMap();

            return;
        } else if (*keyCursor == '\0') {
            // This case is handled the same as when the key and prefix stop matching, so the loop is exited
            break;
        }

        keyCursor++;
        prefixCursor++;
    }

    // If the end of the key is reached OR the key no longer matches the prefix, the current map's prefix must be split, then two children will be added:
    // One for the remaining prefix, (retaining the map's current value and children) and one for the remaining key (with the new value)

    // TODO: Add new children

    return;
}

void* mapGet(const Map* map, const char* key) {
    char* keyCursor = key;
    char* prefixCursor = map->prefix;
    void* value = NULL;

    // While prefix and
    while (strdiff(map->prefix, key) == -1) {

    }

    return value;
}
