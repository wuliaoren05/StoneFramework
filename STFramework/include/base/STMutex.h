#ifndef ST_MUTEX_H
#define ST_MUTEX_H

#include "base/STPtr.h"

class STMutex
{
public:
    STMutex();
    ~STMutex();

    void lock();
    bool tryLock();
    void unlock();

private:
    class PrivateData;
    STScopePtr<PrivateData> m_data;
};

#endif // ST_MUTEX_H
