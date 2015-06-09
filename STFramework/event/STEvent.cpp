#include "event/STEvent.h"


STEvent::STEvent(const STString& typeName)
    : m_typeName(typeName)
    , m_isAccepted(false)
{
}

STString STEvent::name() const
{
    return m_typeName;
}

bool STEvent::isReceived() const
{
    return m_isAccepted;
}

void STEvent::accept()
{
    m_isAccepted = true;
}
