/*=========================================================================*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#define DEFAULT_FILESIZE_MB 1
#define DEFAULT_BUFSIZE_KB 1
#define DEFAULT_SLEEP_US 1000
#define DEFAULT_RUNS 1000000
#define TEST_FILENAME "filewrite_buf.bin"

static unsigned int lk_uiFileSize = DEFAULT_FILESIZE_MB*1024*1024;
static unsigned int lk_uiBufSize  = DEFAULT_BUFSIZE_KB*1024;
static unsigned int lk_uiSleep_ns  = DEFAULT_SLEEP_US*1000;
static unsigned int lk_uiRuns  = DEFAULT_RUNS;
/*=========================================================================*/

/*=========================================================================*/
static int doWrite(char* pcBuf, unsigned int uiBufSize, char cPattern, char* pcBufFileName)
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
        iWrite = lk_uiBufSize;
        if (iSize < iWrite)
            iWrite = iSize;
        i = fwrite(pc, 1, iWrite, fp);
        iSize-=i;
        pc+=i;

        struct timespec t;
        t.tv_sec = 0;
        t.tv_nsec = lk_uiSleep_ns;
        nanosleep(&t, NULL);

    } while(i>0);
    fclose(fp);

    return 0;
}

/*=========================================================================*/
static int doRead(char* pcBuf, unsigned int uiBufSize, char cPattern, char* pcBufFileName)
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
        t.tv_nsec = lk_uiSleep_ns;
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

/*=========================================================================*/
static int doTest(char* pcBuf, unsigned int uiBufSize, char cPattern)
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

/*=========================================================================*/
void Usage(char *programName)
{
    fprintf(stdout,"%s usage:\n\n",programName);
    // Modify here to add your usage message when the program is
    // called without arguments
    printf("-fFILESIZE:  File size in MB   (Defaults to %u)\n", DEFAULT_FILESIZE_MB);
    printf("-bBUFSIZE:   Buffer size in KB (Defaults to %u)\n", DEFAULT_BUFSIZE_KB);
    printf("-sSLEEP:     Sleep time in us  (Defaults to %u)\n", DEFAULT_SLEEP_US);
    printf("-rRUNS:      Number of runs    (Defaults to %u)\n", DEFAULT_RUNS);
    printf("\nExample: Call '%s -f10' for using file size of 10 MB.\n", programName);
    printf("Note: There should be no blank between '-f' and '10'!\n");
}


/*=========================================================================*/
// returns the index of the first argument that is not an option; i.e.
// does not start with a dash or a slash
static int HandleOptions(int argc, char *argv[])
{
    int i,firstnonoption=0;

    for (i=1; i< argc;i++) {
        if (argv[i][0] == '/' || argv[i][0] == '-') {
            switch (argv[i][1]) {
                // An argument -? means help is requested
                case '?':
                    Usage(argv[0]);
                    exit(0);
                case 'h':
                case 'H':
                    if (!strcmp(argv[i]+1,"help")) {
                        Usage(argv[0]);
                        exit(0);
                    }
                    else {
                        Usage(argv[0]);
                        exit(0);
                    }
                    // If the option -h means anything else
                    // in your application add code here
                    // Note: this falls through to the default
                    // to print an "unknow option" message
                // add your option switches here
                case 'f':
                    lk_uiFileSize = atoi(argv[i]+2)*1024*1024;
                    break;
                case 'b':
                    lk_uiBufSize = atoi(argv[i]+2)*1024;
                    break;
                case 's':
                    lk_uiSleep_ns = atoi(argv[i]+2)*1000;
                    break;
                case 'r':
                    lk_uiRuns = atoi(argv[i]+2);
                    break;
                default:
                    fprintf(stdout, "unknown option %s\n", argv[i]);
                    exit(-1);
            }
        }
        else {
            firstnonoption = i;
            break;
        }
    }
    return firstnonoption;
}

/*=========================================================================*/
int main(int argc, char *argv[])
{
    char* pcBuf;
    int iErr, i;

    if (argc == 1)
    {
        // No arguments
    }
    else
    {
        // handle the program options
        HandleOptions(argc,argv);
    }

    pcBuf = malloc(lk_uiFileSize);

    for (i=0; i<lk_uiRuns; i++)
    {
        printf("%i\n", i);
        iErr = doTest(pcBuf, lk_uiFileSize, 0xAA);
        if (iErr != 0)
            return iErr;
        iErr = doTest(pcBuf, lk_uiFileSize, 0x55);
        if (iErr != 0)
            return iErr;
        iErr = doTest(pcBuf, lk_uiFileSize, 0xA5);
        if (iErr != 0)
            return iErr;
        iErr = doTest(pcBuf, lk_uiFileSize, 0x5A);
        if (iErr != 0)
            return iErr;
        iErr = doTest(pcBuf, lk_uiFileSize, 0xFF);
        if (iErr != 0)
            return iErr;
        iErr = doTest(pcBuf, lk_uiFileSize, 0x00);
        if (iErr != 0)
            return iErr;
        iErr = doTest(pcBuf, lk_uiFileSize, 0x4D);
        if (iErr != 0)
            return iErr;

        struct timespec t;
        t.tv_sec = 0;
        t.tv_nsec = lk_uiSleep_ns;
        nanosleep(&t, NULL);
    }

    free(pcBuf);

    return 0;
}
