#ifndef ST_EVENT_H
#define ST_EVENT_H

#include "base/STCommonDefine.h"
#include "base/STPtr.h"

class STEvent
{
public:
    STEvent(const STString& typeName = "");
    virtual ~STEvent(){}

    STString name() const;
    bool isReceived() const;

    void accept();

private:
    STString    m_typeName;
    bool        m_isAccepted;
};

typedef STSharePtr<STEvent> STEventCarrier;

#endif // ST_EVENT_H
