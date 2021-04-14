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

#define VERSION$ "1.0"

#define SpiDev_LogErrAndReturn(fmt, ...)  PrintfErrorAndExitReal(__LINE__, fmt, ##__VA_ARGS__)

#define SpiDev_Atmega_GetInstruct(IDATMEL, IDINSTRUCT, PINSTRUCT) \
    if (RET_OK != Atmega_GetInstruction(IDATMEL, IDINSTRUCT, PINSTRUCT)) \
    {                                                                    \
        PrintfErrorAndExitReal(__LINE__, "Get Instruction error!\n"); \
        return VC_FALSE;                                                          \
    }

#define CV_LOG_A(ARGS) \
            if (lk_verbose >= 1) \
                printf ARGS
#define CV_LOG_A_MORE(ARGS) \
            if (lk_verbose >= 2) \
                printf ARGS

#define HEAD_SIZE 148

/*--------------------------- Typdeklarationen ----------------------------*/

typedef enum {
    MBOARD_BEAGLE,
    MBOARD_CV,
} MBOARD;

/*------------------------------ Prototypen -------------------------------*/

static void PrintfErrorAndExitReal(unsigned int uiLine, const char* fmt, ...);
static int SysFs_GPIOWrite(int pin, int value);
static void SysFs_WriteValue2File(char* pcFile, char* pcValue);

/*------------------------ Konstantendeklarationen ------------------------*/

static const char* lk_spidev = "/dev/spidev1.1";
static const AtmelID lk_idATMEL = ATMEGA64;
#define ATMEGA_BYTES_PER_PAGE 128*2
static const uint32_t lk_speed = 750000;

/*------------------------- modulglobale Variable -------------------------*/

static int lk_fd = -1;
static BOOL   lk_pinsAreSetToSPI;
static MBOARD lk_board = MBOARD_CV;
static int lk_verbose;
static BOOL   lk_isTestOnly;
static BOOL   lk_isVerify;
static BOOL   lk_isVerifyOnly;

static char lk_buf4CompletePageWithInstructions[ATMEGA_BYTES_PER_PAGE*ATMEGA_SIZE_INSTRUCTION+ATMEGA_SIZE_INSTRUCTION];

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
            SysFs_GPIOWrite(43, 1); // Set Reset
            break;
        case MBOARD_BEAGLE:
            SysFs_GPIOWrite(115, 1); // Set Reset
            break;
    }

    if (lk_pinsAreSetToSPI)
    {
        switch (lk_board)
        {
            case MBOARD_CV:
                SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:UART0_CTSN_pinmux/state", "default");
                SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:UART0_RTSN_pinmux/state", "default");
                SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:ECAP0_PWM_pinmux/state", "default");
                break;

            case MBOARD_BEAGLE:
                SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_27_pinmux/state", "default"); // Reset
                SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_29_pinmux/state", "default");
                SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_30_pinmux/state", "default");
                SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_31_pinmux/state", "default"); // Clk
                break;
        }
    }

    exit(ret);
}

/*=========================================================================*/
static int EndsWith(const char *str, const char *suffix)
{
    if (!str || !suffix)
        return 0;
    size_t lenstr = strlen(str);
    size_t lensuffix = strlen(suffix);
    if (lensuffix >  lenstr)
        return 0;
    return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

/*=========================================================================*/
// returns the index of the first argument that is not an option; i.e.
// does not start with a dash or a slash
static int HandleOptions(int argc, char *argv[])
{
    int i,j,firstnonoption=0;

    for (i=1; i< argc;i++) {

        if ( EndsWith(argv[i],".bin") || EndsWith(argv[i],".atd") ) {
            firstnonoption = i;
            break;
        }

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
                case 'm':
                    lk_isVerify = TRUE;
                    break;
                case 'n':
                    lk_isVerifyOnly = TRUE;
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
static int SysFs_GPIOWrite(int pin, int value)
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
        SpiDev_LogErrAndReturn("Failed to open gpio%d value for writing!\n", pin);
        return(-1);
    }

    if (1 != write(fd, &s_values_str[value], 1)) {
        SpiDev_LogErrAndReturn("Failed to write value %c on gpio%d!\n", s_values_str[value], pin);
        return(-1);
    }

    close(fd);

    return(0);
}

/*=========================================================================*/
static void SysFs_WriteValue2File(char* pcFile, char* pcValue)
{
    int fd;
    unsigned int uiLen;

    if (NULL==pcFile || NULL==pcValue) {
        SpiDev_LogErrAndReturn("NULL Pointer error!\n");
    }

    fd = open(pcFile, O_WRONLY);
    if (-1 == fd) {
        SpiDev_LogErrAndReturn("Failed to open %s!\n", pcFile);
    }
    uiLen = strlen(pcValue);
    if (uiLen != write(fd, pcValue, uiLen)) {
        SpiDev_LogErrAndReturn("Failed to write value %s in %s!\n", pcValue, pcFile);
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
static VC_BOOL SpiDev_SendRecv(int fd, uint8_t* tx, uint8_t* rx, size_t len)
{
        int ret;
        if (rx != NULL)
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
        struct spi_ioc_SpiDev_SendRecv xfer[2];
        int                     status;

        memset(xfer, 0, sizeof xfer);

        xfer[0].tx_buf = (unsigned long)tx;
        xfer[0].len = len;

        xfer[1].rx_buf = (unsigned long)rx;
        xfer[1].len = len;

        status = ioctl(fd, SPI_IOC_MESSAGE(2), xfer);
        if (status < 0) {
            SpiDev_LogErrAndReturn("can't send spi message\n");
        }
}
*/

/*=========================================================================*/
static void SpiDev_ResetAtmega(void)
{
    switch (lk_board)
    {
        case MBOARD_CV:
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:ECAP0_PWM_pinmux/state", "default");
            SysFs_GPIOWrite(7, 0);  // SCK Singal -> 0
            SysFs_GPIOWrite(43, 0); // Clr Reset
            usleep(1000);
            SysFs_GPIOWrite(43, 1); // Set Reset
            usleep(1000);
            SysFs_GPIOWrite(43, 0); // Clr Reset
            usleep(20*1000);
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:ECAP0_PWM_pinmux/state", "spi1_sclk");
            break;

        case MBOARD_BEAGLE:
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_31_pinmux/state", "gpio"); // Clk
            SysFs_GPIOWrite(110, 0);  // SCK Singal -> 0
            SysFs_GPIOWrite(115, 0); // Clr Reset
            usleep(1000);
            SysFs_GPIOWrite(115, 1); // Set Reset
            usleep(1000);
            SysFs_GPIOWrite(115, 0); // Clr Reset
            usleep(20*1000);
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_31_pinmux/state", "spi");
            break;
    }
}

/*=========================================================================*/
static VC_BOOL SpiDev_WriteAtmegaFuses(int fd, TYAtmelDevice *pObjAtmelDev)
{
    VC_BOOL bRet = VC_TRUE;
    unsigned char sndBuf[ATMEGA_SIZE_INSTRUCTION];
    char rcvBuf[ATMEGA_SIZE_INSTRUCTION];

    if (NULL != pObjAtmelDev)
    {
        if (RET_OK == Atmega_GetInstruction(pObjAtmelDev->deviceType, ATMEL_WRITE_FUSEBYTE, sndBuf))
        {
#ifdef LINUX_SUPPORT
            sndBuf[BYTE4] = pObjAtmelDev->fuseByteLow;
#else
            sndBuf[BYTE4] = gl_aucMirrorByte[pObjAtmelDev->fuseByteLow];
#endif
            CV_LOG_A_MORE(("WrFuseBitsLowSnd: %x %x %x %x\n", sndBuf[0], sndBuf[1], sndBuf[2], sndBuf[3]));
            if (!SpiDev_SendRecv(fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                bRet = VC_FALSE;
            CV_LOG_A(("WrFuseBitsLowRcv: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]));
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
            CV_LOG_A_MORE(("WrFuseBitsHighSnd: %x %x %x %x\n", sndBuf[0], sndBuf[1], sndBuf[2], sndBuf[3]));
            if (!SpiDev_SendRecv(fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                bRet = VC_FALSE;
            CV_LOG_A(("WrFuseBitsHighRcv: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]));
            VCHAL_msDelay(6);
        }
        else
            bRet = VC_FALSE;

        if (ATMEGA64 == pObjAtmelDev->deviceType)
        {
            if (RET_OK == Atmega_GetInstruction(pObjAtmelDev->deviceType, ATMEL_WRITE_FUSEBYTE_EXTD, sndBuf))
            {
#ifdef LINUX_SUPPORT
                sndBuf[BYTE4] = pObjAtmelDev->fuseByteExtd;
#else
                sndBuf[BYTE4] = gl_aucMirrorByte[pObjAtmelDev->fuseByteExtd];
#endif
                CV_LOG_A_MORE(("WrFuseBitsExtSnd: %x %x %x %x\n", sndBuf[0], sndBuf[1], sndBuf[2], sndBuf[3]));
                if (!SpiDev_SendRecv(fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                    bRet = VC_FALSE;
                CV_LOG_A(("WrFuseBitsExtRcv: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]));
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
static VC_BOOL SpiDev_ReadAndCheckAtmegaFuses(int fd, TYAtmelDevice *pObjAtmelDev)
{
    VC_BOOL bRet = VC_TRUE;
    unsigned char sndBuf[ATMEGA_SIZE_INSTRUCTION];
    char rcvBuf[ATMEGA_SIZE_INSTRUCTION];

    if (NULL != pObjAtmelDev)
    {
        if (RET_OK == Atmega_GetInstruction(pObjAtmelDev->deviceType, ATMEL_READ_FUSEBYTE, sndBuf))
        {
            CV_LOG_A_MORE(("RdFuseBitsLowSnd: %x %x %x %x\n", sndBuf[0], sndBuf[1], sndBuf[2], sndBuf[3]));
            if (!SpiDev_SendRecv(fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                bRet = VC_FALSE;
            CV_LOG_A(("RdFuseBitsLowRcv: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]));
            if (rcvBuf[3] != pObjAtmelDev->fuseByteLow) {
                printf("Error read fuses low: %x instead %x\n", rcvBuf[3], pObjAtmelDev->fuseByteLow);
                bRet = VC_FALSE;
            }
        }
        else
            bRet = VC_FALSE;

        if (RET_OK == Atmega_GetInstruction(pObjAtmelDev->deviceType, ATMEL_READ_FUSEBYTE_HIGH, sndBuf))
        {
            CV_LOG_A_MORE(("RdFuseBitsHighSnd: %x %x %x %x\n", sndBuf[0], sndBuf[1], sndBuf[2], sndBuf[3]));
            if (!SpiDev_SendRecv(fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                bRet = VC_FALSE;
            CV_LOG_A(("RdFuseBitsHighRcv: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]));
            if (rcvBuf[3] != pObjAtmelDev->fuseByteHigh) {
                printf("Error read fuses low: %x instead %x\n", rcvBuf[3], pObjAtmelDev->fuseByteHigh);
                bRet = VC_FALSE;
            }
        }
        else
            bRet = VC_FALSE;

        if (ATMEGA64 == pObjAtmelDev->deviceType)
        {
            if (RET_OK == Atmega_GetInstruction(pObjAtmelDev->deviceType, ATMEL_READ_FUSEBYTE_EXTD, sndBuf))
            {
                CV_LOG_A_MORE(("RdFuseBitsExtSnd: %x %x %x %x\n", sndBuf[0], sndBuf[1], sndBuf[2], sndBuf[3]));
                if (!SpiDev_SendRecv(fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                    bRet = VC_FALSE;
                CV_LOG_A(("RdFuseBitsExtRcv: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]));
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
static void SPI_DoReadCmd(int fd, TYAtmelDevice *pObjAtmelDev, unsigned char idInstruction)
{
    unsigned char sndBuf[ATMEGA_SIZE_INSTRUCTION];
    char rcvBuf[ATMEGA_SIZE_INSTRUCTION];

    if (NULL != pObjAtmelDev)
    {
        if (RET_OK == Atmega_GetInstruction(pObjAtmelDev->deviceType, idInstruction, sndBuf))
        {
            if (!SpiDev_SendRecv(fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                SpiDev_LogErrAndReturn("Error writing prog enable!\n");
            if (idInstruction == ATMEL_READ_CMD_LOW)
                printf("RdLow:  %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]);
            else if (idInstruction == ATMEL_READ_CMD_HIGH)
                printf("RdHigh: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]);
            else
                printf("Inst %u: %x %x %x %x\n", idInstruction, rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]);
        }
        else
            SpiDev_LogErrAndReturn("Get Instruction error!\n");
    }
    else
        SpiDev_LogErrAndReturn("NULL-pointer error!\n");
}

/*=========================================================================*/
int main(int argc, char *argv[])
{
    int ret = 0;
    int i;
    unsigned char sndBuf[ATMEGA_SIZE_INSTRUCTION];
    char rcvBuf[ATMEGA_SIZE_INSTRUCTION];
    FILE* fUpdFile;
    char* pcUpdFile;
    char updFileBuf[ATMEGA_BYTES_PER_PAGE+20];
    uint32_t mode = 0;
    int bytesRead;
    TYAtmelDevice* pObjAtmelDev;
    TYerrno errNo = RET_OK;

    unsigned char ucPageAddress = 0;
    unsigned int  uiPageWords = 0;

    if (argc == 1)
    {
        // No arguments
        printf("wrong Args\n");
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
        SpiDev_LogErrAndReturn("error\n");
    }

    switch (lk_board)
    {
        case MBOARD_CV:
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:UART0_CTSN_pinmux/state", "spi1");
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:UART0_RTSN_pinmux/state", "spi1");
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:ECAP0_PWM_pinmux/state", "spi1_sclk");
            break;

        case MBOARD_BEAGLE:
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_27_pinmux/state", "gpio"); // Reset
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_29_pinmux/state", "spi");
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_30_pinmux/state", "spi");
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_31_pinmux/state", "spi"); // Clk
            break;
    }
    lk_pinsAreSetToSPI = VC_TRUE;

    SpiDev_ResetAtmega();

    lk_fd = open(lk_spidev, O_RDWR);
    if (lk_fd<=0)
        SpiDev_LogErrAndReturn("Can't open device %s!\n", lk_spidev);

    ret = ioctl(lk_fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        SpiDev_LogErrAndReturn("Can't set spi mode on dev %s", lk_spidev);
    ret = ioctl(lk_fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        SpiDev_LogErrAndReturn("Can't set spi mode on dev %s", lk_spidev);

    ret = ioctl(lk_fd, SPI_IOC_WR_MAX_SPEED_HZ, &lk_speed);
    if (ret == -1)
        SpiDev_LogErrAndReturn("Can't set max lk_speed of %u hz on device %s", lk_speed, lk_spidev);

    // Start Programming
    SpiDev_Atmega_GetInstruct(pObjAtmelDev->deviceType, ATMEL_PROG_ENABLE, sndBuf);
    {
        if (!SpiDev_SendRecv(lk_fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
            SpiDev_LogErrAndReturn("Error writing prog enable!\n");
        CV_LOG_A(("ProgEnSeq: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]));
        if (lk_isVerify || lk_isVerifyOnly) {
            if (rcvBuf[2] != 0x53) {
                printf("Error ProgEnSeq: %x instead %x\n", rcvBuf[2], 0x53);
            }
        }
    }

    if (!lk_isVerifyOnly) {
        if (!SpiDev_WriteAtmegaFuses(lk_fd, pObjAtmelDev))
            SpiDev_LogErrAndReturn("Error writing fuses!\n");
    }

    if (lk_isVerify || lk_isVerifyOnly) {
        if (!SpiDev_ReadAndCheckAtmegaFuses(lk_fd, pObjAtmelDev))
            SpiDev_LogErrAndReturn("Error checking fuses!\n");
    }

    if (lk_isTestOnly)
    {
        SPI_DoReadCmd(lk_fd, pObjAtmelDev, ATMEL_READ_CMD_LOW);
        SPI_DoReadCmd(lk_fd, pObjAtmelDev, ATMEL_READ_CMD_HIGH);

        // Chip Erase
        SpiDev_Atmega_GetInstruct(pObjAtmelDev->deviceType, ATMEL_CHIP_ERASE, sndBuf);
        {
            if (!SpiDev_SendRecv(lk_fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                SpiDev_LogErrAndReturn("Error chip erase!\n");
            printf("Erase: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]);
        }
        VCHAL_msDelay(20);

        SPI_DoReadCmd(lk_fd, pObjAtmelDev, ATMEL_READ_CMD_LOW);
        SPI_DoReadCmd(lk_fd, pObjAtmelDev, ATMEL_READ_CMD_HIGH);

        ucPageAddress = 0;
        i = 0;

        // Write Page
        SpiDev_Atmega_GetInstruct(pObjAtmelDev->deviceType, ATMEL_LOAD_CMD_LOW, sndBuf);
        {
            sndBuf[BYTE3] = ((unsigned char*)&i)[0];
            sndBuf[BYTE4] = 0x0C;
            if (!SpiDev_SendRecv(lk_fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                SpiDev_LogErrAndReturn("Error load low!\n");
            printf("WrLow:  %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]);
        }

        SpiDev_Atmega_GetInstruct(pObjAtmelDev->deviceType, ATMEL_LOAD_CMD_HIGH, sndBuf);
        {
            sndBuf[BYTE3] = ((unsigned char*)&i)[0];
            sndBuf[BYTE4] = 0x94;
            if (!SpiDev_SendRecv(lk_fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                SpiDev_LogErrAndReturn("Error load high!\n");
            printf("WrHigh: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]);
        }

        SpiDev_Atmega_GetInstruct(pObjAtmelDev->deviceType, ATMEL_WRITE_PROG_MEM_PAGE, sndBuf);
        {
            sndBuf[BYTE2] = (ucPageAddress >> 1) & 0x7F;
            sndBuf[BYTE3] = ((ucPageAddress & 0x01) << 7) & 0x80;
            if (!SpiDev_SendRecv(lk_fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                SpiDev_LogErrAndReturn("Error write page!\n");
            printf("WrPage: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]);
        }

        VCHAL_msDelay(5);

        SPI_DoReadCmd(lk_fd, pObjAtmelDev, ATMEL_READ_CMD_LOW);
        SPI_DoReadCmd(lk_fd, pObjAtmelDev, ATMEL_READ_CMD_HIGH);

        CleanAndExit(0);
    }

    if (!( EndsWith(pcUpdFile,".bin") || EndsWith(pcUpdFile,".atd") )) {
        SpiDev_LogErrAndReturn("File %s should end either with .bin or with .atd\n", pcUpdFile);
    }

    fUpdFile = fopen(pcUpdFile, "rb");
    if (fUpdFile == NULL)
    {
        SpiDev_LogErrAndReturn("Failed to open file %s\n", pcUpdFile);
    }

    // Chip Erase
    if (!lk_isVerifyOnly)
    {
        SpiDev_Atmega_GetInstruct(pObjAtmelDev->deviceType, ATMEL_CHIP_ERASE, sndBuf);
        {
            if (!SpiDev_SendRecv(lk_fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                SpiDev_LogErrAndReturn("Error chip erase!\n");
            CV_LOG_A(("Erase: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]));
        }
        VCHAL_msDelay(20);
    }

    if (EndsWith(pcUpdFile,".atd"))
    {
        ret = fseek(fUpdFile, HEAD_SIZE, SEEK_SET);
        if (ret != 0) {
            SpiDev_LogErrAndReturn("Failed to seek file %s\n", pcUpdFile);
        }
    }

    ucPageAddress = 0;

    do
    {
        bytesRead = fread(updFileBuf, 1, ATMEGA_BYTES_PER_PAGE, fUpdFile);
        if (bytesRead <= 0)
            break;

        if (!lk_isVerifyOnly)
        {
            if (lk_verbose >= 1)
            {
                for (uiPageWords=0; uiPageWords<bytesRead/2; uiPageWords++)
                {
                    SpiDev_Atmega_GetInstruct(pObjAtmelDev->deviceType, ATMEL_LOAD_CMD_LOW, sndBuf);
                    {
                        sndBuf[BYTE3] = ((unsigned char*)&uiPageWords)[0];
                        sndBuf[BYTE4] = updFileBuf[uiPageWords*2];
                        if (!SpiDev_SendRecv(lk_fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                            SpiDev_LogErrAndReturn("Error load low!\n");
                        CV_LOG_A(("WrLow:  %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]));
                    }

                    SpiDev_Atmega_GetInstruct(pObjAtmelDev->deviceType, ATMEL_LOAD_CMD_HIGH, sndBuf);
                    {
                        sndBuf[BYTE3] = ((unsigned char*)&uiPageWords)[0];
                        sndBuf[BYTE4] = updFileBuf[uiPageWords*2+1];
                        if (!SpiDev_SendRecv(lk_fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                            SpiDev_LogErrAndReturn("Error load high!\n");
                        CV_LOG_A(("WrHigh: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]));
                    }
                }

                SpiDev_Atmega_GetInstruct(pObjAtmelDev->deviceType, ATMEL_WRITE_PROG_MEM_PAGE, sndBuf);
                {
                    sndBuf[BYTE2] = (ucPageAddress >> 1) & 0x7F;
                    sndBuf[BYTE3] = ((ucPageAddress & 0x01) << 7) & 0x80;
                    if (!SpiDev_SendRecv(lk_fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                        SpiDev_LogErrAndReturn("Error write page!\n");
                    CV_LOG_A(("WrPage: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]));
                }
                VCHAL_msDelay(5);
            }
            else
            {
                VC_BOOL isModulo = VC_FALSE;
                char* pc = lk_buf4CompletePageWithInstructions;
                unsigned int uiPageWordsMax = bytesRead/2;
                if (bytesRead%2)
                {
                    uiPageWordsMax += 1;
                    isModulo = VC_TRUE;
                }

                for (uiPageWords=0; uiPageWords<uiPageWordsMax; uiPageWords++)
                {
                    SpiDev_Atmega_GetInstruct(pObjAtmelDev->deviceType, ATMEL_LOAD_CMD_LOW, pc);
                    {
                        pc[BYTE3] = ((unsigned char*)&uiPageWords)[0];
                        pc[BYTE4] = updFileBuf[uiPageWords*2];
                        pc += 4;
                    }
                    if (uiPageWords<uiPageWordsMax-1 || !isModulo) {
                        SpiDev_Atmega_GetInstruct(pObjAtmelDev->deviceType, ATMEL_LOAD_CMD_HIGH, pc);
                        {
                            pc[BYTE3] = ((unsigned char*)&uiPageWords)[0];
                            pc[BYTE4] = updFileBuf[uiPageWords*2+1];
                            pc += 4;
                        }
                    }
                }

                SpiDev_Atmega_GetInstruct(pObjAtmelDev->deviceType, ATMEL_WRITE_PROG_MEM_PAGE, pc);
                {
                    pc[BYTE2] = (ucPageAddress >> 1) & 0x7F;
                    pc[BYTE3] = ((ucPageAddress & 0x01) << 7) & 0x80;
                    if (!SpiDev_SendRecv(lk_fd, lk_buf4CompletePageWithInstructions, NULL,
                            bytesRead*ATMEGA_SIZE_INSTRUCTION+ATMEGA_SIZE_INSTRUCTION))

                        SpiDev_LogErrAndReturn("Error write page!\n");
                    CV_LOG_A(("WrPage: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]));
                }
                VCHAL_msDelay(5);
            }
        }

        if (lk_isVerify || lk_isVerifyOnly)
        {
            for(uiPageWords=0; uiPageWords<bytesRead/2; uiPageWords++)
            {
                SpiDev_Atmega_GetInstruct(pObjAtmelDev->deviceType, ATMEL_READ_CMD_LOW, sndBuf);
                {
                    sndBuf[BYTE2] = (ucPageAddress >> 1) & 0x7F;
                    sndBuf[BYTE3] = ((ucPageAddress & 0x01) << 7) & 0x80;
                    sndBuf[BYTE3] |= ((unsigned char*)&uiPageWords)[0];
                    if (!SpiDev_SendRecv(lk_fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                        SpiDev_LogErrAndReturn("Error read low!\n");
                    CV_LOG_A(("RdLow:  %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]));
                    if (rcvBuf[3] != updFileBuf[uiPageWords*2])
                    {
                        SpiDev_LogErrAndReturn("    Error at %u: %x instead %x\n", uiPageWords, rcvBuf[3], updFileBuf[uiPageWords*2]);
                    }
                }

                SpiDev_Atmega_GetInstruct(pObjAtmelDev->deviceType, ATMEL_READ_CMD_HIGH, sndBuf);
                {
                    sndBuf[BYTE2] = (ucPageAddress >> 1) & 0x7F;
                    sndBuf[BYTE3] = ((ucPageAddress & 0x01) << 7) & 0x80;
                    sndBuf[BYTE3] |= ((unsigned char*)&uiPageWords)[0];
                    if (!SpiDev_SendRecv(lk_fd, sndBuf, rcvBuf, ATMEGA_SIZE_INSTRUCTION))
                        SpiDev_LogErrAndReturn("Error read high!\n");
                    CV_LOG_A(("RdHigh: %x %x %x %x\n", rcvBuf[0], rcvBuf[1], rcvBuf[2], rcvBuf[3]));
                    if (rcvBuf[3] != updFileBuf[uiPageWords*2+1])
                    {
                        SpiDev_LogErrAndReturn("    Error at %u: %x instead %x\n", uiPageWords, rcvBuf[3], updFileBuf[uiPageWords*2+1]);
                    }
                }
            }
        }

        ucPageAddress++;

    } while (bytesRead > 0);

    fclose(fUpdFile);

    CleanAndExit(0);
}
