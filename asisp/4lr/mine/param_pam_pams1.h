#define PROC_COUNT (9)
#define STARTING_PROC_ID (1)
#define MAX_CHILDS_COUNT (4)
#define MAX_USR_COUNT (101)

// (№2) 1->(2,3,4,5) 2->6 3->7 4->8             0  1  2  3  4  5  6  7  8
const unsigned char CHILDS_COUNT[PROC_COUNT] = {1, 4, 1, 1, 1, 0, 0, 0, 0};

const unsigned char CHILDS_IDS[PROC_COUNT][MAX_CHILDS_COUNT] =
    {
        {1},          /* 0 */
        {2, 3, 4, 5}, /* 1 */
        {6},          /* 2 */
        {7},          /* 3 */
        {8},          /* 4 */
        {0},          /* 5 */
        {0},          /* 6 */
        {0},          /* 7 */
        {0}           /* 8 */
};

// Group types:
// 0 = pid;
// 1 = parent's pgid
// 2 = previous child group                   0  1  2  3  4  5  6  7  8
const unsigned char GROUP_TYPE[PROC_COUNT] = {0, 1, 0, 2, 2, 0, 0, 0, 0};
