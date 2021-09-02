#include <signal.h>

#define PROC_COUNT (9)
#define STARTING_PROC_ID (1)
#define MAX_CHILDS_COUNT (3)
#define MAX_USR_COUNT (101)

// Дерево процессов
// (1) 1->(2,3,4)   2->(5,6)   6->7  7->8       0  1  2  3  4  5  6  7  8
const unsigned char CHILDS_COUNT[PROC_COUNT] = {1, 3, 2, 0, 0, 0, 1, 1, 0};

const unsigned char CHILDS_IDS[PROC_COUNT][MAX_CHILDS_COUNT] =
    {
        {1},       /* 0 */
        {2, 3, 4}, /* 1 */
        {5, 6},    /* 2 */
        {0},       /* 3 */
        {0},       /* 4 */
        {0},       /* 5 */
        {7},       /* 6 */
        {8},       /* 7 */
        {0}        /* 8 */
};

// Group types:
// 0 = pid;
// 1 = parent's pgid
// 2 = previous child group                   0  1  2  3  4  5  6  7  8
const unsigned char GROUP_TYPE[PROC_COUNT] = {0, 1, 0, 2, 0, 0, 0, 1, 1};

// Whome to send signal:
// 0 = none
// positive = pid
// negative = processes group
// f/ex: "-x" means, that signal is sent to the group, process with pid = x is member of
//                                0,  1,  2,  3,  4,  5,  6,  7,  8
const char SEND_TO[PROC_COUNT] = {0, -6, 1, 0, -2, 0, 4, 4, 4};

const int SEND_SIGNALS[PROC_COUNT] =
    {
        0,       /* 0 */
        SIGUSR1, /* 1 */
        SIGUSR2, /* 2 */
        0,       /* 3 */
        SIGUSR1, /* 4 */
        0,       /* 5 */
        SIGUSR1, /* 6 */
        SIGUSR2, /* 7 */
        SIGUSR1  /* 8 */
};

const char RECV_SIGNALS_COUNT[2][PROC_COUNT] =
    {
        /*    0, 1, 2, 3, 4, 5, 6, 7, 8  */
        {0, 0, 1, 1, 2, 0, 1, 1, 1}, /* SIGUSR1 */
        {0, 1, 0, 0, 1, 0, 0, 0, 0}  /* SIGUSR2 */
};

//                         U1, U2
volatile int usr_recv[2] = {0, 0};

volatile int usr_amount[2][2] =
    {
        /*r, s*/
        {0, 0}, /* SIGUSR1 */
        {0, 0}  /* SIGUSR2 */
};

int proc_id = 0;
char *execName = NULL;
pid_t *pidsList = NULL;
char *tmp_file_name = "/tmp/pids.log";