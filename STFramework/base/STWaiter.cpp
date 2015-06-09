#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <errno.h>
#include <signal.h>

#include "base/STWaiter.h"



//STWaiter
class STWaiter::PrivateData
{
public:
    sem_t semId;

    PrivateData()
    {
        if ( sem_init (&semId, 0, 0) == -1 ) {
            //"sem_init is err"
        }
    }
    ~PrivateData()
    {
        sem_destroy(&semId);
    }
};

STWaiter::STWaiter()
    : m_data(new PrivateData())
{
}

STWaiter::~STWaiter()
{
}

void STWaiter::wait()
{
    sem_wait(&m_data->semId);
}

void STWaiter::reset()
{
    if ( sem_init (&m_data->semId, 0, 0) == -1 ) {
        //"sem_init is err"
    }
}

STWaitResult STWaiter::wait(int s)
{
    STWaitResult ret = STWaitResult_Happen;

    timespec waitTime;
    waitTime.tv_sec = time(NULL) + s;
    waitTime.tv_nsec = 0;
    int status = -1;
    while ((status = sem_timedwait(&m_data->semId, &waitTime)) == -1 && errno == EINTR) {
        continue;       /* Restart if interrupted by handler */
    }
    if (-1 == status) {
        if (errno == ETIMEDOUT) {
            ret = STWaitResult_Timeout;
        }
        else {
            ret = STWaitResult_Err;
        }
    }
    else {
        ret = STWaitResult_Happen;
    }

    return ret;
}

void STWaiter::setHappen()
{
    sem_post(&m_data->semId);
}

//STFDWaiter
STFDWaiter::STFDWaiter(const STString fileName)
    : m_fileName(fileName)
    , m_fd(-1)
{
}

STFDWaiter::~STFDWaiter()
{
    closeFd();
}

int STFDWaiter::fd()
{
    if (0 > m_fd) {
        m_fd = open(m_fileName.c_str(), O_RDONLY);
    }

    return m_fd;
}

STWaitResult STFDWaiter::wait(int time)
{
    STWaitResult ret = STWaitResult_Err;

    if (fd() >= 0) {
        struct timeval timeout = {0UL, 0UL};
        if (time >= 0) {
            timeout.tv_sec = time/1000;
            timeout.tv_usec = (time%1000) * 1000;
        }
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd(),&fds);
        int selectRet = select(fd() + 1, &fds, NULL, NULL, time>=0 ? &timeout : NULL);
        if (0 > selectRet) {
            ret = STWaitResult_Err;
        }
        else if (0 == selectRet) {
            ret = STWaitResult_Timeout;
        }
        else {
            ret = STWaitResult_Happen;
        }
    }

    return ret;
}

void STFDWaiter::closeFd()
{
    if (m_fd >= 0) {
        close(m_fd);
        m_fd = -1;
    }
}














