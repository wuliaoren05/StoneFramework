#ifndef ST_CONNECT_IDENTIFY_H
#define ST_CONNECT_IDENTIFY_H

#include "base/STCommonDefine.h"
#include "base/STPtr.h"

class STNetIdentify
{
public:
    STNetIdentify();
    ~STNetIdentify();

    bool operator ==(const STNetIdentify&other);

    STString name() const;
    void setName(const STString& value);
    STString ip() const;
    void setIp(const STString& value);
    int port() const;
    void setPort(int value);

    STString toSerialString();
    bool fromSerialString(const STString& serialString);

private:
    STString    m_name;
    STString    m_ip;
    int         m_port;
};


#endif // ST_CONNECT_IDENTIFY_H
