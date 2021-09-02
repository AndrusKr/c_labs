#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <wait.h>
#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include "param_pam_pams.h"

void sigHandler(int signum);
void setSigHandler(void (*handler)(void *), int sig_no, int flags);
void printErrorExit(const char *s_name, const char *msg, const int proc_num);
void forker(int curr_number, int childs_count);
void killWaitForChildren();
void waitForChildren();

int main(int argc, char *argv[])
{
    execName = basename(argv[0]);
    pidsList = (pid_t *)mmap(pidsList, (2 * PROC_COUNT) * sizeof(pid_t), PROT_READ | PROT_WRITE,
                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    int i = 0; // initialize pids list with zeros [1]
    for (i = 0; i < 2 * PROC_COUNT; ++i)
        pidsList[i] = 0;
    forker(0, CHILDS_COUNT[0]); // create processes tree [2]
    setSigHandler(&killWaitForChildren, SIGTERM, 0);
    if (proc_id == 0)
    { // main process waits [3]
        waitForChildren();
        munmap(pidsList, (2 * PROC_COUNT) * sizeof(pid_t));
        return 0;
    }
    on_exit(&waitForChildren, NULL);
    pidsList[proc_id] = getpid(); // save pid to list
    if (proc_id == STARTING_PROC_ID)
    { // starter waits for all pids to be available [3]
        do
        {
            for (i = 1; (i <= PROC_COUNT) && (pidsList[i] != 0); ++i)
                if (pidsList[i] == -1)
                {
                    printErrorExit(execName, "Error: not all processes forked or initialized.\nHalting.", 0);
                    exit(1);
                }
        } while (i < PROC_COUNT);
        FILE *tmp = fopen(tmp_file_name, "wt");
        if (tmp == NULL)
            printErrorExit(execName, "Can't create temp file", 0);

        for (i = 1; i < PROC_COUNT; ++i)
            fprintf(tmp, "%d\n", pidsList[i]);
        fclose(tmp);
        pidsList[0] = 1; // all pids are set
        setSigHandler(&sigHandler, 0, 0);
        do
        {
            for (i = 1 + PROC_COUNT; (i < 2 * PROC_COUNT) && (pidsList[i] != 0); ++i)
                if (pidsList[i] == -1)
                {
                    printErrorExit(execName, "Error: not all processes forked or initialized.\nHalting.", 0);
                    exit(1);
                }
        } while (i < 2 * PROC_COUNT);
        for (i = PROC_COUNT + 1; i < 2 * PROC_COUNT; ++i)
            pidsList[i] = 0; //reset flags
        sigHandler(0);       // start signal-flow
    }
    else
    { // other processes
        do
        {
            // wait for all pids to be written
        } while (pidsList[0] == 0);
        setSigHandler(&sigHandler, 0, 0);
    }
    while (1)
        pause(); // start cycle
    return 0;
}

void printErrorExit(const char *s_name, const char *msg, const int proc_num)
{
    fprintf(stderr, "%s: %s %d\n", s_name, msg, proc_num);
    fflush(stderr);
    pidsList[proc_num] = -1;
    exit(1);
}

void waitForChildren()
{
    int i = CHILDS_COUNT[proc_id];
    while (i > 0)
    {
        wait(NULL);
        --i;
    }
}

long long current_time()
{
    struct timeval time;
    gettimeofday(&time, NULL);
    return time.tv_usec;
}

void killWaitForChildren()
{
    int i = 0;
    for (i = 0; i < CHILDS_COUNT[proc_id]; ++i)
        kill(pidsList[CHILDS_IDS[proc_id][i]], SIGTERM);
    waitForChildren();
    if (proc_id != 0)
        printf("%d %d завершил работу после %d SIGUSR1 и %d SIGUSR2\n",
               getpid(), getppid(), usr_amount[0][1], usr_amount[1][1]);
    fflush(stdout);
    exit(0);
}

void sigHandler(int signum)
{
    if (signum == SIGUSR1)
        signum = 0;
    else if (signum == SIGUSR2)
        signum = 1;
    else
        signum = -1;
    if (signum != -1)
    {
        ++usr_amount[signum][0];
        ++usr_recv[signum];
        printf("%d %d %d получил %s%d %lld\n", proc_id, getpid(), getppid(),
               "USR", signum + 1, current_time());
        fflush(stdout);
        if (proc_id == 1)
        {
            if (usr_amount[0][0] + usr_amount[1][0] == MAX_USR_COUNT)
                killWaitForChildren();
            pidsList[PROC_COUNT + 6] = pidsList[PROC_COUNT + 4] = 0;
        }
        if (proc_id == 8)
            do
                ;
            while ((pidsList[PROC_COUNT + 6] + pidsList[PROC_COUNT + 4]) != 2);

        if (!((usr_recv[0] == RECV_SIGNALS_COUNT[0][proc_id]) &&
              (usr_recv[1] == RECV_SIGNALS_COUNT[1][proc_id])))
        {
            if ((proc_id == 4) && (signum == 0))
                pidsList[PROC_COUNT + 4] = 1;
            return;
        }
        usr_recv[0] = usr_recv[1] = 0;
    }
    char to = SEND_TO[proc_id];
    if (to != 0)
    {
        signum = ((SEND_SIGNALS[proc_id] == SIGUSR1) ? 1 : 2);
        ++usr_amount[signum - 1][1];
    }
    printf("%d %d %d послал %s%d %lld\n", proc_id, getpid(), getppid(),
           "USR", signum, current_time());
    fflush(stdout);
    if (to > 0)
        kill(pidsList[to], SEND_SIGNALS[proc_id]);
    else if (to < 0)
        kill(-getpgid(pidsList[-to]), SEND_SIGNALS[proc_id]);
    else
        return;
    if (proc_id == 6)
        pidsList[PROC_COUNT + 6] = 1;
}

void setSigHandler(void (*handler)(void *), int sig_no, int flags)
{
    struct sigaction sa, oldsa; // set sighandler [4]
    sigset_t block_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGUSR1);
    sigaddset(&block_mask, SIGUSR2);
    sa.sa_mask = block_mask;
    sa.sa_flags = flags;
    sa.sa_handler = handler;
    if (sig_no != 0)
    {
        sigaction(sig_no, &sa, &oldsa);
        return;
    }
    int i = 0;
    for (i = 0; i < PROC_COUNT; ++i)
    {
        char to = SEND_TO[i];
        /* someone sends me a signal: */
        if (((to > 0) && (to == proc_id)) || /* <- directly */
            ((to < 0) && (getpgid(pidsList[-to]) == getpgid(0))))
        { /* <- or through the group */
            if (SEND_SIGNALS[i] != 0)
            { // signal is really sent
                if (sigaction(SEND_SIGNALS[i], &sa, &oldsa) == -1)
                    printErrorExit(execName, "Can't set sighandler!", proc_id);
            }
        }
    }
    pidsList[proc_id + PROC_COUNT] = 1;
}

void forker(int curr_number, int childs_count)
{
    pid_t pid = 0;
    int i = 0;
    for (i = 0; i < childs_count; ++i)
    {
        int chld_id = CHILDS_IDS[curr_number][i];
        if ((pid = fork()) == -1)
            printErrorExit(execName, "Can't fork!", chld_id);
        else if (pid == 0)
        { /*  child    */
            proc_id = chld_id;
            if (CHILDS_COUNT[proc_id] != 0)
                forker(proc_id, CHILDS_COUNT[proc_id]); // fork children
            break;
        }
        else
        { // pid != 0 (=> parent)
            static int prev_chld_grp = 0;
            int grp_type = GROUP_TYPE[chld_id];
            if (grp_type == 0)
            {
                if (setpgid(pid, pid) == -1)
                    printErrorExit(execName, "Can't set group", chld_id);
                else
                    prev_chld_grp = pid;
            }
            else if (grp_type == 1)
            {
                if (setpgid(pid, getpgid(0)) == -1)
                    printErrorExit(execName, "Can't set group", chld_id);
            }
            else if (grp_type == 2)
                if (setpgid(pid, prev_chld_grp) == -1)
                    printErrorExit(execName, "Can't set group", chld_id);
        }
    }
}