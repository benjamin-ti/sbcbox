/***************************************************************************/
/*                          Display Program Tool                           */
/*                    Copyright (C) Carl Valentin GmbH                     */
/***************************************************************************/

/*. IMPORT ================================================================*/

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h> // uint32_t
#include <linux/ioctl.h>
#include <linux/spi/spidev.h>

#include "vcplatform.h"
#include "atmega.h"
#include "byteoperation.h"

/*. ENDIMPORT =============================================================*/


/*. EXPORT ================================================================*/

/*. ENDEXPORT =============================================================*/


/*. LOCAL =================================================================*/

/*-------------------------------- Makros ---------------------------------*/

#define VERSION$ "0.1"

#define PrintfErrAndExit(fmt, ...)  PrintfErrorAndExitReal(__LINE__, fmt, ##__VA_ARGS__)
#define ARRAY_SIZE(array) sizeof(array)/sizeof(array[0])

/*--------------------------- Typdeklarationen ----------------------------*/

typedef enum {
    MBOARD_BEAGLE,
    MBOARD_CV,
} MBOARD;

/*------------------------------ Prototypen -------------------------------*/

static void PrintfErrorAndExitReal(unsigned int uiLine, const char* fmt, ...);
static int GPIOWrite(int pin, int value);
static void WriteValue2File(char* pcFile, char* pcValue);

/*------------------------ Konstantendeklarationen ------------------------*/

static const char* lk_spidev = "/dev/spidev1.1";
static const AtmelID idATMEL = ATMEGA64;

/*------------------------- modulglobale Variable -------------------------*/

static int lk_fd = -1;
static BOOL   lk_pinsAreSetToSPI;
static MBOARD lk_board = MBOARD_CV;
static int lk_verbose;
static BOOL   lk_isTestOnly;

/*. ENDLOCAL ==============================================================*/

/*=========================================================================*/
static void Usage(char *programName)
{
    //_ Programm-Name needs only to be translated if these names are
    //_ localized too!
    printf("Carl Valentin Display Program Tool %s\n\n", VERSION$);

    printf("%s usage:\n", programName);
    printf("%s <options> updFile\n\n", programName);
    printf("Program must be able to modify pin functionality over pinmux-helper\n");
    printf("GPIOs must be exported and configured (dir) for sysfs use\n\n");
    printf("Options:\n");
    printf("-b BeagleBone: SPI1_SCLK: P9_31 (+GPIO out); SPI1_D0: P9_29 (RXD);\n");
    printf("               SPI1_D1: P9_30 (TXD), Reset: P9_27 (GPIO out); RXD: D0\n");
    printf("-c CV-Main-Board (Default): SPI1_SCLK: ECAP0_PWM (+GPIO out); SPI1_D0: UART0_CTSN (RXD);\n");
    printf("               SPI1_D1: UART0_CTSN (TXD), Reset: UART0_TXD (GPIO out); RXD: D0\n");
    printf("-vLevel        Level=verbose level (0=Default)\n");
    printf("-m             Verify\n");
    printf("-n             Verify only\n");
    printf("-t             run a few tests only\n");
}

/*=========================================================================*/
static void CleanAndExit(int ret)
{
    static int inExit = 0;

    if (inExit)
        return;

    inExit = 1;

    if (lk_fd > 0)
        close(lk_fd);

    switch (lk_board)
    {
        case MBOARD_CV:
            GPIOWrite(43, 1); // Set Reset
            break;
        case MBOARD_BEAGLE:
            GPIOWrite(115, 1); // Set Reset
            break;
    }

    if (lk_pinsAreSetToSPI)
    {
        switch (lk_board)
        {
            case MBOARD_CV:
                WriteValue2File("/sys/devices/platform/ocp/ocp:UART0_CTSN_pinmux/state", "default");
                WriteValue2File("/sys/devices/platform/ocp/ocp:UART0_RTSN_pinmux/state", "default");
                WriteValue2File("/sys/devices/platform/ocp/ocp:ECAP0_PWM_pinmux/state", "default");
                break;

            case MBOARD_BEAGLE:
                WriteValue2File("/sys/devices/platform/ocp/ocp:P9_27_pinmux/state", "default"); // Reset
                WriteValue2File("/sys/devices/platform/ocp/ocp:P9_29_pinmux/state", "default");
                WriteValue2File("/sys/devices/platform/ocp/ocp:P9_30_pinmux/state", "default");
                WriteValue2File("/sys/devices/platform/ocp/ocp:P9_31_pinmux/state", "default"); // Clk
                break;
        }
    }

    exit(ret);
}

/*=========================================================================*/
// returns the index of the first argument that is not an option; i.e.
// does not start with a dash or a slash
static int HandleOptions(int argc, char *argv[])
{
    int i,j,firstnonoption=0;

    for (i=1; i< argc;i++) {
        if (argv[i][0] == '/' || argv[i][0] == '-') {
            switch (argv[i][1]) {
                // An argument -? means help is requested
                case '?':
                    Usage(argv[0]);
                    CleanAndExit(0);
                    break;
                case 'h':
                case 'H':
                    if (!strcmp(argv[i]+1,"help")) {
                        Usage(argv[0]);
                        CleanAndExit(0);
                    }
                    else {
                        Usage(argv[0]);
                        CleanAndExit(0);
                    }
                    break;
                case 'b':
                    lk_board = MBOARD_BEAGLE;
                    break;
                case 'c':
                    lk_board = MBOARD_CV;
                    break;
                case 'v':
                    lk_verbose = atoi(argv[i]+2);
                    break;
                case 't':
                    lk_isTestOnly = TRUE;
                    break;
                default:
                    printf("unknown option %s\n", argv[i]);
                    CleanAndExit(255);
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
static int GPIOWrite(int pin, int value)
{
    static const char s_values_str[] = "01";

#define VALUE_MAX 30
    char path[VALUE_MAX];
    int fd;

    if ((value != 0) && (value != 1))
        return -1;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        PrintfErrAndExit("Failed to open gpio%d value for writing!\n", pin);
        return(-1);
    }

    if (1 != write(fd, &s_values_str[value], 1)) {
        PrintfErrAndExit("Failed to write value %c on gpio%d!\n", s_values_str[value], pin);
        return(-1);
    }

    close(fd);

    return(0);
}

/*=========================================================================*/
static void WriteValue2File(char* pcFile, char* pcValue)
{
    int fd;
    unsigned int uiLen;

    if (NULL==pcFile || NULL==pcValue) {
        PrintfErrAndExit("NULL Pointer error!\n");
    }

    fd = open(pcFile, O_WRONLY);
    if (-1 == fd) {
        PrintfErrAndExit("Failed to open %s!\n", pcFile);
    }
    uiLen = strlen(pcValue);
    if (uiLen != write(fd, pcValue, uiLen)) {
        PrintfErrAndExit("Failed to write value %s in %s!\n", pcValue, pcFile);
    }
    close(fd);
}

/*=========================================================================*/
static void PrintfErrorAndExitReal(unsigned int uiLine, const char* fmt, ...)
{
    va_list arglist;

    va_start(arglist,fmt);
    fprintf(stderr, "%s, %u: ", __FILE__, uiLine);
    vfprintf(stderr, fmt, arglist);
    va_end(arglist);

    CleanAndExit(-1);
}

/*=========================================================================*/
static VC_BOOL SPI_SendRecv(int fd, uint8_t* tx, uint8_t* rx, size_t len)
{
        int ret;
        int out_fd;
        memset(rx, 0, len);
        struct spi_ioc_transfer tr = {
                .tx_buf = (unsigned long)tx,
                .rx_buf = (unsigned long)rx,
                .len = len,
                .delay_usecs = 0,
                .speed_hz = 0,
                .bits_per_word = 0,
        };

        ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
        if (ret < 1)
            return VC_FALSE;
        return VC_TRUE;
}

/*=========================================================================*/
/*
static void do_msg(int fd, uint8_t const *tx, uint8_t const *rx, size_t len)
{
        struct spi_ioc_SPI_SendRecv xfer[2];
        int                     status;

        memset(xfer, 0, sizeof xfer);

        xfer[0].tx_buf = (unsigned long)tx;
        xfer[0].len = len;

        xfer[1].rx_buf = (unsigned long)rx;
        xfer[1].len = len;

        status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
        if (status < 0) {
            PrintfErrAndExit("can't send spi message\n");
        }
}
*/

/*=========================================================================*/
static void ResetAtmega(void)
{
    switch (lk_board)
    {
        case MBOARD_CV:
            WriteValue2File("/sys/devices/platform/ocp/ocp:ECAP0_PWM_pinmux/state", "gpio");
            GPIOWrite(7, 0);  // SCK Singal -> 0
            GPIOWrite(43, 0); // Clr Reset
            usleep(1000);
            GPIOWrite(43, 1); // Set Reset
            usleep(1000);
            GPIOWrite(43, 0); // Clr Reset
            usleep(20*1000);
            WriteValue2File("/sys/devices/platform/ocp/ocp:ECAP0_PWM_pinmux/state", "spi1_sclk");
            break;

        case MBOARD_BEAGLE:
            WriteValue2File("/sys/devices/platform/ocp/ocp:P9_31_pinmux/state", "gpio"); // Clk
            GPIOWrite(110, 0);  // SCK Singal -> 0
            GPIOWrite(115, 0); // Clr Reset
            usleep(1000);
            GPIOWrite(115, 1); // Set Reset
            usleep(1000);
            GPIOWrite(115, 0); // Clr Reset
            usleep(20*1000);
            WriteValue2File("/sys/devices/platform/ocp/ocp:P9_31_pinmux/state", "spi");
            break;
    }
}

/*=========================================================================*/
static VC_BOOL SPI_WriteAtmelFuses(int fd, TYAtmelDevice *pObjAtmelDev)
{
    VC_BOOL bRet = VC_TRUE;
    unsigned char sndBuf[ATMEGA_SIZE_INSTRUCTION];
    char rcvBuf[ATMEGA_SIZE_INSTRUCTION];

    if (NULL != pObjAtmelDev)
    {
        if( RET_OK == Atmega_GetInstruction(pObjAtmelDev->deviceType, ATMEL_WRITE_FUSEBYTE, sndBuf))
        {
#ifdef LINUX_SUPPORT
            sndBuf[BYTE4] = pObjAtmelDev->fuseByteLow;
#else
            sndBuf[BYTE4] = gl_aucMirrorByte[pObjAtmelDev->fuseByteLow];
#endif
            if (lk_verbose >= 2)
                printf("WrFuseBitsLowSnd: %x %x %x %x\n", sndBuf[0], sndBuf[1], sndBuf[2], sndBuf[3]);
            if (!SPI_SendRecv(fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                bRet = VC_FALSE;
            if (lk_verbose >= 1)
                printf("WrFuseBitsLowRcv: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]);
            VCHAL_msDelay(6);
        }
        else
            bRet = VC_FALSE;

        if (RET_OK == Atmega_GetInstruction(pObjAtmelDev->deviceType, ATMEL_WRITE_FUSEBYTE_HIGH, sndBuf))
        {
#ifdef LINUX_SUPPORT
            sndBuf[BYTE4] = pObjAtmelDev->fuseByteHigh;
#else
            sndBuf[BYTE4] = gl_aucMirrorByte[pObjAtmelDev->fuseByteHigh];
#endif
            if (lk_verbose >= 2)
                printf("WrFuseBitsHighSnd: %x %x %x %x\n", sndBuf[0], sndBuf[1], sndBuf[2], sndBuf[3]);
            if (!SPI_SendRecv(fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                bRet = VC_FALSE;
            if (lk_verbose >= 1)
                printf("WrFuseBitsHighRcv: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]);
            VCHAL_msDelay(6);
        }
        else
            bRet = VC_FALSE;

        if( ATMEGA64 == pObjAtmelDev->deviceType )
        {
            if (RET_OK == Atmega_GetInstruction(pObjAtmelDev->deviceType, ATMEL_WRITE_FUSEBYTE_EXTD, sndBuf))
            {
#ifdef LINUX_SUPPORT
                sndBuf[BYTE4] = pObjAtmelDev->fuseByteExtd;
#else
                sndBuf[BYTE4] = gl_aucMirrorByte[pObjAtmelDev->fuseByteExtd];
#endif
                if (lk_verbose >= 2)
                    printf("WrFuseBitsExtSnd: %x %x %x %x\n", sndBuf[0], sndBuf[1], sndBuf[2], sndBuf[3]);
                if (!SPI_SendRecv(fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                    bRet = VC_FALSE;
                if (lk_verbose >= 1)
                    printf("WrFuseBitsExtRcv: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]);
                VCHAL_msDelay(6);
            }
        }
        else
            bRet = VC_FALSE;
    }
    else
        bRet = VC_FALSE;

    return bRet;
}

/*=========================================================================*/
static VC_BOOL SPI_ReadAndCheckAtmelFuses(int fd, TYAtmelDevice *pObjAtmelDev)
{
    VC_BOOL bRet = VC_TRUE;
    unsigned char sndBuf[ATMEGA_SIZE_INSTRUCTION];
    char rcvBuf[ATMEGA_SIZE_INSTRUCTION];

    if (NULL != pObjAtmelDev)
    {
        if( RET_OK == Atmega_GetInstruction(pObjAtmelDev->deviceType, ATMEL_READ_FUSEBYTE, sndBuf))
        {
            if (lk_verbose >= 2)
                printf("RdFuseBitsLowSnd: %x %x %x %x\n", sndBuf[0], sndBuf[1], sndBuf[2], sndBuf[3]);
            if (!SPI_SendRecv(fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                bRet = VC_FALSE;
            if (lk_verbose >= 1)
                printf("RdFuseBitsLowRcv: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]);
            if (rcvBuf[3] != pObjAtmelDev->fuseByteLow) {
                printf("Error read fuses low: %x instead %x\n", rcvBuf[3], pObjAtmelDev->fuseByteLow);
                bRet = VC_FALSE;
            }
        }
        else
            bRet = VC_FALSE;

        if (RET_OK == Atmega_GetInstruction(pObjAtmelDev->deviceType, ATMEL_READ_FUSEBYTE_HIGH, sndBuf))
        {
            if (lk_verbose >= 2)
                printf("RdFuseBitsHighSnd: %x %x %x %x\n", sndBuf[0], sndBuf[1], sndBuf[2], sndBuf[3]);
            if (!SPI_SendRecv(fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                bRet = VC_FALSE;
            if (lk_verbose >= 1)
                printf("RdFuseBitsHighRcv: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]);
            if (rcvBuf[3] != pObjAtmelDev->fuseByteHigh) {
                printf("Error read fuses low: %x instead %x\n", rcvBuf[3], pObjAtmelDev->fuseByteHigh);
                bRet = VC_FALSE;
            }
        }
        else
            bRet = VC_FALSE;

        if( ATMEGA64 == pObjAtmelDev->deviceType )
        {
            if (RET_OK == Atmega_GetInstruction(pObjAtmelDev->deviceType, ATMEL_READ_FUSEBYTE_EXTD, sndBuf))
            {
                if (lk_verbose >= 2)
                    printf("RdFuseBitsExtSnd: %x %x %x %x\n", sndBuf[0], sndBuf[1], sndBuf[2], sndBuf[3]);
                if (!SPI_SendRecv(fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                    bRet = VC_FALSE;
                if (lk_verbose >= 1)
                    printf("RdFuseBitsExtRcv: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]);
                if (rcvBuf[3] != pObjAtmelDev->fuseByteExtd) {
                    printf("Error read fuses low: %x instead %x\n", rcvBuf[3], pObjAtmelDev->fuseByteExtd);
                    bRet = VC_FALSE;
                }
            }
        }
        else
            bRet = VC_FALSE;
    }
    else
        bRet = VC_FALSE;

    return bRet;
}

/*=========================================================================*/
int main(int argc, char *argv[])
{
    int ret = 0;
    int i;
    int fd;
    unsigned char instruction[ATMEGA_SIZE_INSTRUCTION];
    FILE* fUpdFile;
    char* pcUpdFile;
    char updFileBuf[130*2];
    uint32_t speed = 500000;
    uint32_t mode = 0;
    char buf[40];
    int bytesRead;
    char* pc;
    TYAtmelDevice* pObjAtmelDev;
    TYerrno errNo = RET_OK;

    unsigned char ucPageAddress = 0;
    unsigned int  uiPageWords = 0;

    char atmegaProgEnableSeq[]    = {0xAC, 0x53, 0x00, 0x00};

    char atmegaChipErase[]        = {0xAC, 0x80, 0x00, 0x00};

    char atmegaLoadProgMemPageLow[] = {0x40, 0x00, 0x00, 0x0C};
    char atmegaLoadProgMemPageHigh[]= {0x48, 0x00, 0x00, 0x94};
    char atmegaWriteProgMemPage[]   = {0x4C, 0x00, 0x00, 0x00};

    char atmegaReadProgMemPageLow[] = {0x20, 0x00, 0x00, 0x00};
    char atmegaReadProgMemPageHigh[]= {0x28, 0x00, 0x00, 0x00};


    if (argc == 1)
    {
        // No arguments
        Usage(argv[0]);
        CleanAndExit(0);
    }
    else
    {
        // handle the program options
        i = HandleOptions(argc, argv);
        printf("updFile: %s\n", argv[i]);
        pcUpdFile = argv[i];
    }

    errNo = Atmega_SelectCurrentAtmelDevice( &pObjAtmelDev, ATMEL_DISPLAY );
    if(RET_OK != errNo)
    {
        PrintfErrAndExit("error\n");
    }

    switch (lk_board)
    {
        case MBOARD_CV:
            WriteValue2File("/sys/devices/platform/ocp/ocp:UART0_CTSN_pinmux/state", "spi1");
            WriteValue2File("/sys/devices/platform/ocp/ocp:UART0_RTSN_pinmux/state", "spi1");
            WriteValue2File("/sys/devices/platform/ocp/ocp:ECAP0_PWM_pinmux/state", "spi1_sclk");
            break;

        case MBOARD_BEAGLE:
            WriteValue2File("/sys/devices/platform/ocp/ocp:P9_27_pinmux/state", "gpio"); // Reset
            WriteValue2File("/sys/devices/platform/ocp/ocp:P9_29_pinmux/state", "spi");
            WriteValue2File("/sys/devices/platform/ocp/ocp:P9_30_pinmux/state", "spi");
            WriteValue2File("/sys/devices/platform/ocp/ocp:P9_31_pinmux/state", "spi"); // Clk
            break;
    }
    lk_pinsAreSetToSPI = VC_TRUE;

    ResetAtmega();

    lk_fd = open(lk_spidev, O_RDWR);
    if (lk_fd<=0)
        PrintfErrAndExit("Can't open device %s!\n", lk_spidev);

    ret = ioctl(lk_fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        PrintfErrAndExit("Can't set spi mode on dev %s", lk_spidev);
    ret = ioctl(lk_fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        PrintfErrAndExit("Can't set spi mode on dev %s", lk_spidev);

    ret = ioctl(lk_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        PrintfErrAndExit("Can't set max speed of %u hz on device %s", speed, lk_spidev);

    SPI_SendRecv(lk_fd, atmegaProgEnableSeq, buf, ARRAY_SIZE(atmegaProgEnableSeq));
    if (lk_verbose >= 1)
        printf("ProgEnSeq: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

    if (!SPI_WriteAtmelFuses(lk_fd, pObjAtmelDev))
        PrintfErrAndExit("Error writing fuses!\n");

    if (!SPI_ReadAndCheckAtmelFuses(lk_fd, pObjAtmelDev))
        PrintfErrAndExit("Error checking fuses!\n");

    if (lk_isTestOnly)
    {
        unsigned char sndBuf[ATMEGA_SIZE_INSTRUCTION];
        char rcvBuf[ATMEGA_SIZE_INSTRUCTION];

        SPI_SendRecv(lk_fd, atmegaReadProgMemPageLow, buf, ARRAY_SIZE(atmegaReadProgMemPageLow));
        printf("atmegaReadProgMemPageLow: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

        SPI_SendRecv(lk_fd, atmegaReadProgMemPageHigh, buf, ARRAY_SIZE(atmegaReadProgMemPageHigh));
        printf("atmegaReadProgMemPageHigh: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

        // Chip Erase
        SPI_SendRecv(lk_fd, atmegaChipErase, buf, ARRAY_SIZE(atmegaChipErase));
        printf("atmegaChipErase: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3], buf[4]);

        usleep(100*1000);

        SPI_SendRecv(lk_fd, atmegaReadProgMemPageLow, buf, ARRAY_SIZE(atmegaReadProgMemPageLow));
        printf("atmegaReadProgMemPageLow: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

        SPI_SendRecv(lk_fd, atmegaReadProgMemPageHigh, buf, ARRAY_SIZE(atmegaReadProgMemPageHigh));
        printf("atmegaReadProgMemPageHigh: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

        ucPageAddress = 0;
        i = 0;

        pc = atmegaLoadProgMemPageLow;
        pc[2] = ((unsigned char*)&i)[0];
        pc[3] = 0x0C;
        // printf("%x %x %x %x\n", pc[0], pc[1], pc[2], pc[3]);
        SPI_SendRecv(lk_fd, atmegaLoadProgMemPageLow, buf, ARRAY_SIZE(atmegaLoadProgMemPageLow));
        printf("atmegaLoadProgMemPageLow: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

        pc = atmegaLoadProgMemPageHigh;
        pc[2] = ((unsigned char*)&i)[0];
        pc[3] = 0x94;
        // printf("%x %x %x %x\n", pc[0], pc[1], pc[2], pc[3]);
        SPI_SendRecv(lk_fd, atmegaLoadProgMemPageHigh, buf, ARRAY_SIZE(atmegaLoadProgMemPageHigh));
        printf("atmegaLoadProgMemPageHigh: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

        pc = atmegaWriteProgMemPage;
        atmegaWriteProgMemPage[1] = (ucPageAddress >> 1) & 0x7F;
        atmegaWriteProgMemPage[2] = ((ucPageAddress & 0x01) << 7) & 0x80;
        // printf("%x %x %x %x\n", pc[0], pc[1], pc[2], pc[3]);
        SPI_SendRecv(lk_fd, atmegaWriteProgMemPage, buf, ARRAY_SIZE(atmegaWriteProgMemPage));
        printf("atmegaWriteProgMemPage: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

        usleep(4500);

        SPI_SendRecv(lk_fd, atmegaReadProgMemPageLow, buf, ARRAY_SIZE(atmegaReadProgMemPageLow));
        printf("atmegaReadProgMemPageLow: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

        SPI_SendRecv(lk_fd, atmegaReadProgMemPageHigh, buf, ARRAY_SIZE(atmegaReadProgMemPageHigh));
        printf("atmegaReadProgMemPageHigh: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

        CleanAndExit(0);
    }

    fUpdFile = fopen(pcUpdFile, "rb");
    if (fUpdFile == NULL)
    {
        PrintfErrAndExit("Failed to open file %s\n", pcUpdFile);
    }

    // Chip Erase
    SPI_SendRecv(lk_fd, atmegaChipErase, buf, ARRAY_SIZE(atmegaChipErase));
    printf("atmegaChipErase: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3], buf[4]);

    usleep(100*1000);

    ucPageAddress = 0;
    i = 0;

    do
    {
        bytesRead = fread(updFileBuf, 1, 128*2, fUpdFile);
        if (bytesRead <= 0)
            break;

        for(uiPageWords=0; uiPageWords<bytesRead/2; uiPageWords++)
        {
            pc = atmegaLoadProgMemPageLow;
            pc[2] = ((unsigned char*)&uiPageWords)[0];
            pc[3] = updFileBuf[uiPageWords*2];
            // printf("%x %x %x %x\n", pc[0], pc[1], pc[2], pc[3]);
            SPI_SendRecv(lk_fd, atmegaLoadProgMemPageLow, buf, ARRAY_SIZE(atmegaLoadProgMemPageLow));
            // printf("ProgLow: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);

            pc = atmegaLoadProgMemPageHigh;
            pc[2] = ((unsigned char*)&uiPageWords)[0];
            pc[3] = updFileBuf[uiPageWords*2+1];
            // printf("%x %x %x %x\n", pc[0], pc[1], pc[2], pc[3]);
            SPI_SendRecv(lk_fd, atmegaLoadProgMemPageHigh, buf, ARRAY_SIZE(atmegaLoadProgMemPageHigh));
            // printf("ProgHigh: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);
        }

        pc = atmegaWriteProgMemPage;
        atmegaWriteProgMemPage[1] = (ucPageAddress >> 1) & 0x7F;
        atmegaWriteProgMemPage[2] = ((ucPageAddress & 0x01) << 7) & 0x80;
        // printf("%x %x %x %x\n", pc[0], pc[1], pc[2], pc[3]);
        SPI_SendRecv(lk_fd, atmegaWriteProgMemPage, buf, ARRAY_SIZE(atmegaWriteProgMemPage));
        // printf("atmegaWriteProgMemPage: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);
        usleep(4500);

        for(uiPageWords=0; uiPageWords<bytesRead/2; uiPageWords++)
        {
            pc = atmegaReadProgMemPageLow;
            pc[1] = atmegaWriteProgMemPage[1];
            pc[2] = atmegaWriteProgMemPage[2];
            pc[2] |= ((unsigned char*)&uiPageWords)[0];
            SPI_SendRecv(lk_fd, atmegaReadProgMemPageLow, buf, ARRAY_SIZE(atmegaReadProgMemPageLow));
            // printf("ReadLow: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);
            if (buf[3] != updFileBuf[uiPageWords*2])
            {
                PrintfErrAndExit("    Error at %u: %x instead %x\n", uiPageWords, buf[3], updFileBuf[uiPageWords*2]);
            }

            pc = atmegaReadProgMemPageHigh;
            pc[1] = atmegaWriteProgMemPage[1];
            pc[2] = atmegaWriteProgMemPage[2];
            pc[2] |= ((unsigned char*)&uiPageWords)[0];
            SPI_SendRecv(lk_fd, atmegaReadProgMemPageHigh, buf, ARRAY_SIZE(atmegaReadProgMemPageHigh));
            // printf("ReadHigh: %x %x %x %x\n", buf[0], buf[1], buf[2], buf[3]);
            if (buf[3] != updFileBuf[uiPageWords*2+1])
            {
                PrintfErrAndExit("    Error at %u: %x instead %x\n", uiPageWords, buf[3], updFileBuf[uiPageWords*2+1]);
            }
        }

        ucPageAddress++;

    } while (bytesRead > 0);
//    } while (0);

    fclose(fUpdFile);

    CleanAndExit(0);
}
