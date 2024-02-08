#include "json.h"

char* cursor;

char next() {
    return *(++cursor);
}

char peek() {
    return *(cursor + 1);
}

JSONValue parseJSON(char* str) {

}

// Ellis
JSONValue parseObject(void) {

}

// Connor
JSONValue parseArray(void) {

}


// Ellis
JSONValue parseValue(void) {

}

// Connor
JSONValue parseString(void) {

}

// Ellis
JSONValue parseNumber(void) {

}

// Connor
JSONValue parseBool(void) {

}

// Ellis
JSONValue parseNull(void) {

}

// Connor
void parseWhitespace(void) {

}
