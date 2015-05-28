#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

#include "tools/STIOTools.h"


STString STConsoleTool::readOneLine()
{
    STString ret;

    std::getline(std::cin, ret);

    return ret;
}

void STConsoleTool::write(const STString& str)
{
    std::cout<<str<<std::endl;
}











STSerialTool::STSerialTool(const STString &devStr, STSerialTool::BaudRate speed,
    STSerialTool::DataBits dataBits, STSerialTool::StopBits stopBits, STSerialTool::Parity parity)
    : m_devStr(devStr)
    , m_fd(-1)
    , m_baudRate(speed)
    , m_dataBits(dataBits)
    , m_stopBits(stopBits)
    , m_parity(parity)
{
}

STSerialTool::~STSerialTool()
{
}

bool STSerialTool::openDev()
{
    m_fd = open(m_devStr.c_str(), O_RDWR);

    if ( 0 >= m_fd) {
        return false;
    }

    if ( !setSpeed(m_baudRate) ) {
        return false;
    }

    if ( !setParity(m_dataBits, m_stopBits, m_parity) )  {
        return false;
    }

    return true;
}

inline bool STSerialTool::isOpen()
{
    return m_fd >= 0;
}

void STSerialTool::closeDev()
{
    if ( isOpen() ) {
        close(m_fd);
    }
}

bool STSerialTool::setSpeed(BaudRate speed)
{
    if ( !isOpen() ) {
        return false;
    }

    static int speedArr[BaudRate_Count] = {B230400, B115200, B57600, B38400, B19200
                                           , B9600, B4800, B2400, B1200, B300};
    int   status;
    struct termios   Opt;
    tcgetattr(m_fd, &Opt);
    if (BaudRate_Count != speed) {
        tcflush(m_fd, TCIOFLUSH);
        cfsetispeed(&Opt, speedArr[speed]);
        cfsetospeed(&Opt, speedArr[speed]);
        status = tcsetattr(m_fd, TCSANOW, &Opt);
        if (0 != status) {
            return false;
        }
        tcflush(m_fd,TCIOFLUSH);
    }

    return true;
}

bool STSerialTool::setParity(DataBits dataBits, StopBits stopBits, Parity parity)
{
    if ( !isOpen() ) {
        return false;
    }

    struct termios options;
    if ( tcgetattr( m_fd, &options) != 0 ) {
        return false;
    }
    options.c_cflag &= ~CSIZE;

    switch (dataBits) {
    case DataBits_7:
        options.c_cflag |= CS7;
        break;
    case DataBits_8:
        options.c_cflag |= CS8;
        break;
    default:
        //fprintf(stderr,"Unsupported data size\n");
        return false;
    }

    switch (parity) {
    case Parity_none:
        options.c_cflag &= ~PARENB;   /* Clear parity enable */
        options.c_iflag &= ~INPCK;     /* Enable parity checking */
        break;
    case Parity_Odd:
        options.c_cflag |= (PARODD | PARENB);
        options.c_iflag |= INPCK;             /* Disnable parity checking */
        break;
    case Parity_Even:
        options.c_cflag |= PARENB;     /* Enable parity */
        options.c_cflag &= ~PARODD;
        options.c_iflag |= INPCK;       /* Disnable parity checking */
        break;
    default:
        //fprintf(stderr,"Unsupported parity\n");
        return false;
    }

    switch (stopBits) {
    case StopBits_1:
        options.c_cflag &= ~CSTOPB;
        break;
    case StopBits_2:
        options.c_cflag |= CSTOPB;
        break;
    default:
        //fprintf(stderr,"Unsupported stop bits\n");
        return false;
    }

    /* Set input parity option */
    if (parity != Parity_none) {
        options.c_iflag |= INPCK;
    }
    tcflush(m_fd, TCIFLUSH);
    options.c_cc[VTIME] = 150;
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
    if ( tcsetattr(m_fd,TCSANOW,&options) != 0 ) {
        //perror("SetupSerial 3");
        return false;
    }

    return true;
}
