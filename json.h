/*
 * json.h
 *
 *  Created on: Feb 7, 2024
 *      Author: gibbonec
 */

#ifndef JSON_H_
#define JSON_H_

#include "map.h"
#include "array.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JSONString {
    const char* str;
    size_t length;
} JSONString;

typedef struct JSONValue {
    enum {OBJECT, ARRAY, STRING, NUMBER, BOOLEAN, JSONNULL} type;
    union {
        Map* object;
        Array* array;
        JSONString* str;
        bool boolean;
        float number;
    } value;
} JSONValue;

JSONValue* parseJSON(char* str);

JSONValue* JSONGet(JSONValue* object, char* key);

void destroyJSON(JSONValue* value);

JSONValue* parseObject(void);

JSONValue* parseArray(void);

JSONValue* parseValue(void);

JSONValue* parseString(void);

JSONValue* parseNumber(void);

JSONValue* parseBool(void);

JSONValue* parseNull(void);

void parseWhitespace(void);

void testParser(void);

#ifdef __cplusplus
}
#endif

#endif /* JSON_H_ */
