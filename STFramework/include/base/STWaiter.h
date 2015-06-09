#ifndef ST_WAITER_H
#define ST_WAITER_H

#include "base/STCommonDefine.h"
#include "base/STPtr.h"

enum STWaitResult
{
    STWaitResult_Happen = 0,
    STWaitResult_Timeout,
    STWaitResult_Err,

    STWaitResult_Count
};

//thread safe
class STWaiter
{
public:
    STWaiter();
    ~STWaiter();

    void wait();//just wait till happen
    STWaitResult wait(int s);//time:second, wait till happen or timeout
    void reset();//set wait value to 0
    void setHappen();//let it happen

private:
    STWaiter(const STWaiter&);
private:
    class PrivateData;
    STScopePtr<PrivateData> m_data;
};

class STFDWaiter
{
public:
    STFDWaiter(const STString fileName);
    ~STFDWaiter();

    int fd();
    STWaitResult wait(int time = -1);//time:millisecond, <0 mean wait till happen
    void closeFd();

private:
    STFDWaiter(const STFDWaiter&);
private:
    STString m_fileName;
    int      m_fd;
};



#endif // ST_WAITER_H
