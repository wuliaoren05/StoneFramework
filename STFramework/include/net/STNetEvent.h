#ifndef ST_CONNECT_EVENT_H
#define ST_CONNECT_EVENT_H

#include "event/STEvent.h"
#include "net/STNetIdentify.h"


class STNetEvent : public STEvent
{
public:
    static STString eventName()
    {
        return "STConnectEvent";
    }

    enum Type
    {
        Type_ClientConnected = 0,
        Type_ClientDisConnect,
        Type_DataFromClient,

        Type_Count
    };

public:
    STNetEvent(const STNetIdentify& sender, const STString& dataStr)
        : STEvent(eventName())
        , m_type(Type_Count)
        , m_sender(sender)
        , m_data(dataStr)
    {}

    ~STNetEvent(){}

    Type type()
    {
        return m_type;
    }

    inline STNetIdentify sender() const
    {
        return m_sender;
    }

    inline STString dataStr() const
    {
        return m_data;
    }

private:
    Type            m_type;
    STNetIdentify   m_sender;
    STString        m_data;
};

#endif // ST_CONNECT_EVENT_H
