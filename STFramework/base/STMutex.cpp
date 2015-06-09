#include <pthread.h>
#include "base/STMutex.h"

class STMutex::PrivateData
{
public:
    pthread_mutex_t mutexVar;


    PrivateData()
    {
        pthread_mutex_init(&mutexVar, NULL);
    }
    ~PrivateData()
    {
        pthread_mutex_unlock(&mutexVar);
        pthread_mutex_destroy(&mutexVar);
    }
};

STMutex::STMutex()
    : m_data(new PrivateData)
{
}

STMutex::~STMutex()
{}


void STMutex::lock()
{
    pthread_mutex_lock( &(m_data->mutexVar) );
}

bool STMutex::tryLock()
{
    return 0 == pthread_mutex_trylock( &(m_data->mutexVar) );
}

void STMutex::unlock()
{
    pthread_mutex_unlock( &(m_data->mutexVar) );
}












