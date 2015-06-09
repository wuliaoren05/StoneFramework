#include "net/STNetIdentify.h"
#include "tools/STDataItem.h"

STNetIdentify::STNetIdentify()
    : m_name("")
    , m_ip("")
    , m_port(-1)
{
}

STNetIdentify::~STNetIdentify()
{
}

bool STNetIdentify::operator ==(const STNetIdentify& other)
{
    return (m_name == other.m_name) && (m_ip == other.m_ip)
            && (m_port == other.m_port);
}

STString STNetIdentify::name() const
{
    return m_name;
}

void STNetIdentify::setName(const STString& value)
{
    m_name = value;
}

STString STNetIdentify::ip() const
{
    return m_ip;
}

void STNetIdentify::setIp(const STString& value)
{
    m_ip = value;
}

int STNetIdentify::port() const
{
    return m_port;
}
void STNetIdentify::setPort(int value)
{
    m_port = value;
}

STString STNetIdentify::toSerialString()
{
    STDataItem item("STNetIdentify");
    item.addChild(STDataItem("name", m_name));
    item.addChild(STDataItem("ip", m_ip));
    item.addChild(STDataItem("port", m_port));

    return item.serialToStr();
}

bool STNetIdentify::fromSerialString(const STString& serialString)
{
    bool ret = true;

    STDataItem item("tmp");
    if (item.loadFromStr(serialString)) {
        bool transRet = true;
        STString name = item.child("name").toString(transRet);
        ret &= transRet;
        STString ip = item.child("ip").toString(transRet);
        ret &= transRet;
        int port = item.child("port").toInt(transRet);
        ret &= transRet;

        if (ret) {
            m_name = name;
            m_ip = ip;
            m_port = port;
        }
    }
    else {
        ret = false;
    }

    return ret;
}


















