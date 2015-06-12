#include <iostream>

#include "core/STCoreApplication.h"
#include "net/STServer.h"
#include "net/STClient.h"
#include "net/STNetEvent.h"
#include "tools/STStringTool.h"



class ClientReceiver : public STObject
{
public:
    virtual void eventHappen(STEventCarrier e)
    {
        static int lastSendInt = 1;
        if (e->name() == STNetEvent::eventName()) {
            STNetEvent* netEvent = (STNetEvent*)e.get();
            switch (netEvent->type()) {
            case STNetEvent::Type_RemoteDisConnect:
                std::cout<<"ClientReceiver::eventHappen(), sender:"<<netEvent->sender().ip()<<
                        "<"<<netEvent->sender().port()<<"  server disconnected"<<" atTime:"<<time(NULL)<<std::endl;
                break;

            case STNetEvent::Type_DataFromClient:
                std::cout<<"ClientReceiver::eventHappen(), sender:"<<netEvent->sender().ip()<<
                        "<"<<netEvent->sender().port()<<"  dataStr:"<<netEvent->dataStr()<<" atTime:"<<time(NULL)<<std::endl;
                //STDAssert(lastSendInt == STStringTool::strToInt(netEvent->dataStr())-1);
                lastSendInt += 2;
                m_client->sendToServer( STStringTool::intToStr(lastSendInt) );
                break;

            default:
                break;
            }


        }
    }

    STClient* m_client;
};

int main(int argc, char *argv[])
{
    STCoreApplication app(argc, argv);

    ClientReceiver receiver;
    STClient client("demoClient");

    receiver.m_client = &client;
    client.setEventReceiver(&receiver);

    bool connectRet = client.connectToServer("127.0.0.1", 12345);
    if (!connectRet) {
        std::cout<<"connect failed!"<<std::endl;
    }
    else {
        client.sendToServer("1");
        //sleep(1);
    }


    return app.exec();
}


