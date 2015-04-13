#ifndef ST_OBJECT_H
#define ST_OBJECT_H

#include "event/STEvent.h"
#include "base/STGuard.h"
#include "core/STIEventDispatcher.h"

class STObject
{
public:
    STObject();
    virtual ~STObject();

    virtual void eventHappen(STEventCarrier e);
    void setReceiveEvent(STEvent e);
    void setUnReceiveEvent(STEvent e);

    STGuard getGuard() const;

private:
    STGuard m_guard;

 //static, for event
public:
    static void postGlobalEvent(STEventCarrier e, STObject* receiver = NULL);
    static bool sendGlobalEvent(STEventCarrier e, STObject* receiver = NULL);
protected:
    static void setGlobalEventDispatcher(STIEventDispatcher* disPatcher);
private:
    static STIEventDispatcher* s_eventDispatcher;
};

#endif // ST_OBJECT_H





