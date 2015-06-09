#include <list>
#include <map>

#include "core/STEventDispatcher.h"
#include "base/STCommonDefine.h"
#include "base/STWaiter.h"
#include "base/STMutex.h"



class STEventDispatcher::EventDispatcherPrivateData
{
public:
    class EventPackage
    {
    public:
        EventPackage(STEventCarrier e, STObject* receiver=NULL)
            : m_event(e)
            , m_receiver(receiver)
        {
            if (NULL != m_receiver) {
                m_receiverGuard = m_receiver->getGuard();
            }
        }

        STObject* getReceiver() const
        {
            STObject* ret = NULL;
            if (NULL != m_receiver && !m_receiverGuard.isDeleted()) {
                ret = m_receiver;
            }
            return ret;
        }

        STEventCarrier getEvent() const
        {
            return m_event;
        }

    private:
        STEventCarrier  m_event;
        STObject*       m_receiver;
        STGuard         m_receiverGuard;
    };

    class SafeEventFilter
    {
    public:
        SafeEventFilter(STObject* filter)
            : m_filter(filter)
            , m_eventFilterGuard(m_filter->getGuard())
        {}

        STObject* getFilter()
        {
            STObject* ret = NULL;
            if (!m_eventFilterGuard.isDeleted()) {
                ret = m_filter;
            }
            return ret;
        }

        bool operator ==(const SafeEventFilter& other)
        {
            return m_filter == other.m_filter;
        }

    private:
        STObject*   m_filter;
        STGuard     m_eventFilterGuard;
    };

    typedef std::list<SafeEventFilter> FilterList;
    typedef std::map<STString, FilterList> FilterMap;

public:
    EventDispatcherPrivateData()
        : m_exitReason(STExitReason_Normal)
        , m_needExit(false)
    {}

    void setNeedExit(bool isNeed)
    {
        m_needExit = isNeed;
    }

    void setExitReason(STExitReason reason)
    {
        m_exitReason = reason;
    }

    void setEventHappen()
    {
        m_eventWaiter.setHappen();
    }

    void postEventPkg(const EventPackage& ePkg)
    {
        m_eventMutex.lock();
        m_eventList.push_back(ePkg);
        m_eventMutex.unlock();

        m_eventWaiter.setHappen();
    }

    void sendEventPkg(const EventPackage& ePkg)
    {
        disPatchEventPkg(ePkg);
    }

    STExitReason waiteAndDealEvent()
    {
        STExitReason ret = STExitReason_Normal;

        while (1) {
            while (m_eventList.size() != 0) {
                m_eventMutex.lock();
                const EventDispatcherPrivateData::EventPackage e = m_eventList.front();
                m_eventList.pop_front();
                if (0 == m_eventList.size()) {
                    m_eventWaiter.reset();
                }
                m_eventMutex.unlock();

                disPatchEventPkg(e);
            }

            if (m_needExit) {
                ret = m_exitReason;
                break;
            }

            m_eventWaiter.wait();
        }

        return ret;
    }

    bool disPatchEventPkg(const EventPackage& ePkg)
    {
        if (NULL != ePkg.getReceiver())  {
            ePkg.getReceiver()->eventHappen(ePkg.getEvent());
        }
        else {
            FilterMap::iterator mapItor = m_typeToFilter.find(ePkg.getEvent()->name());
            if ( mapItor != m_typeToFilter.end() ) {
                FilterList::iterator listItor = mapItor->second.begin();
                for (; listItor != mapItor->second.end(); ++listItor) {
                    if (NULL != listItor->getFilter()) {
                        listItor->getFilter()->eventHappen(ePkg.getEvent());
                    }
                    else {
                        mapItor->second.erase(listItor--);//remove this unavailable filter
                    }
                }
            }
        }

        return ePkg.getEvent()->isReceived();
    }

    void addEventFilter(STObject* filter, STEvent e)
    {
        FilterMap::iterator mapItor = m_typeToFilter.find(e.name());
        if ( mapItor != m_typeToFilter.end() ) {
            FilterList::iterator listItor = mapItor->second.begin();
            for (; listItor != mapItor->second.end(); ++listItor) {
                if (NULL != listItor->getFilter()) {
                    if (listItor->getFilter() == filter) {
                        break;//already in filter list
                    }
                }
                else {
                    mapItor->second.erase(listItor--);//remove this unavailable filter
                }
            }
            if ( listItor == mapItor->second.end() ) {
                mapItor->second.push_back(SafeEventFilter(filter));
            }
        }
        else {
            FilterList filterList;
            filterList.push_back(SafeEventFilter(filter));
            m_typeToFilter[e.name()] = filterList;
        }
    }

    void removeEventFilter(STObject* filter, STEvent e)
    {
        FilterMap::iterator mapItor = m_typeToFilter.find(e.name());
        if ( mapItor != m_typeToFilter.end() ) {
            FilterList::iterator listItor = mapItor->second.begin();
            for (; listItor != mapItor->second.end(); ++listItor) {
                if ( (NULL == listItor->getFilter()) || (listItor->getFilter() == filter) ) {
                    mapItor->second.erase(listItor--);
                    break;
                }
            }
        }
    }
private:
    STWaiter                    m_eventWaiter;
    STMutex                     m_eventMutex;
    std::list<EventPackage>     m_eventList;

    FilterMap                   m_typeToFilter;

    STExitReason                m_exitReason;
    bool                        m_needExit;

};

STEventDispatcher::STEventDispatcher()
    : m_data(new EventDispatcherPrivateData())
{
}

STEventDispatcher::~STEventDispatcher()
{
}


STExitReason STEventDispatcher::startEventLoop()
{
    STExitReason ret = STExitReason_Normal;

    ret = m_data->waiteAndDealEvent();

    return ret;
}

void STEventDispatcher::exitEventLoop(STExitReason reason)
{
    m_data->setExitReason(reason);
    m_data->setNeedExit(true);

    m_data->setEventHappen();
}

void STEventDispatcher::postEvent(STEventCarrier e, STObject* receiver)
{
    EventDispatcherPrivateData::EventPackage eventPkg(e, receiver);
    m_data->postEventPkg(eventPkg);
}

bool STEventDispatcher::sendEvent(STEventCarrier e, STObject* receiver)
{
    EventDispatcherPrivateData::EventPackage eventPkg(e, receiver);
    m_data->sendEventPkg(eventPkg);

    return eventPkg.getEvent()->isReceived();
}

void STEventDispatcher::addEventFilter(STObject* filter, STEvent e)
{
    m_data->addEventFilter(filter, e);
}

void STEventDispatcher::removeEventFilter(STObject* filter, STEvent e)
{
    m_data->removeEventFilter(filter, e);
}












