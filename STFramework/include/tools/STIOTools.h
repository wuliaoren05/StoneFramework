#ifndef ST_IO_TOOLS_H
#define ST_IO_TOOLS_H

#include "base/STCommonDefine.h"

class STConsoleTool
{
public:
    static STString readOneLine();
    static void write(const STString& str);
};

class STSerialTool
{
public:
    enum BaudRate
    {
        BaudRate_230400 = 0,
        BaudRate_115200,
        BaudRate_57600,
        BaudRate_38400,
        BaudRate_19200,
        BaudRate_9600,
        BaudRate_4800,
        BaudRate_2400,
        BaudRate_1200,
        BaudRate_300,

        BaudRate_Count
    };

    enum DataBits
    {
        DataBits_7 = 7,
        DataBits_8 = 8
    };
    enum StopBits
    {
        StopBits_1 = 1,
        StopBits_2 = 2
    };
    enum Parity
    {
        Parity_none = 0,
        Parity_Odd,
        Parity_Even
    };

public:
    STSerialTool(const STString& devStr = "/dev/tty0", BaudRate speed = BaudRate_115200
            , DataBits dataBits = DataBits_7, StopBits stopBits = StopBits_1, Parity parity = Parity_none);
    ~STSerialTool();

    bool openDev();
    bool isOpen();
    void closeDev();

private:
    bool setSpeed(BaudRate speed);
    bool setParity(DataBits dataBits, StopBits stopBits, Parity parity);

private:
    STString    m_devStr;
    int         m_fd;

    BaudRate    m_baudRate;
    DataBits    m_dataBits;
    StopBits    m_stopBits;
    Parity      m_parity;
};

#endif // ST_IO_TOOLS_H
