#include "json.h"
#include "map.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

char* cursor;

char next() {
    return *(++cursor);
}

char peek() {
    return *(cursor + 1);
}

// Helper function to parse NUL-terminated string
bool parseLiteral(char* str) {
    while (*str != '\0') {
        if (*(str++) != next()) {
            return false;
        }
    }

    return true;
}

JSONValue* parseJSON(char* str) {
    if (!str) return NULL;
    cursor = str;
    return parseObject();
}

JSONValue* JSONGet(JSONValue* object, char* key) {
    if (!object || object->type != OBJECT) return NULL;

    return mapGet(object->value.object, key, strlen(key));
}

void destroyJSON(JSONValue* value) {
    if (!value) return;
    switch (value->type) {
    case OBJECT:
        destroyMap(value->value.object);
        break;
    case ARRAY:
        destroyArray(value->value.array);
    }

    free(value);
}

// Ellis
JSONValue* parseObject(void) {
    JSONValue* object = malloc(sizeof(JSONValue));
    object->type = OBJECT;
    object->value.object = newMap();

    if (next() != '{') return NULL;
    parseWhitespace();

    if (peek() != '}') {
        while (true) {
            // Parse string then whitespace
            JSONValue* string = parseString();
            if (string == NULL) return NULL;
            parseWhitespace();

            // Parse colon
            if (next() != ':') return NULL;

            // Parse value
            JSONValue* value = parseValue();
            if (value == NULL) return NULL;

            // Add key-value pair to map
            JSONString key = string->value.string;
            mapInsert(object->value.object, key.str, key.length, (void*)value);

            // Parse comma or break, then whitespace
            if (peek() != ',') break;
            next();
            parseWhitespace();
        }
    }

    return next() == '}' ? object : NULL;
}

// Connor
JSONValue* parseArray(void) {
    return NULL;
}

// Ellis
JSONValue* parseValue(void) {
    parseWhitespace();

    JSONValue* value = NULL;
    char c = peek();
    switch(c) {
    case '"':
        value = parseString();
        break;
    case '{':
        value = parseObject();
        break;
    case '[':
        value = parseArray();
        break;
    case 't':
    case 'f':
        value = parseBool();
        break;
    case 'n':
        value = parseNull();
        break;
    default:
        if ((c >= '0' && c <= '9') || c == '-') {
            value = parseNumber();
        }
    }

    parseWhitespace();
    return value;
}

// Connor
JSONValue* parseString(void) {
    return NULL;
}

// Ellis
JSONValue* parseNumber(void) {
    int multiplier = 1;

    // Optional negative sign
    if (peek() == '-') {
        multiplier = -1;
        next();
    }

    // Parse integer part
    int integerPart = 0;
    char c = peek();
    if (c == '0') {
        // Skip parsing if the integer part is a zero
        next();
    } else {
        // Parse first non-zero digit
        if (c >= '1' && c <= '9') {
            integerPart = c - '0';
        } else {
            return NULL;
        }

        // Parse many digits
        for (c = peek(); c >= '0' && c <= '9'; next()) {
            integerPart = integerPart * 10 + c - '0';
        }
    }

    // Parse optional fractional part
    double fractionalPart = 0;
    if (peek() == '.') {
        next();

        // Parse many digits
        int divisor = 10;
        for (c = peek(); c >= '0' && c <= '9'; next()) {
            fractionalPart += c / divisor;
            divisor *= 10;
        }
    }

    // Parse optional exponential part
    if (peek() == 'e' || peek() == 'E') {
        next();

        // Parse exponent sign
        char sign = next();
        if (sign != '+' && sign != '-') return NULL;

        // Parse many digits
        int exponent = 0;
        for (c = peek(); c >= '0' && c <= '9'; next()) {
            exponent = exponent * 10 + c - '0';
        }
        multiplier *= pow(10, (sign == '+' ? 1 : -1) * exponent);
    }

    JSONValue* number = malloc(sizeof(JSONValue));
    number->type = NUMBER;
    number->value.number = multiplier * (integerPart + fractionalPart);
    return number;
}

// Connor
JSONValue* parseBool(void) {
    return NULL;
}

// Ellis
JSONValue* parseNull(void) {
    if (parseLiteral("null")) {
        JSONValue* value = malloc(sizeof(JSONValue));
        value->type = JSONNULL;
        return value;
    } else {
        return NULL;
    }
}

// Connor
void parseWhitespace(void) {
}
