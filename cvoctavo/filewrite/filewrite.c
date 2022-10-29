#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

// #define TEST_BUF_SIZE 1024*1014
#define TEST_BUF_SIZE 10*1014*1024
#define TEST_FILENAME "filewrite_buf.bin"

int doWrite(char* pcBuf, unsigned int uiBufSize, char cPattern, char* pcBufFileName)
{
    FILE* fp = NULL;
    char* pc;
    int iErr, i, iSize, iWrite;

    for (i=0; i<uiBufSize; i++) {
        pcBuf[i] = cPattern;
    }

    fp = fopen(pcBufFileName, "w");
    if (fp == NULL) {
        iErr = errno;
        fprintf(stderr, "%s, %i: %s open failed:\n    %i: %s\n", __FILE__, __LINE__,
                pcBufFileName, iErr, strerror(iErr));
        return errno;
    }
    pc = pcBuf;
    iSize = uiBufSize;
    do
    {
        iWrite = 1024;
        if (iSize < iWrite)
            iWrite = iSize;
        i = fwrite(pc, 1, iWrite, fp);
        iSize-=i;
        pc+=i;

        struct timespec t;
        t.tv_sec = 0;
        t.tv_nsec = 1*1000*1000;
        nanosleep(&t, NULL);

    } while(i>0);
    fclose(fp);

    return 0;
}

int doRead(char* pcBuf, unsigned int uiBufSize, char cPattern, char* pcBufFileName)
{
    FILE* fp = NULL;
    char* pc;
    int iErr, i, iSize;

    fp = fopen(pcBufFileName, "r");
    if (fp == NULL) {
        iErr = errno;
        fprintf(stderr, "%s, %i: %s open failed:\n    %i: %s\n", __FILE__, __LINE__,
                pcBufFileName, iErr, strerror(iErr));
        return errno;
    }
    pc = pcBuf;
    iSize = uiBufSize;
    do
    {
        i = fread(pc, 1, iSize, fp);
        iSize-=i;
        pc+=i;

        struct timespec t;
        t.tv_sec = 0;
        t.tv_nsec = 1*1000*1000;
        nanosleep(&t, NULL);

    } while(i>0);
    fclose(fp);

    for (i=0; i<uiBufSize; i++) {
        if (pcBuf[i] != cPattern) {
            printf("Error at Pos %i: %hx instead of %hx\n", i, (char)pcBuf[i], (char)0xAA);
            break;
        }
    }

    return 0;
}

int doTest(char* pcBuf, unsigned int uiBufSize, char cPattern)
{
    int iErr;
    iErr = doWrite(pcBuf, uiBufSize, cPattern, TEST_FILENAME);
    if (iErr != 0)
        return iErr;
    memset(pcBuf, 0, uiBufSize);
    iErr = doRead(pcBuf, uiBufSize, cPattern, TEST_FILENAME);
    if (iErr != 0)
        return iErr;

    return 0;
}

int main(void)
{

    unsigned int uiBufSize = TEST_BUF_SIZE;
    char* pcBuf;
    int iErr, i;

    pcBuf = malloc(uiBufSize);

    for (i=0; i<100000; i++)
    {
        printf("%i\n", i);
        iErr = doTest(pcBuf, uiBufSize, 0xAA);
        if (iErr != 0)
            return iErr;
        iErr = doTest(pcBuf, uiBufSize, 0x55);
        if (iErr != 0)
            return iErr;
        iErr = doTest(pcBuf, uiBufSize, 0xA5);
        if (iErr != 0)
            return iErr;
        iErr = doTest(pcBuf, uiBufSize, 0x5A);
        if (iErr != 0)
            return iErr;
        iErr = doTest(pcBuf, uiBufSize, 0xFF);
        if (iErr != 0)
            return iErr;
        iErr = doTest(pcBuf, uiBufSize, 0x00);
        if (iErr != 0)
            return iErr;
        iErr = doTest(pcBuf, uiBufSize, 0x4D);
        if (iErr != 0)
            return iErr;

        struct timespec t;
        t.tv_sec = 0;
        t.tv_nsec = 100*1000*1000;
        nanosleep(&t, NULL);
    }

    free(pcBuf);

    return 0;
}
