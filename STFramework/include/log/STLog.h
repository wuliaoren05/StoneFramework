#ifndef ST_LOG_H
#define ST_LOG_H

#include "base/STCommonDefine.h"


enum LogLevel
{
    LogLevel_Info = 0,
    LogLevel_Warn,
    LogLevel_Debug,
    LogLevel_Err,

    LogLevel_Count
};

class STLog;

STLog& LogInfo();
STLog& LogWarn();
STLog& LogDebug();
STLog& LogErr();

void Log(LogLevel level, const STString& value);


class STLog
{
public:
    static const STString endl;

public:
    STLog(LogLevel level);
    ~STLog();

public:
    STLog& operator <<(const STString& value);
    STLog& operator <<(int value);
    STLog& operator <<(double value);

private:
    void writeToConsole(const STString& value);
    void writeToSerialPort(const STString& value);
    void writeToFile(const STString& value);//not supported now

private:
    LogLevel m_logLevel;
};



#endif // ST_LOG_H
