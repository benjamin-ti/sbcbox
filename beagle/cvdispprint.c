/***************************************************************************/
/*                             Druckersoftware                             */
/*                    Copyright (C) Carl Valentin GmbH                     */
/***************************************************************************/


/*. IMPORT ================================================================*/

/*. ENDIMPORT =============================================================*/


/*. EXPORT ================================================================*/

/*. ENDEXPORT =============================================================*/


/*. LOCAL =================================================================*/

/*-------------------------------- Makros ---------------------------------*/

#define DISPLAY_BUF_SIZE 60

/*--------------------------- Typdeklarationen ----------------------------*/

/*------------------------------ Prototypen -------------------------------*/

/*------------------------ Konstantendeklarationen ------------------------*/

/*------------------------- modulglobale Variable -------------------------*/

/*. ENDLOCAL ==============================================================*/

/*=========================================================================*/
int InitSerial(char cPort, unsigned long ulBaudRate, lk_ui8DataBits,
               lk_ui8StopBits, lk_cParity, lk_bXon, lk_bRecv, lk_bTimeout,
               lk_bTimeoutInMsecs)
{
#define SER_ERR_TXT_LEN   30
    int iFD; // File descriptor for the port
    struct termios termios;
    char pcPort[SER_PORT_NAME_LEN];
    char pcError[SER_ERR_TXT_LEN];

    snprintf(pcPort, SER_PORT_NAME_LEN, "/dev/ttyS%c", lk_cPort-1);

    iFD = open(pcPort, O_RDWR | O_NOCTTY/* | O_NDELAY*/);
    if (iFD == -1)
    {
        snprintf(pcError, SER_ERR_TXT_LEN, _("Unable to open %s"), pcPort);
        HandleError(240, 240, __FILE__, __LINE__, pcError);
    }

    fcntl(iFD, F_SETFL, 0); // Rcv-Blocking

    tcgetattr(iFD, &termios);
    // Set baud rate
    switch (lk_ulBaudRate)
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
    switch (lk_ui8DataBits)
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
    switch (lk_ui8StopBits)
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
    switch (lk_cParity)
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
    if (lk_bXon)
    {
        termios.c_iflag |= (IXON | IXOFF | IXANY);
    }
    else
    {
        termios.c_iflag &= ~(IXON | IXOFF | IXANY);
    }

    termios.c_cflag |= CLOCAL; // Do not change owner of port
    if (lk_bRecv)
    {
        termios.c_cflag |= CREAD; // Trun on data rcv
        termios.c_iflag |= (INPCK | ISTRIP); // Checksum for incoming
        // Strip Parity bits from data
        termios.c_lflag = 0; // RAW input
    }
    termios.c_oflag &= ~OPOST; // RAW-Output

    if (lk_bTimeout)
    {
        if (lk_bTimeoutInMsecs)
        {
            termios.c_cc[VTIME] = lk_ulTimeout/1000*10;
        }
        else
        {
            termios.c_cc[VTIME] = lk_ulTimeout*10;
        }
    }

    tcsetattr(iFD, TCSANOW, &termios);

    return (iFD);
}

/*=========================================================================*/
static void SendData(unsigned char* pucBuf, int iBytestoSend)
{
    int iBytesSend;

    iBytesSend = write(iFD, pucBuf, iBytestoSend);
}

/*=========================================================================*/
int main(int argc, char *argv[])
{
    char  pcDisplayBuf[DISPLAY_BUF_SIZE+1];
    char* pcText = "hello";
    int iSerial;

/*
    switch (lk_board)
    {
        case MBOARD_CV:
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:UART0_CTSN_pinmux/state", "default");
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:UART0_RTSN_pinmux/state", "default");
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:ECAP0_PWM_pinmux/state", "default");
            break;

        case MBOARD_BEAGLE:*/
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_27_pinmux/state", "default"); // Reset
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_29_pinmux/state", "default");
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_30_pinmux/state", "default");
            SysFs_WriteValue2File("/sys/devices/platform/ocp/ocp:P9_31_pinmux/state", "default"); // Clk
/*            break;
    }*/

/*
    switch (lk_board)
    {
        case MBOARD_CV:
            SysFs_GPIOWrite(43, 1); // Set Reset
            break;
*/
        case MBOARD_BEAGLE:
            SysFs_GPIOWrite(115, 1); // Set Reset
/*            break;
    }*/

    iSerial = InitSerial();

    iBytestoSend = snprintf(pcDisplayBuf, DISPLAY_BUF_SIZE, "\001TD\"%s\n\";1;1;0\027", pcText);

    iBytesSend = write(iSerial, pcDisplayBuf, iBytestoSend);

    close(iSerial);
}
