#include <iostream>
#include <list>
#include <unistd.h>

#include "core/STCoreApplication.h"
#include "base/STWaiter.h"
#include "base/STPtr.h"
#include "tools/STDataItem.h"
#include "tools/STStringTool.h"
#include "net/STNetIdentify.h"
#include "thread/STThread.h"

class testThread : public STThread
{
public:
    STWaiter* m_testWaiter;
    void main()
    {
        sleep(7);
        m_testWaiter->setHappen();
    }
};

void waiterTest()
{
    STWaiter waiter;


    testThread thread;
    thread.m_testWaiter = &waiter;
    thread.exec();


    int t = time(NULL);
    STWaitResult ret = waiter.wait(10);
    t = time(NULL) -t;

    thread.join();
    //waiter.wait();

    std::cout<<"waiterTest, ret:"<< ret <<", t="<<t<<std::endl;
}

class TestSharePtrClass
{
public:
    TestSharePtrClass()
        : m_name("TestSharePtrClass")
    {
        std::cout<<("TestSharePtrClass() create");
    }
    ~TestSharePtrClass()
    {
        std::cout<<("~TestSharePtrClass() destroy");
    }

    STString m_name;
};

void testSharePtr()
{
    STSharePtr<TestSharePtrClass> ptr(new TestSharePtrClass);

    {
        STSharePtr<TestSharePtrClass> ptr2 = ptr;
        STSharePtr<TestSharePtrClass> ptr3 = ptr2;
    }
    STSharePtr<TestSharePtrClass> ptr4 = ptr;
    STSharePtr<TestSharePtrClass> ptr5 = ptr;

    std::cout<<("testSharePtr(), m_name:")<<( ptr->m_name.c_str())<<std::endl;
}



class TestCoreApplicationThread : public STThread
{
public:
    void main()
    {
        while (1) {
            STEventCarrier e1(new STEvent("eventType1"));
            STObject::postGlobalEvent(e1);

            STEventCarrier e2(new STEvent("eventType2"));
            STObject::postGlobalEvent(e2);

            STEventCarrier e3(new STEvent("eventType3"));
            STObject::postGlobalEvent(e3);


            STObject::postGlobalEvent(STEventCarrier(new STEvent("eventType1-1")));
            STObject::postGlobalEvent(STEventCarrier(new STEvent("forDel1")));

            STEventCarrier e5(new STEvent("eventType5"));
            STObject::postGlobalEvent(e5);

            usleep(1000*100);
        }

    }
};


class TestEventReceiver : public STObject
{
public:
    TestEventReceiver(const STString& name)
        : m_name(name)
    {}

    virtual void eventHappen(STEventCarrier e)
    {
        std::cout<< m_name << "->eventHappen, e.typeName()=" << e->name() << std::endl;
    }
private:
    STString m_name;
};

void testCoreApplication(int argc, char *argv[])
{
    STCoreApplication app(argc, argv);

    TestEventReceiver receiver1("receiver1");
    receiver1.setReceiveEvent(STEvent("eventType1"));
    receiver1.setReceiveEvent(STEvent("eventType1-1"));
    receiver1.setUnReceiveEvent(STEvent("eventType1-1"));

    TestEventReceiver receiver2("receiver2");
    receiver2.setReceiveEvent(STEvent("eventType2"));

    TestEventReceiver *forDel = new TestEventReceiver("forDel");
    forDel->setReceiveEvent(STEvent("forDel1"));
    delete forDel;

    TestEventReceiver receiver3("receiver3");
    receiver3.setReceiveEvent(STEvent("eventType3"));


    TestCoreApplicationThread thread;
    TestCoreApplicationThread thread2;
    TestCoreApplicationThread thread3;
    thread.exec();
    thread2.exec();
    thread3.exec();

    app.exec();
}

class TestGuardObj : public STObject
{

};

void testGuard()
{
    TestGuardObj* obj = new TestGuardObj();
    STGuard guard = obj->getGuard();
    std::cout<<"guard.isDeleted()="<< (guard.isDeleted() ? "true" : "false") <<std::endl;

    delete obj;
    std::cout<<"guard.isDeleted()="<< (guard.isDeleted() ? "true" : "false") <<std::endl;
}


void testListErase()
{
    int maxIdx = 10;
    int deleteIdx = 6;
    std::list<int> testList;
    for (int i=0; i<maxIdx; ++i) {
        testList.push_back(i);
    }

    std::cout<<"before erase:"<<std::endl;
    std::list<int>::iterator itor = testList.begin();
    for (; itor != testList.end(); ++itor) {
        std::cout<<"  "<<(*itor);
    }
    std::cout<<std::endl;

    itor = testList.begin();
    for (; itor != testList.end(); ++itor) {
        if ( deleteIdx == *itor) {
            testList.erase(itor--);
            //break;
        }
    }

    std::cout<<"after erase:"<<std::endl;
    itor = testList.begin();
    for (; itor != testList.end(); ++itor) {
        std::cout<<"  "<<(*itor);
    }
    std::cout<<std::endl;

}


/*
<root>
    <parent1>
        <child1><int>12</ing></child1>
        <child2><bool>true</bool></child2>
    </parent1>
    <child3><string>this is child3</string></child3>
    <child4><binaryAsBase64>..........</binaryAsBase64></child4>
</root>

*/
#include <base/STBuffer.h>
void testDataItem()
{
    STString folderPath("/home/stone/myCode/build-StoneFramework-Desktop_Qt_5_4_0_GCC_32bit-Debug/test/");
    STString xmlPath1 = folderPath + "dataItem1";
    STString xmlPath2 = folderPath + "dataItem2";

    STDataItem root("root");

    STDataItem parent1("parent1");
    parent1.addChild(STDataItem("child1", 12));
    parent1.addChild(STDataItem("child2", true));

    root.addChild(parent1);
    root.addChild(STDataItem("child3", "this is child3"));

    STBuffer buf(100);
    char* ptr = const_cast<char*>(buf.ptr());
    for(int i=0; i<buf.size(); ++i) {
        ptr[i] = i;
    }
    root.addChild(STDataItem("child4", buf));

    root.saveToFile(xmlPath1);

    STDataItem root2("");
    root2.loadFromFile(xmlPath1);
    root2.saveToFile(xmlPath2);
}

void testSTNetIdentify()
{
    STNetIdentify id1;
    //id1.setName("idName");
    id1.setIp("192.168.1.1");
    id1.setPort(1234);

    std::cout<<id1.toSerialString()<<std::endl;

    STNetIdentify id2;
    id2.fromSerialString(id1.toSerialString());

    //id2.setPort(321);
    std::cout<<id2.toSerialString()<<std::endl;

    std::cout<<"id1 == id2 is:"<<((id1==id2)? "true" : "false")<<std::endl;
}


#include "net/SocketFdReader.h"
#include "net/STNetDefine.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <fcntl.h>

void testSocketFdReader(int argc, char *argv[])
{
    STCoreApplication app(argc, argv);
    if (1 == argc) {
        std::cout<<"run as receiver"<<std::endl;
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
        servaddr.sin_port = htons(54321);

        if( bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
            //printf("bind socket error: %s(errno: %d)\n",strerror(errno),errno);
            return;
        }

        if( listen(listenfd, 10) == -1) {
            //printf("listen socket error: %s(errno: %d)\n",strerror(errno),errno);
            return;
        }

        //"waiting for client's request"
        while (1) {
            if( (connfd = accept(listenfd, (struct sockaddr*)NULL, NULL)) == -1){
                //printf("accept socket error: %s(errno: %d)",strerror(errno),errno);
                continue;
            }
            SocketFdReader reader;
            while (1) {
                fcntl(connfd, F_SETFL, fcntl(connfd, F_GETFD, 0)|O_NONBLOCK);
                reader.readData(connfd);
                if (reader.preparedFrameCount() > 0) {
                    SocketFdReader::FrameInfo oneFrame = reader.getOneFrameData();
                    std::cout<<"received one frame from:"<<oneFrame.fd<<", dataStr:"<<oneFrame.dataStr<<std::endl;
                }
                sleep(1);
            }
        }

        close(listenfd);
    }
    else if (2 == argc) {
        std::cout<<"run as data sender"<<std::endl;

        STString ip = "127.0.0.1";
        int port = 54321;
        struct sockaddr_in server_addr, client_addr;
        socklen_t socklen = sizeof(server_addr);

        int clientFd = -1;
        if((clientFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
                std::cout<<("create socket error, exit!\n")<<std::endl;
                return;
        }
        srand(time(NULL));
        bzero(&client_addr, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_addr.s_addr = htons(INADDR_ANY);

        bzero(&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        inet_aton(ip.c_str(), &server_addr.sin_addr);
        server_addr.sin_port = htons(port);

        if(connect(clientFd, (struct sockaddr*)&server_addr, socklen) < 0)
        {
                std::cout<<"can not connect to "<<argv[1]<<", exit!"<<std::endl;
                std::cout<<strerror(errno)<<std::endl;
                return;
        }

        while (1) {
            STString msg = "hello, from:";
            msg += argv[1];
            int msgSize = msg.size();

            write(clientFd, FrameHeadTag, sizeof(FrameHeadTag));
            write(clientFd, &msgSize, sizeof(msgSize));
            write(clientFd, msg.c_str(), msgSize);

            sleep(1);
        }
    }


    app.exec();
}

#include "net/STServer.h"
#include "net/STClient.h"
#include "net/STNetEvent.h"
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


void testNet(int argc, char *argv[])
{
    STCoreApplication app(argc, argv);

    if (1 == argc) {
        std::cout<<"run as server"<<std::endl;
        ServerReceiver receiver;
        STServer server("testServer");
        receiver.m_server = &server;

        server.setEventReceiver(&receiver);
        server.startListen(12345);

        app.exec();
    }
    else if (2 == argc) {
        std::cout<<"run as client"<<std::endl;
        ClientReceiver receiver;
        STClient client("testClient");

        receiver.m_client = &client;
        client.setEventReceiver(&receiver);

        bool connectRet = client.connectToServer("10.226.102.102", 12345);
        if (!connectRet) {
            std::cout<<"connect failed!"<<std::endl;
        }
        else {
            client.sendToServer("1");
            //sleep(1);
        }

        app.exec();
    }
}


void testStringTool()
{
    bool transRet = false;
    bool transRet2 = false;

    int value = 789455;
    int value2 = STStringTool::strToInt(STStringTool::intToStr(value, transRet), transRet2);
    STDAssert(value == value2);

    STString str = "1235778";
    STString str2 = STStringTool::intToStr(STStringTool::strToInt(str, transRet), transRet2);
    STDAssert(str == str2);

//    double doubleValue = 3.14159265;
//    double doubleValue2 = STStringTool::strToDouble(STStringTool::doubleToStr(doubleValue, transRet), transRet2);
//    STDAssert(doubleValue-doubleValue2 > -0.0001  &&  doubleValue-doubleValue2 < 0.0001);

    STString doubleStr = "3.14159";
    STString doubleStr2 = STStringTool::doubleToStr(STStringTool::strToDouble(doubleStr, transRet), transRet2);
    STDAssert(doubleStr == doubleStr2);


}

#include "tools/STIOTools.h"
void testIOTools()
{
    while (1) {
        STString str = STConsoleTool::readOneLine();

        STConsoleTool::writeLine("your input:" + str);
    }
}

#include "tools/STRandom.h"
void testRandomTools()
{
    for (int i=1; i<=100; ++i) {
        STConsoleTool::writeLine(STRandom::randomLetterString(i));
    }
    STConsoleTool::writeLine("");
    for (int i=0; i<100; ++i) {
        STConsoleTool::write( STStringTool::intToStr(STRandom::randomInt(0, 100)) );
        STConsoleTool::write(", ");
    }
}


#include "tools/STBase64.h"
void testBase64()
{
    STBuffer buf(10);
    char* ptr = const_cast<char*>(buf.ptr());
    for(int i=0; i<buf.size(); ++i) {
        ptr[i] = i;
    }

    STString base64Str1 = STBase64::encode(buf);
    STString base64Str2 = STBase64::encode(STBase64::decode(base64Str1)) ;
    STDAssert( base64Str1 == base64Str2);
}

#include "log/STLog.h"
void testLog()
{
    LogDebug()<<"hello, this is log!"<<STLog::endl;
    LogDebug()<<"hello, this is log!"<<123<<12.3123<<STLog::endl;
}

int main(int argc, char *argv[])
{
    STUNUSED(argc);
    STUNUSED(argv);

    //fdwaiterTest();
    //waiterTest();
    //testSharePtr();
    //testGuard();
    //testListErase();
    //testCoreApplication(argc, argv);
    //testDataItem();
    //testSTNetIdentify();
    //testSocketFdReader(argc, argv);
    //testNet(argc, argv);
    //testStringTool();
    //testIOTools();
    //testRandomTools();
    //testBase64();
    testLog();

    return 0;
}











