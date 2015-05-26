#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#include "net/STNetDefine.h"
#include "net/STServer.h"
#include "thread/STThread.h"
#include "net/STFdListeenerBase.h"
#include "net/SocketFdReader.h"
#include "net/STNetEvent.h"



const STString ServerListenConnectThreadPrefix  =  "ServerListenConnectThread_";
const STString ServerListenDataThreadPrefix     =  "ServerListenDataThread_";


class STServer::Impl
{
public:
    Impl(const STString& name)
        : m_serverName(name)
        , m_listenConnectThread(name, this)
        , m_listenDataThread(name, this)
    {
        signal(SIGPIPE, SIG_IGN);
    }
    ~Impl(){}


    void setEventReceiver(STObject* receiver)
    {
        if (NULL != receiver && m_safeReceiver.receiver != receiver) {
            m_safeReceiver.receiver = receiver;
            m_safeReceiver.receiverGuard = receiver->getGuard();
        }
    }

    bool startListen(int port)
    {
        if ( !m_listenConnectThread.isRunning() ) {
            m_listenConnectThread.setListenPort(port);
            m_listenConnectThread.exec();
        }

        return m_listenConnectThread.isRunning();
    }

    void stop()
    {
        m_listenConnectThread.forceThreadStop();
        m_listenDataThread.stopListen();
    }

    bool sendToClient(const STNetIdentify& target, const STString& dataStr)
    {
        int dataSize = dataStr.size();
        int ret = write(target.fd(), FrameHeadTag, sizeof(FrameHeadTag));
        if (-1 != ret) {
            ret = write(target.fd(), &dataSize, sizeof(dataSize));
        }
        if (-1 != ret) {
            write(target.fd(), dataStr.c_str(), dataSize);
        }
        return -1 != ret;
    }

private:
    STNetIdentify getIdViaFd(int fd)
    {
        STNetIdentify ret;
        ret.setFd(fd);

        int port;

        struct sockaddr local_addr;
        socklen_t len = sizeof(sockaddr);
        if (getpeername(fd, &local_addr, &len) == 0) {
            struct sockaddr_in* sin = (struct sockaddr_in*)(&local_addr);
            port = sin->sin_port;

            char addr_buffer[INET_ADDRSTRLEN];
            void * tmp = &(sin->sin_addr);
            if (inet_ntop(AF_INET, tmp, addr_buffer, INET_ADDRSTRLEN) != NULL) {
                ret.setIp(addr_buffer);
                ret.setPort(port);
            }
            else {
                //"inet_ntop err";
            }
        }
        else {
            //"getsockname err";
        }

        return ret;
    }

private://call from the two listen threads
    void onClientConnected(int fd)//call from listenConnectThread
    {
        m_listenDataThread.addFd(fd);
        STEventCarrier e(new STNetEvent(getIdViaFd(fd), STNetEvent::Type_RemoteConnected));
        postEventToReceiver(e);
    }
    void onClientDisConnected(int fd)//call from listenDataThread
    {
        m_listenDataThread.removeFd(fd);
        STEventCarrier e(new STNetEvent(getIdViaFd(fd), STNetEvent::Type_RemoteDisConnect));
        postEventToReceiver(e);
    }
    void onReceivedClientData(int fd, const STString& dataStr)//call from listenDataThread
    {
        STEventCarrier e(new STNetEvent(getIdViaFd(fd), dataStr));
        postEventToReceiver(e);
    }
    void postEventToReceiver(const STEventCarrier& e)
    {
        if (NULL != m_safeReceiver.receiver && !m_safeReceiver.receiverGuard.isDeleted()) {
            STObject::postGlobalEvent(e, m_safeReceiver.receiver);
        }
        else {
            STObject::postGlobalEvent(e, NULL);
        }
    }

private:
    struct SafeReceiver
    {
        STObject*   receiver;
        STGuard     receiverGuard;
        SafeReceiver()
            : receiver(NULL)
            , receiverGuard()
        {}
    };
    class ListenConnectThread : public STThread//listen client's connect action
    {
    public:
        ListenConnectThread(const STString& name, STServer::Impl* owner)
            : STThread(ServerListenConnectThreadPrefix + name)
            , m_owner(owner)
            , m_listenPort(-1)
        {}
        void setListenPort(int port)
        {
            m_listenPort = port;
        }
        virtual void main();
    private:
        STServer::Impl* m_owner;
        int             m_listenPort;
    };

    class ListenDataThread : public STFdListeenerBase//listen data sent from connected client
    {
    public:
        ListenDataThread(const STString& name, STServer::Impl* owner)
            : STFdListeenerBase(ServerListenConnectThreadPrefix + name)
            , m_owner(owner)
        {}

    private:
        virtual void fdChanged(int fd, FdChangeType changeType)
        {
            std::cout<<"STServer::ListenDataThread::fdChanged(), fd:"<<fd<<"changeType:"<<changeType<<std::endl;
            if (FdChangeType_CanRead == changeType) {
                int readCount = m_fdReader.readData(fd);
                std::cout<<"STServer::ListenDataThread::fdChanged(), fd:"<<fd<<"readCount:"<<readCount<<std::endl;
                if (0 == readCount) {
                    //client disconnected
                    m_fdReader.clearData(fd);
                    m_owner->onClientDisConnected(fd);
                }
                while (m_fdReader.preparedFrameCount() != 0) {
                    SocketFdReader::FrameInfo dataInfo = m_fdReader.getOneFrameData();
                    std::cout<<"STServer::ListenDataThread::fdChanged(),oneFrame, dataStr="<<dataInfo.dataStr<<std::endl;
                    m_owner->onReceivedClientData(dataInfo.fd, dataInfo.dataStr);
                }
            }
            else {
                //err!!!
            }
        }
    private:
        STServer::Impl*     m_owner;
        SocketFdReader      m_fdReader;
    };

private:
    STString        m_serverName;
    int             m_listenPort;
    SafeReceiver    m_safeReceiver;

    ListenConnectThread m_listenConnectThread;
    ListenDataThread    m_listenDataThread;
};



void STServer::Impl::ListenConnectThread::main()
{
    int                 listenfd;
    int                 connfd;
    struct sockaddr_in  servaddr;

    if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 ){
        //printf("create socket error: %s(errno: %d)\n",strerror(errno),errno);
        return;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(m_listenPort);

    if( bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
        //printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
        return;
    }

    if( listen(listenfd, 10) == -1){
        //printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
        return;
    }

    //"waiting for client's request"
    while (1) {
        if( (connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1){
            //printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
            continue;
        }
        m_owner->onClientConnected(connfd);
    }

    close(listenfd);
}






STServer::STServer(const STString& name)
    : m_impl(new Impl(name))
{}

STServer::~STServer()
{}

void STServer::setEventReceiver(STObject* receiver)
{
    m_impl->setEventReceiver(receiver);
}

bool STServer::startListen(int port)
{
    return m_impl->startListen(port);
}

void STServer::stop()
{
    m_impl->stop();
}

bool STServer::sendToClient(const STNetIdentify& target, const STString& dataStr)
{
    return m_impl->sendToClient(target, dataStr);
}





















