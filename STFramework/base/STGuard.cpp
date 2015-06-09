#include "base/STGuard.h"

STGuard::STGuard()
    : m_isMainObj(true)
    , m_isMainObjDeleted(new bool(false))
{}

STGuard::STGuard(const STGuard& other)
    : m_isMainObj(other.m_isMainObj)
    , m_isMainObjDeleted(NULL)
{
    if (this->m_isMainObj) {
        this->m_isMainObjDeleted = new bool(false);
    }
    else {
        this->m_isMainObjDeleted = other.m_isMainObjDeleted;
    }
}

STGuard::~STGuard()
{
    if (m_isMainObj) {
        *(m_isMainObjDeleted.get()) = true;
    }
}

STGuard& STGuard::operator =(const STGuard& other)
{
    this->m_isMainObj = other.m_isMainObj;
    if (this->m_isMainObj) {
        this->m_isMainObjDeleted = new bool(false);
    }
    else {
        this->m_isMainObjDeleted = other.m_isMainObjDeleted;
    }

    return *this;
}

bool STGuard::isDeleted() const
{
    return *(m_isMainObjDeleted.get());
}

STGuard STGuard::getListeenGuard() const
{
    STGuard guard;
    guard.m_isMainObj = false;
    guard.m_isMainObjDeleted = this->m_isMainObjDeleted;

    return guard;
}






