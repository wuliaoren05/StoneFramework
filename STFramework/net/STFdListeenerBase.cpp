#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>

#include "net/STFdListeenerBase.h"
#include "thread/STThread.h"
#include "base/STMutex.h"



const int MaxEventCount = 500;


class STFdListeenerBase::Impl : public STThread
{
public:
    enum FdCmd
    {
        FdCmd_Add = 0,
        FdCmd_Remove,

        FdCmd_Count
    };
    enum ThreadCmd
    {
        ThreadCmd_NewFdTask = 0,
        ThreadCmd_Exit,

        ThreadCmd_Count
    };
    struct FdTaskItem
    {
        FdCmd   cmd;
        int     fd;
        FdTaskItem(int itemFd, FdCmd itemCmd)
            : cmd(itemCmd)
            , fd(itemFd)
        {}
    };
    typedef std::vector<FdTaskItem> FdTask;

public:
    Impl(const STString& threadName, STFdListeenerBase* owner)
        : STThread(threadName)
        , m_owner(owner)
    {
        m_cmdPipe[0] = -1;
        m_cmdPipe[1] = -1;
    }

    void addFd(int fd)
    {
        if ( !this->isRunning() ) {
            checkAndOpenPipe();
            this->exec();
        }
        setnonblocking(fd);
        addOneFdTask(fd, FdCmd_Add);
    }

    void removeFd(int fd)
    {
        addOneFdTask(fd, FdCmd_Remove);
    }

    void stopListen()
    {
        if ( checkAndOpenPipe() ) {
            ThreadCmd cmd = ThreadCmd_Exit;
            write(m_cmdPipe[1], &cmd, sizeof(cmd));

            this->join();//wait read data thread exit
        }
    }

    bool setnonblocking(int sockfd)
    {
        if (fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK) == -1)
        {
            return false;
        }
        return true;
    }

    void main()
    {
        epoll_event* eventArr = new epoll_event[MaxEventCount];/* Buffer where events are returned */
        STDAssert(NULL != eventArr);

        int efd = epoll_create1(0);
        STDAssert(-1 != efd);//check if epoll create failed

        bool openRet = checkAndOpenPipe();
        STDAssert(openRet > 0);
        epoll_event event;
        event.data.fd = m_cmdPipe[0];
        event.events = EPOLLIN | EPOLLHUP;//just care about in and disconnect

        int ret = epoll_ctl(efd, EPOLL_CTL_ADD, m_cmdPipe[0], &event);
        STDAssert(-1 != ret);

        while (1) {
            int n = epoll_wait(efd, eventArr, MaxEventCount, -1);
            if (-1 == n) {
                continue;//error happend!
            }
            //must deal task first.
            bool isExitCommand = false;
            std::vector<int> removedFds;
            for(int i=0; i<n; i++) {
                if ((eventArr[i].data.fd == m_cmdPipe[0]) && (eventArr[i].events & EPOLLIN)) {
                    ThreadCmd cmd = ThreadCmd_Count;
                    if (read(m_cmdPipe[0], &cmd, sizeof(cmd)) > 0) {
                        if (ThreadCmd_Exit == cmd) {
                            //ask thread to exit
                            isExitCommand = true;
                            break;
                        }
                    }

                    if (m_fdTask.size() > 0) {
                        m_fdTaskLock.lock();
                        for (unsigned int j=0; j<m_fdTask.size(); ++j) {
                            switch (m_fdTask[j].cmd) {
                            case FdCmd_Add:
                                epoll_event event;
                                event.data.fd = m_fdTask[j].fd;
                                event.events = EPOLLIN | EPOLLHUP;//just care about in and disconnect

                                epoll_ctl(efd, EPOLL_CTL_ADD, m_fdTask[j].fd, &event);
                                break;
                            case FdCmd_Remove:
                                epoll_ctl(efd, EPOLL_CTL_DEL, m_fdTask[j].fd, NULL);
                                removedFds.push_back(m_fdTask[j].fd);
                                break;
                            default:
                                break;
                            }
                        }
                        m_fdTask.clear();
                        m_fdTaskLock.unlock();
                    }
                    break;
                }
            }

            if (isExitCommand) {
                break;
            }

            //just notify not removed fd.
            for(int i=0; i<n; i++) {
                if ((eventArr[i].data.fd != m_cmdPipe[0]) && eventArr[i].events & EPOLLIN) {
                    bool isRemoved = false;
                    for(unsigned int j=0; j<removedFds.size(); ++j) {
                        if (removedFds[i] == eventArr[i].data.fd) {
                            isRemoved = true;
                            break;
                        }
                    }
                    if (!isRemoved) {
                        m_owner->fdChanged(eventArr[i].data.fd, STFdListeenerBase::FdChangeType_CanRead);
                    }
                }
            }
        }

        close(efd);
        delete [] eventArr;
    }

private:
    bool checkAndOpenPipe()
    {
        bool ret = true;
        if (-1 == m_cmdPipe[0]) {
            ret = ( pipe(m_cmdPipe) < 0 );
        }
        return ret;
    }

    void closePipe()
    {
        if (-1 != m_cmdPipe[0]) {
            close(m_cmdPipe[0]);
            close(m_cmdPipe[1]);
        }
    }

    void addOneFdTask(int fd, FdCmd cmd)
    {
        m_fdTaskLock.lock();
        m_fdTask.push_back( FdTaskItem(fd, cmd) );
        m_fdTaskLock.unlock();

        if ( checkAndOpenPipe() ) {
            ThreadCmd cmd = ThreadCmd_NewFdTask;
            write(m_cmdPipe[1], &cmd, sizeof(cmd));
        }
    }

private:
    STFdListeenerBase*  m_owner;

    int                 m_cmdPipe[2];//0 for read, 1 for write

    FdTask              m_fdTask;
    STMutex             m_fdTaskLock;
};


STFdListeenerBase::STFdListeenerBase(const STString& name)
    : m_impl(new Impl(name, this))
{
}

STFdListeenerBase::~STFdListeenerBase()
{
}

void STFdListeenerBase::addFd(int fd)
{
    m_impl->addFd(fd);
}

void STFdListeenerBase::removeFd(int fd)
{
    m_impl->removeFd(fd);
}

void STFdListeenerBase::stopListen()
{
    m_impl->stopListen();
}











