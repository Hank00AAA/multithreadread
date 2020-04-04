#include <stdio.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>          
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>

char *buf;
pthread_mutex_t mutex;
int count;

struct TaskInfo {
    char *path;
    long offset;
    int len;
};

void* MultiThreadRead(void *args)
{
    struct TaskInfo *task = (struct TaskInfo*)args;
    int fd;
    off_t offsetRet;
    int n;
    int curPos = 0;

    // 1. open file
    fd = open(task->path, O_RDONLY);
    if (fd == -1) {
        printf("error occured while open file:%s %s\n", task->path, strerror(errno));
        return 0;
    }

    // 2. offset
    offsetRet = lseek(fd, task->offset, SEEK_SET);
    if (offsetRet == -1) {
        printf("error occur while lseek %s\n", strerror(errno));
        close(fd);
        return 0;
    }

    // 3. copy file to buffer
    while (task->len - curPos > 0) {
        n = read(fd, buf + task->offset + curPos, task->len - curPos);
        if (n == -1) {
            if (errno == EAGAIN) {
                continue;
            }
            printf("error occur while read %s\n", strerror(errno));
            close(fd);
            return 0;
        }

        if (n == 0) {
            goto FINI;
            return 0;
        }

        curPos += n;
    }
FINI:
    pthread_mutex_lock(&mutex);
    ++count;
    printf("%d ", count);
    pthread_mutex_unlock(&mutex);

    return 0;
}

long getFileSize(int fd) 
{
    int ret;
    struct stat fileStat;
    int res;

    ret = fstat(fd, &fileStat);
    if (ret == -1) {
        printf("error occur while fstat error:%s\n", strerror(errno));
        _exit(0);
    }

    res = fileStat.st_size;
    return res;
}

void Start(int threadNum, char* path)
{
    int fd;
    long size;
    int idx;
    struct TaskInfo* args;
    int spilt;
    int start = 0;
    pthread_t pid;
    clock_t s, e;

    pthread_mutex_init(&mutex, NULL);
    count = 0;

    fd = open(path, O_RDONLY);
    if (fd == -1) {
        printf("error occured while open file:%s\n", strerror(errno));
        _exit(0);
    }

    size = getFileSize(fd);
    spilt = size / threadNum + 1;
    buf = (char*)malloc(sizeof(char) * size + 1);
    buf[size] = 0;
    s = clock();
    for (idx = 0; idx < threadNum; ++idx) {
        args = (struct TaskInfo*)malloc(sizeof(struct TaskInfo));
        args->len = spilt;
        args->offset = start;
        args->path = path;
        pthread_create(&pid, NULL, MultiThreadRead, args);
        pthread_detach(pid);
        start += spilt;
    }

    while (1) {
        if (count == threadNum) {
            e = clock();
           // printf("%s\n", buf);
            printf("\ncost time: %f\n", ((double)e - s));
            return;
        }
    }
}

int main(int argc, char* argv[])
{
    int threadNum = atoi(argv[1]);
    printf("threadNum: %d\n", threadNum);
    Start(threadNum, "test");
    return 0;
}

