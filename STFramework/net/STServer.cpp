#include "net/STServer.h"
#include "thread/STThread.h"

class STServer::Impl : public STThread
{
public:
    Impl(const STString& name)
        : STThread(name)
    {}
    ~Impl(){}

    void main()
    {

    }

    bool startListen(const STString& ip, int port)
    {
        //TBD
        STUNUSED(ip);
        STUNUSED(port);
        return false;
    }

private:
    STString    m_listenIp;
    int         m_listenPort;
};

STServer::STServer(const STString& name)
    : STObject()
    , m_impl(new Impl(name))
{

}

STServer::~STServer()
{

}

void STServer::setEventReceiver(STObject* receiver)
{
    //TBD
    STUNUSED(receiver);
}

bool STServer::startListen(const STString& ip, int port)
{
    bool ret = false;

    STUNUSED(ip);
    STUNUSED(port);

    return ret;
}

bool STServer::sendToClient(const STNetIdentify& target, const STString& dataStr)
{
    bool ret = false;

    STUNUSED(target);
    STUNUSED(dataStr);

    return ret;
}






