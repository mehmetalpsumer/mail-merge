#ifndef ARRAY_H_
#define ARRAY_H_
#endif
#define ARRAY_DEFAULT_SIZE 10

// Array struct
typedef struct {
  int size;      // total cells used
  int capacity;  // total available cells
  char **data;     // array of strings
} Array;

void arrayDefine(Array *array);
void arrayAppend(Array *array, char *value);
void arrayCheckCapacity(Array *array);
void arrayFree(Array *array);
char *arrayFind(Array *array, char *query);
char *arrayGet(Array *array, int index);
bool arrayExists(Array *array, char *query);
bool arrayKeyExists(Array *array, char *query);