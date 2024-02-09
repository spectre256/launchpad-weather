/*
 * json.h
 *
 *  Created on: Feb 7, 2024
 *      Author: gibbonec
 */

#ifndef JSON_H_
#define JSON_H_

#include "array.h"
#include "map.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct JSONString {
    char* str;
    size_t length;
} JSONString;

typedef struct JSONValue {
    enum {OBJECT, ARRAY, STRING, NUMBER, BOOLEAN, JSONNULL} type;
    union {
        Map* object;
        Array* array;
        JSONString string;
        bool boolean;
        double number;
    } value;
} JSONValue;

JSONValue parseJSON(char* str);

JSONValue parseObject(void);

JSONValue* parseArray(void);

JSONValue parseValue(void);

JSONValue* parseString(void);

JSONValue parseNumber(void);

JSONValue* parseBool(void);

JSONValue parseNull(void);

void parseWhitespace(void);

#ifdef __cplusplus
}
#endif

#endif /* JSON_H_ */
