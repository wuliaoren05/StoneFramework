#ifndef ST_THREAD_H
#define ST_THREAD_H

#include "base/STWaiter.h"
#include "base/STCommonDefine.h"
#include "base/STPtr.h"

class STThread
{
enum ExecRet
{
    ExecRet_Success = 0,
    ExecRet_AlreadyRunning,
    ExecRet_Err,
    ExecRet_Count
};

public:
    STThread(const STString& name = "");
    virtual ~STThread();

    STString name();
    virtual void main() = 0;//thread main function

    ExecRet exec();
    void join();

    bool isRunning() const;
    void askToStop();//ask thread to exit, not wait
    void askAndWaitToStop(int s=2);//ask and wait thread to exit, second

    bool isNeedStop() const;
    virtual void dealAfterKill() //if exit not normal, this function will be run
    {
        //you may need free you resource here when exception happen
    }
private:

    STThread(const STThread& other);
    STThread& operator=(const STThread& other);
    bool operator==(const STThread& other);

private:
    static void* runThread(void *thread);

protected:
    class PrivateData;
    STScopePtr<PrivateData> m_data;
};

#endif // ST_THREAD_H
