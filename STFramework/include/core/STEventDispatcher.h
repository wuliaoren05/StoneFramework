#ifndef ST_EVENT_DISPATCHER_H
#define ST_EVENT_DISPATCHER_H

#include "event/STEvent.h"
#include "core/STObject.h"
#include "core/STIEventDispatcher.h"


class STEventDispatcher : public STIEventDispatcher
{
public:
    STEventDispatcher();
    virtual ~STEventDispatcher();

    virtual void postEvent(STEventCarrier e, STObject* receiver = NULL);
    virtual bool sendEvent(STEventCarrier e, STObject* receiver = NULL);

    virtual void addEventFilter(STObject* filter, STEvent e);
    virtual void removeEventFilter(STObject* filter, STEvent e);

    STExitReason startEventLoop();
    void exitEventLoop(STExitReason reason=STExitReason_Normal);

private:
    class EventDispatcherPrivateData;
    STScopePtr<EventDispatcherPrivateData> m_data;
};




#endif // ST_EVENT_DISPATCHER_H
