#include "json.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

const char* const exampleResponse = "{\"location\":{\"name\":\"Terre Haute\",\"region\":\"Indiana\",\"country\":\"USA\",\"lat\":39.47,\"lon\":-87.35,\"tz_id\":\"America/Indiana/Indianapolis\",\"localtime_epoch\":1707771348,\"localtime\":\"2024-02-12 15:55\"},\"current\":{\"temp_f\":44.4,\"condition\":{\"text\":\"Partly cloudy\"},\"humidity\":58}}";
// const char* const exampleResponse = "{\"location\":{\"name\":\"Terre Haute\",\"region\":\"Indiana\",\"country\":\"USA\",\"lat\":39.47,\"lon\":-87.35,\"tz_id\":\"America/Indiana/Indianapolis\",\"localtime_epoch\":1707420270,\"localtime\":\"2024-02-08 14:24\"},\"current\":{\"last_updated_epoch\":1707419700,\"last_updated\":\"2024-02-08 14:15\",\"temp_c\":14.0,\"temp_f\":57.2,\"is_day\":1,\"condition\":{\"text\":\"Overcast\",\"icon\":\"//cdn.weatherapi.com/weather/64x64/day/122.png\",\"code\":1009},\"wind_mph\":23.0,\"wind_kph\":37.1,\"wind_degree\":180,\"wind_dir\":\"S\",\"pressure_mb\":1012.0,\"pressure_in\":29.87,\"precip_mm\":0.0,\"precip_in\":0.0,\"humidity\":70,\"cloud\":100,\"feelslike_c\":11.6,\"feelslike_f\":52.9,\"vis_km\":16.0,\"vis_miles\":9.0,\"uv\":4.0,\"gust_mph\":26.4,\"gust_kph\":42.4}}";

const char* cursor;

size_t allocated_json = 0;

#define MALLOC(size) \
    ({ allocated_json += size; \
        malloc(size); })

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
        break;
    case STRING:
        free(value->value.str);
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
            JSONValue* str = parseString();
            if (str == NULL) return NULL;
            parseWhitespace();

            // Parse colon
            if (next() != ':') return NULL;

            // Parse value
            JSONValue* value = parseValue();
            if (value == NULL) return NULL;

            // Add key-value pair to map
            JSONString* key = str->value.str;
            mapInsert(map, key->str, key->length, (void*)value);
            destroyJSON(str);
            allocated_json -= sizeof(JSONString);

            // Parse comma or break, then whitespace
            if (peek() != ',') break;
            next();
            parseWhitespace();
        }

        if (next() != '}') return NULL;
    }

    JSONValue* object = MALLOC(sizeof(JSONValue));
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

    JSONValue* newJSONArray = MALLOC(sizeof(JSONValue));
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

    JSONValue* newJSONString = MALLOC(sizeof(JSONValue));
    newJSONString->type = STRING;
    newJSONString->value.str = MALLOC(sizeof(JSONString));
    newJSONString->value.str->str = cursor;

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

    newJSONString->value.str->length = len;
    return newJSONString;
}

// Ellis
JSONValue* parseNumber(void) {
    float multiplier = 1;

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
    float fractionalPart = 0;
    if (peek() == '.') {
        next();

        // Parse at least 1 digit
        if (!isdigit(peek())) return NULL;

        int divisor = 10;
        for (; isdigit(c = peek()); next()) {
            fractionalPart += (float)(c - '0') / divisor;
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

    JSONValue* number = MALLOC(sizeof(JSONValue));
    number->type = NUMBER;
    number->value.number = multiplier * (integerPart + fractionalPart);
    return number;
}

// Connor
const char t[] = "true";
const char f[] = "false";

JSONValue* parseBool(void) {
    JSONValue* newJSONBool = MALLOC(sizeof(JSONValue));
    newJSONBool->type = BOOLEAN;
    int i;
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
    } else {
        return NULL;
    }

    return newJSONBool;
}

// Ellis
JSONValue* parseNull(void) {
    if (parseLiteral("null")) {
        JSONValue* value = MALLOC(sizeof(JSONValue));
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

void testParser(void) {
    JSONValue* value;
    // char* rawjson;

//    // Parse null
//    cursor = "null";
//    value = parseNull();
//    destroyJSON(value);
//
//    cursor = "nul";
//    value = parseNull();
//
//    // Parse bools
//    cursor = "true";
//    value = parseBool();
//    destroyJSON(value);
//    cursor = "false";
//    value = parseBool();
//    destroyJSON(value);
//
//    cursor = "truth";
//    value = parseBool();
//    cursor = "fake";
//    value = parseBool();
//
//    // Parse numbers
//    cursor = "1";
//    value = parseNumber();
//    destroyJSON(value);
//    cursor = "1.0";
//    value = parseNumber();
//    destroyJSON(value);
//    cursor = "-123";
//    value = parseNumber();
//    destroyJSON(value);
//    cursor = "-123.0045";
//    value = parseNumber();
//    destroyJSON(value);
//    cursor = "1e-2";
//    value = parseNumber();
//    destroyJSON(value);
//    cursor = "-1.2E+2";
//    value = parseNumber();
//    destroyJSON(value);
//    cursor = "15E+0";
//    value = parseNumber();
//    destroyJSON(value);
//
//    cursor = "1.";
//    value = parseNumber();
//    cursor = "01";
//    value = parseNumber();
//
//    // Parse strings
//    cursor = "\"normal string\"";
//    value = parseString();
//    destroyJSON(value);
//    cursor = "\"str with \\b \\n \\r \\f \\\\ \\/ \\\" \\t escaped chars\"";
//    value = parseString();
//    destroyJSON(value);
//    cursor = "\"str with unicode: \\ua0f2 \\u0D3C \\u1234 \"";
//    value = parseString();
//    destroyJSON(value);
//    cursor = "\"\"";
//    value = parseString();
//    destroyJSON(value);
//
//    cursor = "\"bad escape \\uaf2\"";
//    value = parseString();
//    cursor = "\"\0\"";
//    value = parseString();
//    cursor = "\"\x20\"";
//    value = parseString();
//    cursor = "\"\x7F\"";
//    value = parseString();
//    cursor = "\"wrong quote'";
//    value = parseString();
//
//    // Parse arrays
//    cursor = "[]";
//    value = parseArray();
//    destroyJSON(value);
//    cursor = "[   ]";
//    value = parseArray();
//    destroyJSON(value);
//    cursor = "[  -1.0 ]";
//    value = parseArray();
//    destroyJSON(value);
//    cursor = "[1, \"string\",  true, null]";
//    value = parseArray();
//    destroyJSON(value);
//    cursor = "[ 1, \"string\", [ \"nested array\"]  ]";
//    value = parseArray();
//    destroyJSON(value);
//
//    cursor = "[\"extra comma\",]";
//    value = parseArray();
//    cursor = "[  \"missing bracket\" ";
//    value = parseArray();
//
//    // Parse objects
//    rawjson = "{}";
//    value = parseJSON(rawjson);
//    destroyJSON(value);
//
//    rawjson = "{  \"location\" : \"Terre Haute\"}";
//    value = parseJSON(rawjson);
//    JSONValue* location = JSONGet(value, "location");
//    JSONValue* invalid = JSONGet(value, "locatio"); // TODO: Since we're only checking the first character, this invalid key will return...
//    destroyJSON(value);

    value = parseJSON(exampleResponse);
    JSONValue* location = JSONGet(value, "location");
    JSONValue* locationName = JSONGet(location, "name");
    JSONValue* current = JSONGet(value, "current");
    JSONValue* temp_f = JSONGet(current, "temp_f");
    JSONValue* condition = JSONGet(current, "condition");
    JSONValue* conditionText = JSONGet(condition, "text");
    destroyJSON(value);
}
