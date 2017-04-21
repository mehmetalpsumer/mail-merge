#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "array.h"
// initializes the array
void arrayDefine(Array *array) {
    // initialize size and capacity
    array->size = 0;
    array->capacity = ARRAY_DEFAULT_SIZE;

    // allocate memory for vector->data
    array->data = malloc(sizeof(char*) * array->capacity);
}
// adds element to the array, increases size if required
void arrayAppend(Array *array, char *value) {
    // make sure there's room to expand into
    arrayCheckCapacity(array);
    array->data[array->size++] = value;
}
// checks capacity before adding an element and doubles the array size if needed
void arrayCheckCapacity(Array *array) {
    if (array->size >= array->capacity) {
        // double vector->capacity and resize the allocated memory accordingly
        array->capacity *= 2;
        array->data = realloc(array->data, sizeof(char*) * array->capacity);
    }
}
// frees the array from memory
void arrayFree(Array *array) {
    free(array->data);
}
// gets value from given index
char *arrayGet(Array *array, int index) {
    if (index >= array->size || index < 0) {
        printf("Index %d out of bounds for array of size %d\n", index, array->size);
        exit(1);
    }
    return array->data[index];
}
/*
 * works only for variables that are stored as "value:key"
 * key is searched and value is returned
 */
char *arrayFind(Array *array, char *query){
    int i;
    const char *delimeter = ":";
    for(i=0; i<array->size; i++){
        char *token, *value;
        value = strdup(array->data[i]);
        token = strtok(value, delimeter);
        strcpy(value, token);
        token = strtok(NULL, delimeter);
        if(strcmp(token, query)==0){
            return value;
        }
    }
    return NULL;
}
// checks if a value exists in the array
bool arrayExists(Array *array, char *query){
    int i;
    for(i=0; i<array->size; i++){
        if(strcmp(array->data[i], query)==0)
            return true;
    }
    return false;
}
// checks if a key exists in the array (to prevent same tag being entered more than once)
bool arrayKeyExists(Array *array, char *query){
    int i;
    const char *delimeter = ":";
    for(i=0; i<array->size; i++){
        char *token, *value;
        value = strdup(array->data[i]);
        token = strtok(value, delimeter);
        token = strtok(NULL, delimeter);
        if(strcmp(token, query)==0){
            return true;
        }
    }
    return false;
}