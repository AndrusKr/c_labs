#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct
{
    char *arr[100];
    char **arr_ptr;
    int count;
} StringArr;

void initStringArr(StringArr *strArr)
{
    strArr->arr_ptr = strArr->arr;
    strArr->count = 0;
}

void addStringToStringArr(StringArr *strArr, char *newElem)
{
    strArr->arr_ptr[strArr->count] = newElem;
    strArr->count++;
}

void printStringArr(StringArr *strArr)
{
    for (int i = 0; i < strArr->count; i++)
        printf("String %d : %s\n", i, strArr->arr[i]);
}

int main(int argc, char *argv[])
{
    StringArr a;
    initStringArr(&a);

    addStringToStringArr(&a, "abc");
    addStringToStringArr(&a, "def");
    addStringToStringArr(&a, "ghi");

    printStringArr(&a);

    return 0;
}

// #include <stdio.h>

// int main(void)
// {
//     int i;

//     char *arr[] = {"C", "C++", "Java", "VBA"};
//     char **ptr = arr;

//     ptr[4] = "Sander";

//     for (i = 0; i < 5; i++)
//         printf("String %d : %s\n", i + 1, ptr[i]);

//     return 0;
// }