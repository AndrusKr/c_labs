// 1.Написать программу шифрования всех файлов для заданного каталога его подкаталогов.
// Пользователь задаёт имя каталога.
// Главный процесс открывает каталоги и запускает для каждого файла каталога
// отдельный поток  шифрования файла.
// Каждый поток выводит на экран свой id, полный путь к файлу,
// общее число зашифрованных байт.
// Число одновременно работающих потоков N (вводится пользователем).
// Проверить работу программы для каталога /etc.
#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <alloca.h>
#include <libgen.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>

#include "AndrusStringArr.c"

char *programName;
volatile sig_atomic_t childThreadsCount = 0,
                      maxChildThreadsCount = 0;
pthread_t *allowedThreads;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *encrypt(void *arg)
{
    pthread_mutex_lock(&lock);
    char *path = (char *)arg;
    FILE *file;
    file = fopen(path, "rb+");
    if (!file)
        fprintf(stderr, "Unable to open file %s", path);
    int ch, encryptedBytesCount = 0;
    fpos_t pos, pos_end;
    fgetpos(file, &pos);
    fseek(file, 0L, SEEK_END);
    fgetpos(file, &pos_end);
    rewind(file);
    while (pos.__pos != pos_end.__pos)
    {
        ch = fgetc(file);
        if (EOF == ch)
            break;
        if (!iscntrl(ch) && !iscntrl(ch + 1))
        {
            fsetpos(file, &pos);
            fputc(ch + 1, file);
            fflush(file);
            encryptedBytesCount++;
        }
        pos.__pos += 1;
        fsetpos(file, &pos);
    }
    printf("%d %s ENCRYPTED BYTES #%d \n", (int)pthread_self(), path, encryptedBytesCount);
    if (fclose(file) == -1)
        fprintf(stderr, "%s: %s. File: %s\n", programName, strerror(errno), path);
    pthread_mutex_unlock(&lock);
    pthread_exit(NULL);
}

void getAllDirFiles(StringArr *allFiles, char *curPath)
{
    DIR *curDir;
    if ((curDir = opendir(curPath)) == NULL)
    {
        fprintf(stderr, "%s: %s. File: %s\n", programName, strerror(errno), curPath);
        errno = 0;
        return;
    }
    struct dirent *dirEntry;
    char *dirEntryName = alloca(strlen(curPath) + NAME_MAX + 2);
    if (dirEntryName == NULL)
    {
        fprintf(stderr, "%s: %s.", programName, strerror(errno));
        return;
    }
    errno = 0;
    while (dirEntry = readdir(curDir))
    {
        if (strcmp(".", dirEntry->d_name) && strcmp("..", dirEntry->d_name))
        {
            strcpy(dirEntryName, curPath);
            if (strcmp(dirEntryName, "/") != 0)
                strcat(dirEntryName, "/");
            strcat(dirEntryName, dirEntry->d_name);
            struct stat dirEntryStatus;
            if (lstat(dirEntryName, &dirEntryStatus) == -1)
            {
                fprintf(stderr, "%s:   %s.   File:   %s\n", programName, strerror(errno), curPath);
                return;
            }
            if (S_ISDIR(dirEntryStatus.st_mode))
                getAllDirFiles(allFiles, dirEntryName);
            if (S_ISREG(dirEntryStatus.st_mode))
                addStringToStringArr(allFiles, dirEntryName);
        }
    }
    if (errno != 0)
    {
        fprintf(stderr, "%s: %s. File: %s\n", programName, strerror(errno), curPath);
        errno = 0;
        return;
    }
    if (closedir(curDir) == -1)
    {
        fprintf(stderr, "%s: %s. File: %s\n", programName, strerror(errno), curPath);
        errno = 0;
        return;
    }
    return;
}

int main(int argc, char *argv[])
{
    programName = alloca(strlen(basename(argv[0])));
    strcpy(programName, basename(argv[0]));
    // argc = 2;
    if (argc >= 2)
    {
        maxChildThreadsCount = atoi(argv[2]);
        // maxChildThreadsCount = 3;
        allowedThreads = (pthread_t *)malloc(sizeof(pthread_t) * maxChildThreadsCount);
        if (maxChildThreadsCount > 0)
        {
            char *encryptingDirName;
            encryptingDirName = realpath(argv[1], NULL);
            // encryptingDirName = "./etc";
            if (encryptingDirName != NULL)
            {
                StringArr filePaths;
                initStringArr(&filePaths);
                getAllDirFiles(&filePaths, encryptingDirName);
                for (int i = 0; i < filePaths.count; i++)
                {
                    pthread_join(allowedThreads[childThreadsCount], NULL);
                    if (pthread_create(&allowedThreads[childThreadsCount], NULL, encrypt, filePaths.arr[i]) == 0)
                        childThreadsCount++;
                    if (childThreadsCount == maxChildThreadsCount)
                        childThreadsCount = 0;
                }
            }
        }
    }
    // wait all threads by joining them
    for (int i = 0; i < maxChildThreadsCount; i++)
        pthread_join(allowedThreads[i], NULL);
    return 0;
}