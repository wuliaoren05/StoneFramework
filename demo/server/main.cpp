#include <iostream>

#include "core/STCoreApplication.h"
#include "net/STServer.h"
#include "net/STClient.h"
#include "net/STNetEvent.h"
#include "tools/STStringTool.h"

class ServerReceiver : public STObject
{
public:
    virtual void eventHappen(STEventCarrier e)
    {
        static int lastSendInt = 0;
        if (e->name() == STNetEvent::eventName()) {
            STNetEvent* netEvent = (STNetEvent*)e.get();
            switch (netEvent->type()) {
            case STNetEvent::Type_RemoteConnected:
                std::cout<<"ServerReceiver::eventHappen(),Type_RemoteConnected,sender:"<<netEvent->sender().ip()<<
                        "<"<<netEvent->sender().port()<<" atTime:"<<time(NULL)<<std::endl;
                break;

            case STNetEvent::Type_RemoteDisConnect:
                std::cout<<"ServerReceiver::eventHappen(),Type_RemoteDisConnect,sender:"<<netEvent->sender().ip()<<
                        "<"<<netEvent->sender().port()<<" atTime:"<<time(NULL)<<std::endl;
                break;

            case STNetEvent::Type_DataFromClient:
            {
                std::cout<<"ServerReceiver::eventHappen(),Type_DataFromClient,sender:"<<netEvent->sender().ip()<<
                        "<"<<netEvent->sender().port()<<"  dataStr:"<<netEvent->dataStr()<<" atTime:"<<time(NULL)<<std::endl;
                //STDAssert(lastSendInt == STStringTool::strToInt(netEvent->dataStr())-1);
                lastSendInt += 2;
                m_server->sendToClient(netEvent->sender(), STStringTool::intToStr(lastSendInt) );
                break;
            }
            default:
                break;
            }
        }
    }

    STServer* m_server;
};

int main(int argc, char *argv[])
{
    STCoreApplication app(argc, argv);

    ServerReceiver receiver;
    STServer server("demoServer");
    receiver.m_server = &server;

    server.setEventReceiver(&receiver);
    server.startListen(12345);

    app.exec();


    return app.exec();
}

