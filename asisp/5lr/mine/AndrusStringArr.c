#include <string.h>
#include <stdlib.h>

typedef struct
{
    char *arr[100];
    int count;
} StringArr;

void initStringArr(StringArr *strArr)
{
    strArr->count = 0;
}

void addStringToStringArr(StringArr *strArr, char *newElem)
{
    char *copiedStr = (char *)malloc((strlen(newElem) + 1) * sizeof(char));
    strcpy(copiedStr, newElem);
    strArr->arr[strArr->count] = copiedStr;
    strArr->count++;
}