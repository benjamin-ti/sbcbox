#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define MAXBUFFER 256

static char lk_pcLineBuffer[MAXBUFFER];

#define TEST_THREAD_NUM 200
static pthread_t lk_pTestThread[TEST_THREAD_NUM];

/*=========================================================================*/
static void*
Appl_TestThread(void *threadArg)
{
    while (1) {
        sleep(200);
        printf("TestThread: %i\n", (int)threadArg);
    }
}

/*=========================================================================*/
void static Appl_InitTestThreads()
{
    int iTestThreadNum;
    int iStatus;
    char* pcName;
    int i;

    iTestThreadNum = 0;

    for (i=0; i<TEST_THREAD_NUM; i++)
    {
        iTestThreadNum++;
        iStatus = pthread_create(&lk_pTestThread[i], NULL, Appl_TestThread, (void*)iTestThreadNum);
        if (iStatus != 0) {
            printf("Error %i in line %i", iStatus, __LINE__);
            return;
        }
        pcName = malloc(20);
        if (pcName == NULL) {
            printf("Error in line %i", __LINE__);
            return;
        }
        snprintf(pcName, 20, "Test%i", iTestThreadNum);
        iStatus = pthread_setname_np(lk_pTestThread[i], pcName);
        if (iStatus != 0) {
            printf("Error %i in line %i", iStatus, __LINE__);
            return;
        }
    }
}

/*=========================================================================*/
int main(void)
{
    FILE* fp = NULL;

    Appl_InitTestThreads();
    sleep(2);

    printf("Hello\n");

    fp = fopen("/etc/asound.conf", "r");
    if (fp == NULL) {
        printf("Can't open file\n");
        return 1;
    }

    while (!feof(fp)) {
        fgets(lk_pcLineBuffer, MAXBUFFER, fp);
        if (NULL == strstr(lk_pcLineBuffer, "this_should_never_happen")) {
            printf(lk_pcLineBuffer);
        }
    }

    fclose(fp);

    return 0;
}
