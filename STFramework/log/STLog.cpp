#include "log/STLog.h"

#include "tools/STIOTools.h"
#include "tools/STStringTool.h"

const STString STLog::endl = "\r\n";

STLog::STLog(LogLevel level)
    : m_logLevel(level)
{
}

STLog::~STLog()
{
}

STLog &STLog::operator <<(const STString &value)
{
    writeToConsole(value);
    return *this;
}

STLog &STLog::operator <<(int value)
{
    writeToConsole( STStringTool::intToStr(value) );
    return *this;
}

STLog &STLog::operator <<(double value)
{
    writeToConsole( STStringTool::doubleToStr(value) );
    return *this;
}

void STLog::writeToConsole(const STString &value)
{
    STConsoleTool::write(value);
}

void STLog::writeToSerialPort(const STString &value)
{
    STUNUSED(value);
    //implement latter
}

void STLog::writeToFile(const STString &value)
{
    STUNUSED(value);
    //implement latter
}



STLog &LogInfo()
{
    static STLog log(LogLevel_Info);
    return log;
}


STLog &LogWarn()
{
    static STLog log(LogLevel_Warn);
    return log;
}


STLog &LogDebug()
{
    static STLog log(LogLevel_Debug);
    return log;
}


STLog &LogErr()
{
    static STLog log(LogLevel_Err);
    return log;
}


void Log(LogLevel level, const STString &value)
{
    switch (level) {
    case LogLevel_Info:
        LogInfo()<<value;
        break;
    case LogLevel_Warn:
        LogWarn()<<value;
        break;
    case LogLevel_Debug:
        LogDebug()<<value;
        break;
    case LogLevel_Err:
        LogErr()<<value;
        break;
    default:
        //can not be here
        break;
    }
}
























