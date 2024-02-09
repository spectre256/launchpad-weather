#include "json.h"
#include <ctype.h>

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
JSONValue* parseArray(void) {
    JSONValue* newJSONArray;
    newJSONArray = (JSONValue*)malloc(sizeof(JSONValue));
    Array* modifiableArray = newArray();
    while(peek() != ']'){
        if(peek() == ','){
            next();
        }
        char curChar = next();
        arrayAppend(modifiableArray, &curChar);
    }
    newJSONArray->value.array = modifiableArray;
    newJSONArray->type = ARRAY;
    next();
    return newJSONArray;
}



// Ellis
JSONValue parseValue(void) {

}

// Connor
JSONValue* parseString(void) {
    JSONValue* newJSONString;
    int i;
    newJSONString = (JSONValue*)malloc(sizeof(JSONValue));
    newJSONString->type = STRING;
    newJSONString->value.string.str = cursor;
    while(peek() != '"'){
        if(peek() == '\\'){
            newJSONString->value.string.length++;
            next();
            if(peek() == 'u'){
                next();
                for(i = 0; i < 4; i++){
                    if(isdigit(peek()) == 0){
                        return NULL;
                    }
                    newJSONString->value.string.length++;
                    next();
                }
                if(peek() == '\\'){
                    continue;
                }
            } else if(peek() != ('"' || '\\' || '/' || 'b' || 'f' || 'n' || 'r' || 't')){
                return NULL;
            }
        }
        newJSONString->value.string.length++;
        next();
    }
    return newJSONString;
}

// Ellis
JSONValue parseNumber(void) {

}

// Connor
JSONValue* parseBool(void) {
    JSONValue* newJSONBool;
    newJSONBool = (JSONValue*)malloc(sizeof(JSONValue));
    newJSONBool->type = BOOLEAN;
    int i;
    const char t[5] = "true";
    const char f[6] = "false";
    if(peek() == 't'){
        for(i = 0; i < sizeof(t)/sizeof(t[0]); i++){
            if(peek() == t[i]){
                next();
            } else{
                return NULL;
            }
        }
        newJSONBool->value.boolean = true;
    } else if(peek() == 'f'){
        for(i = 0; i < sizeof(f)/sizeof(f[0]); i++){
            if(peek() == f[i]){
                next();
            } else{
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
JSONValue parseNull(void) {

}

// Connor
void parseWhitespace(void) {
    while(peek() == ' ' || peek() == '\n' || peek() == '\r' || peek() == '\t'){
        next();
    }
}
