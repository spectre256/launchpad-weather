#include "json.h"
#include "map.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

char next() {
    return *(cursor++);
}

char peek() {
    return *cursor;
}

// Helper function to parse NUL-terminated string
bool parseLiteral(const char* str) {
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
    Map* map = newMap();

    // Parse opening bracket and whitespace
    if (next() != '{') return NULL;
    parseWhitespace();

    if (peek() == '}') {
        // If there's a closing bracket, consume it
        next();
    } else {
        // Otherwise, parse key value pairs
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
            JSONString key = string->value.str;
            mapInsert(map, key.str, key.length, (void*)value);

            // Parse comma or break, then whitespace
            if (peek() != ',') break;
            next();
            parseWhitespace();
        }

        if (next() != '}') return NULL;
    }

    JSONValue* object = malloc(sizeof(JSONValue));
    object->type = OBJECT;
    object->value.object = map;
    return object;
}

// Connor
JSONValue* parseArray(void) {
    Array* modifiableArray = newArray();

    // Parse opening bracket and whitespace
    if (next() != '[') return NULL;
    parseWhitespace();

    if (peek() == ']') {
        // If there's a closing bracket, consume it
        next();
    } else {
        // Otherwise, parse values
        while (true) {
            JSONValue* value = parseValue();
            if (!value) return NULL;
            arrayAppend(modifiableArray, value);

            // Parse comma or break, then whitespace
            if (peek() != ',') break;
            next();
        }

        if (next() != ']') return NULL;
    }

    JSONValue* newJSONArray = malloc(sizeof(JSONValue));
    newJSONArray->value.array = modifiableArray;
    newJSONArray->type = ARRAY;
    return newJSONArray;
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
        if (isdigit(c) || c == '-') {
            value = parseNumber();
        }
    }

    parseWhitespace();
    return value;
}

// Connor
JSONValue* parseString(void) {
    // Parse initial quote
    if (next() != '"') return NULL;

    JSONValue* newJSONString = malloc(sizeof(JSONValue));
    newJSONString->type = STRING;
    newJSONString->value.str.str = cursor;

    int len = 0;
    char c;
    while ((c = next()) != '"') {
        // Disallow control characters
        if (iscntrl(c)) return NULL;

        if (c == '\\') {
            len++;
            c = next();
            switch (c) {
            case 'u': {
                // Parse 4 hex digits
                int i;
                for (i = 0; i < 4; i++) {
                    if (!isxdigit(next())) return NULL;
                    len++;
                }
                break;
            }
            case '"':
            case '\\':
            case '/':
            case 'b':
            case 'f':
            case 'n':
            case 'r':
            case 't':
                break;
            default:
                return NULL;
            }
        }

        len++;
    }

    newJSONString->value.str.length = len;
    return newJSONString;
}

// Ellis
JSONValue* parseNumber(void) {
    double multiplier = 1;

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
        // In this case, the fractional part is not optional
        if (next() != '.') return NULL;
    } else {
        // Parse first non-zero digit
        if (c >= '1' && c <= '9') {
            integerPart = c - '0';
            next();
        } else {
            return NULL;
        }

        // Parse 0 or more digits
        for (; isdigit(c = peek()); next()) {
            integerPart = integerPart * 10 + c - '0';
        }
    }

    // Parse optional fractional part
    double fractionalPart = 0;
    if (peek() == '.') {
        next();

        // Parse at least 1 digit
        if (!isdigit(peek())) return NULL;

        int divisor = 10;
        for (; isdigit(c = peek()); next()) {
            fractionalPart += (double)(c - '0') / divisor;
            divisor *= 10;
        }
    }

    // Parse optional exponential part
    if (peek() == 'e' || peek() == 'E') {
        next();

        // Parse exponent sign
        char sign = next();
        if (sign != '+' && sign != '-') return NULL;

        // Parse at least 1 digit
        if (!isdigit(peek())) return NULL;

        int exponent = 0;
        for (; isdigit(c = peek()); next()) {
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
    JSONValue* newJSONBool = malloc(sizeof(JSONValue));
    newJSONBool->type = BOOLEAN;
    int i;
    const char t[] = "true";
    const char f[] = "false";
    if (peek() == 't') {
        for (i = 0; i < sizeof(t)/sizeof(char) - 1; i++) {
            if(peek() == t[i]){
                next();
            } else {
                return NULL;
            }
        }
        newJSONBool->value.boolean = true;
    } else if (peek() == 'f') {
        for (i = 0; i < sizeof(f)/sizeof(char) - 1; i++) {
            if (peek() == f[i]) {
                next();
            } else {
                return NULL;
            }
        }
        newJSONBool->value.boolean = false;
    } else{
        return NULL;
    }
    return newJSONBool;
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
    while (peek() == ' ' || peek() == '\n' || peek() == '\r' || peek() == '\t') {
        next();
    }
}
