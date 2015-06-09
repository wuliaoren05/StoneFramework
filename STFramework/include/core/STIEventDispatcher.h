#ifndef ST_I_EVENT_DISPATCHER_H
#define ST_I_EVENT_DISPATCHER_H

#include "event/STEvent.h"
#include "core/STObject.h"

class STObject;
class STIEventDispatcher
{
public:
    virtual ~STIEventDispatcher(){}
    virtual void postEvent(STEventCarrier e, STObject* receiver = NULL) = 0;
    virtual bool sendEvent(STEventCarrier e, STObject* receiver = NULL) = 0;

    virtual void addEventFilter(STObject* filter, STEvent e) = 0;
    virtual void removeEventFilter(STObject* filter, STEvent e) = 0;
};


#endif // ST_I_EVENT_DISPATCHER_H
