#include <pthread.h>

#include "thread/STThread.h"
#include "base/STWaiter.h"

class STThread::PrivateData
{
public:
    STString        name;
    bool            isRunning;
    pthread_t       threadNo;
    pthread_attr_t  threadAttr;//not use, but I will use it latter

public:
    PrivateData(const STString& name)
        : name(name)
        , isRunning(false)
        , threadNo(0)
    {}
    ~PrivateData()
    {
        //pthread_attr_destroy(&threadAttr);
    }
};

void* STThread::runThread(void *threadObj)
{
    if (NULL == threadObj) {
        return NULL;
    }

    STThread *thread = static_cast<STThread*>(threadObj);
    if (NULL != thread) {
        thread->m_data->isRunning = true;
        thread->main();
        thread->m_data->isRunning = false;
    }

    return NULL;
}


STThread::STThread(const STString& name)
    : m_data(new PrivateData(name))
{
}


STThread::~STThread()
{
    if (m_data->isRunning) {
        forceThreadStop();
        join();
    }
}

STString STThread::name()
{
    return m_data->name;
}

STThread::ExecRet STThread::exec()
{
    ExecRet ret = ExecRet_Success;

    if (!m_data->isRunning && m_data->threadNo==0) {
        int err = pthread_create(&m_data->threadNo, NULL, &STThread::runThread, this);//TD, use attr
        if (0 != err) {
            m_data->threadNo = 0;
            ret = ExecRet_Err;
        }
        else {
            ret = ExecRet_Success;
        }
    }
    else {
        ret = ExecRet_AlreadyRunning;
    }

    return ret;
}

void STThread::join()
{
    if (m_data->isRunning) {
        pthread_join(m_data->threadNo, 0);
    }
}

void STThread::forceThreadStop()
{
    if (m_data->isRunning) {
        pthread_cancel(m_data->threadNo);
        dealAfterKill();
    }
}

bool STThread::isRunning() const
{
    return m_data->isRunning;
}








