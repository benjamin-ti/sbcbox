/***************************************************************************/
/*                             Printerfirmware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/*                       http://www.carl-valentin.de                       */
/*                  This code is licenced under the LGPL                   */
/***************************************************************************/


/* IMPORT =================================================================*/

#include <stdio.h>  // Standard input/output definitions
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>  // File control definitions (O_BINARY)
#include <unistd.h>
#include <errno.h>   // Error number definitions
#include <termios.h> // POSIX terminal control definitions

typedef enum {FALSE, TRUE} BOOL;

/* ENDIMPORT ==============================================================*/


/* EXPORT =================================================================*/

BOOL gl_bDebug = FALSE;

/* ENDEXPORT ==============================================================*/


/* LOCAL ==================================================================*/

/*-------------------------------- macros ---------------------------------*/

#define VERSION$ "1.0"

#define DISPLAY_BUF_SIZE  60
#define SER_PORT_NAME_LEN 12
#define SER_ERR_TXT_LEN   60

/*--------------------------- type declaration ----------------------------*/

typedef enum {
    MBOARD_BEAGLE,
    MBOARD_CV,
} MBOARD;

/*------------------------------ prototypes -------------------------------*/

/*------------------------- constant declarations -------------------------*/

/*---------------------------- local variables ----------------------------*/

static MBOARD lk_board = MBOARD_CV;
static char lk_pcPort[SER_PORT_NAME_LEN];
unsigned long lk_ulBaudRate = 57600;

/* ENDLOCAL ===============================================================*/

/*=========================================================================*/
static void Usage(char *programName)
{
    //_ Programm-Name needs only to be translated if these names are
    //_ localized too!
    printf("Carl Valentin Display Print Tool %s\n\n", VERSION$);

    printf("%s usage:\n", programName);
    printf("%s <options> updFile\n\n", programName);
    printf("Program must be able to modify pin functionality over pinmux-helper\n");
    printf("GPIOs must be exported and configured (dir) for sysfs use\n\n");
    printf("Options:\n");
    printf("-sPORT         Serial Port\n");
    printf("-pBAUD         Serial Port Baud Rate\n");
    printf("-b BeagleBone: SPI1_SCLK: P9_31 (+GPIO out); SPI1_D0: P9_29 (RXD);\n");
    printf("               SPI1_D1: P9_30 (TXD), Reset: P9_27 (GPIO out); RXD: D0\n");
    printf("-c CV-Main-Board (Default): SPI1_SCLK: ECAP0_PWM (+GPIO out); SPI1_D0: UART0_CTSN (RXD);\n");
    printf("               SPI1_D1: UART0_CTSN (TXD), Reset: UART0_TXD (GPIO out); RXD: D0\n");
    printf("-d             Debug\n");
}

/*=========================================================================*/
static void CleanAndExit(int ret)
{
    static int inExit = 0;

    if (inExit)
        return;

    inExit = 1;

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
                case 's':
                    strncpy(lk_pcPort, argv[i]+2, SER_PORT_NAME_LEN);
                    break;
                case 'p':
                    lk_ulBaudRate = (unsigned long)atoi(argv[i]+2);
                    break;
                case 'b':
                    lk_board = MBOARD_BEAGLE;
                    break;
                case 'c':
                    lk_board = MBOARD_CV;
                    break;
                case 'd':
                    gl_bDebug = TRUE;
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
void HandleError(int iExitCode, int iErrorCode, const char* const pcFile,
                 const int iLine, char* pcErrorText)
{
    // Give out source file name and line number where the Error happened for
    // debug issues
    if (gl_bDebug) {
        fprintf(stderr, "%s, Line %i: %i, %i: %s\n", pcFile, iLine, iErrorCode,
                errno, pcErrorText);
    }

    perror(NULL);
}

/*=========================================================================*/
int InitSerial(char* pcPort, unsigned long ulBaudRate, unsigned char ui8DataBits,
               unsigned char ui8StopBits, char cParity, BOOL bXon, BOOL bRecv,
               BOOL bTimeout, BOOL bTimeoutInMsecs, unsigned long ulTimeout)
{
    int iFD; // File descriptor for the port
    struct termios termios;
    char pcError[SER_ERR_TXT_LEN];

    iFD = open(pcPort, O_RDWR | O_NOCTTY/* | O_NDELAY*/);
    if (iFD == -1)
    {
        snprintf(pcError, SER_ERR_TXT_LEN, "Unable to open %s", pcPort);
        HandleError(240, 240, __FILE__, __LINE__, pcError);
    }

    fcntl(iFD, F_SETFL, 0); // Rcv-Blocking

    tcgetattr(iFD, &termios);
    // Set baud rate
    switch (ulBaudRate)
    {
        case 50:
            cfsetispeed(&termios, B50);
            cfsetospeed(&termios, B50);
            break;
        case 75:
            cfsetispeed(&termios, B75);
            cfsetospeed(&termios, B75);
            break;
        case 110:
            cfsetispeed(&termios, B110);
            cfsetospeed(&termios, B110);
            break;
        case 134:
            cfsetispeed(&termios, B134);
            cfsetospeed(&termios, B134);
            break;
        case 150:
            cfsetispeed(&termios, B150);
            cfsetospeed(&termios, B150);
            break;
        case 200:
            cfsetispeed(&termios, B200);
            cfsetospeed(&termios, B200);
            break;
        case 300:
            cfsetispeed(&termios, B300);
            cfsetospeed(&termios, B300);
            break;
        case 600:
            cfsetispeed(&termios, B600);
            cfsetospeed(&termios, B600);
            break;
        case 1200:
            cfsetispeed(&termios, B1200);
            cfsetospeed(&termios, B1200);
            break;
        case 1800:
            cfsetispeed(&termios, B1800);
            cfsetospeed(&termios, B1800);
            break;
        case 2400:
            cfsetispeed(&termios, B2400);
            cfsetospeed(&termios, B2400);
            break;
        case 4800:
            cfsetispeed(&termios, B4800);
            cfsetospeed(&termios, B4800);
            break;
        case 9600:
            cfsetispeed(&termios, B9600);
            cfsetospeed(&termios, B9600);
            break;
        case 19200:
            cfsetispeed(&termios, B19200);
            cfsetospeed(&termios, B19200);
            break;
        case 38400:
            cfsetispeed(&termios, B38400);
            cfsetospeed(&termios, B38400);
            break;
        default:
        case 57600:
            cfsetispeed(&termios, B57600);
            cfsetospeed(&termios, B57600);
            break;
        case 115200:
            cfsetispeed(&termios, B115200);
            cfsetospeed(&termios, B115200);
            break;
    }
    termios.c_cflag |= CLOCAL;
    // Set data bits
    termios.c_cflag &= ~CSIZE;
    switch (ui8DataBits)
    {
        default:
        case 8:
            termios.c_cflag |= CS8;
            break;
        case 7:
            termios.c_cflag |= CS7;
            break;
        case 6:
            termios.c_cflag |= CS6;
            break;
        case 5:
            termios.c_cflag |= CS5;
            break;
    }
    // Set stop bits
    termios.c_cflag &= ~CSTOPB;
    switch (ui8StopBits)
    {
        case 1:
            break;
        default:
        case 2:
            termios.c_cflag |= CSTOPB;
            break;
    }
    // Set parity
    termios.c_cflag &= ~PARENB;
    switch (cParity)
    {
        default:
        case 'n':
            break;
        case 'e':
            termios.c_cflag |= PARENB;
            termios.c_cflag &= ~PARODD;
            break;
        case 'o':
            termios.c_cflag |= PARENB;
            termios.c_cflag |= PARODD;
            break;
    }
    // Set xon/xoff
    if (bXon)
    {
        termios.c_iflag |= (IXON | IXOFF | IXANY);
    }
    else
    {
        termios.c_iflag &= ~(IXON | IXOFF | IXANY);
    }

    termios.c_cflag |= CLOCAL; // Do not change owner of port
    if (bRecv)
    {
        termios.c_cflag |= CREAD; // Turn on data rcv
        termios.c_iflag |= (INPCK | ISTRIP); // Checksum for incoming
        // Strip Parity bits from data
        termios.c_lflag = 0; // RAW input
    }
    termios.c_oflag &= ~OPOST; // RAW-Output

    if (bTimeout)
    {
        if (bTimeoutInMsecs)
        {
            termios.c_cc[VTIME] = ulTimeout/1000*10;
        }
        else
        {
            termios.c_cc[VTIME] = ulTimeout*10;
        }
    }

    tcsetattr(iFD, TCSANOW, &termios);

    return (iFD);
}

/*=========================================================================*/
static void SysFs_WriteValue2File(char* pcFile, char* pcValue)
{
    int fd;
    unsigned int uiLen;
    char pcError[SER_ERR_TXT_LEN];

    if (NULL==pcFile || NULL==pcValue) {
        snprintf(pcError, SER_ERR_TXT_LEN, "NULL Pointer error!");
        HandleError(240, 240, __FILE__, __LINE__, pcError);
    }

    fd = open(pcFile, O_WRONLY);
    if (-1 == fd) {
        snprintf(pcError, SER_ERR_TXT_LEN, "Failed to open %s!\n", pcFile);
        HandleError(240, 240, __FILE__, __LINE__, pcError);
    }
    uiLen = strlen(pcValue);
    if (uiLen != write(fd, pcValue, uiLen)) {
        snprintf(pcError, SER_ERR_TXT_LEN, "Failed to write value %s in %s!\n", pcValue, pcFile);
        HandleError(240, 240, __FILE__, __LINE__, pcError);
    }
    close(fd);
}

/*=========================================================================*/
static int SysFs_GPIOWrite(int pin, int value)
{
    static const char s_values_str[] = "01";
    char pcError[SER_ERR_TXT_LEN];

#define VALUE_MAX 30
    char path[VALUE_MAX];
    int fd;

    if ((value != 0) && (value != 1))
        return -1;

    snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
    fd = open(path, O_WRONLY);
    if (-1 == fd) {
        snprintf(pcError, SER_ERR_TXT_LEN, "Failed to open gpio%d value for writing!\n", pin);
        HandleError(240, 240, __FILE__, __LINE__, pcError);
        return(-1);
    }

    if (1 != write(fd, &s_values_str[value], 1)) {
        snprintf(pcError, SER_ERR_TXT_LEN, "Failed to write value %c on gpio%d!\n", s_values_str[value], pin);
        HandleError(240, 240, __FILE__, __LINE__, pcError);
        return(-1);
    }

    close(fd);

    return(0);
}

/*=========================================================================*/
int main(int argc, char *argv[])
{
    int i;
    char  pcDisplayBuf[DISPLAY_BUF_SIZE+1];
    char* pcText = "hello";
    int iSerial;
    int iBytestoSend;
    int iBytesSend;
    char* pcPort;

    lk_pcPort[0] = '\0';

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
        printf("Text: %s\n", argv[i]);
        pcText = argv[i];
    }

    switch (lk_board)
    {
        case MBOARD_CV:
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:UART0_CTSN_pinmux/state", "default");
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:UART0_RTSN_pinmux/state", "default");
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:ECAP0_PWM_pinmux/state", "default");
            break;

        case MBOARD_BEAGLE:
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_27_pinmux/state", "default"); // Reset
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_21_pinmux/state", "uart");
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_32_pinmux/state", "uart");
            break;
    }

    switch (lk_board)
    {
        case MBOARD_CV:
            SysFs_GPIOWrite(43, 1); // Set Reset
            pcPort = "/dev/ttyO1";
            break;

        case MBOARD_BEAGLE:
            SysFs_GPIOWrite(115, 1); // Set Reset
            pcPort = "/dev/ttyS2";
            break;
    }

    if (lk_pcPort[0] != '\0') {
        pcPort = lk_pcPort;
    }

    iSerial = InitSerial(pcPort, lk_ulBaudRate, 8, 1, 'N', FALSE, FALSE, TRUE, FALSE, 10);

    iBytestoSend = snprintf(pcDisplayBuf, DISPLAY_BUF_SIZE, "\001TD\"%s\n\";1;1;0\027", pcText);

    iBytesSend = write(iSerial, pcDisplayBuf, iBytestoSend);
    if (iBytesSend != iBytestoSend)
    {
        fprintf(stderr, "Failed to send. %i instead of %i Bytes\n", iBytesSend, iBytestoSend);
        fprintf(stderr, "Failed Send Msg:\n");
        fprintf(stderr, pcDisplayBuf);
    }

    close(iSerial);
}
