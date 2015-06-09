#ifndef ST_SERVER_H
#define ST_SERVER_H

#include "base/STPtr.h"
#include "core/STObject.h"
#include "net/STNetIdentify.h"

class STServer : public STObject
{
public:
    STServer(const STString& name);
    ~STServer();

    void setEventReceiver(STObject* receiver);//event will send as STNetEvent
    bool startListen(const STString& ip, int port);//will run another thread to listen

    bool sendToClient(const STNetIdentify& target, const STString& dataStr);

private:
    STServer(const STServer& other);
    bool operator==(const STServer& other);
    bool operator=(const STServer& other);

private:
    class Impl;
    STScopePtr<Impl> m_impl;
};

#endif // ST_SERVER_H
