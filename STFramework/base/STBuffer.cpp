#include "base/STCommonDefine.h"
#include "base/STBuffer.h"


STBuffer::STBuffer(int size)
    : m_data(size, '\0')
{
}

STBuffer::STBuffer(char *buf, int size)
    : m_data(buf, size)
{
}

STBuffer::~STBuffer()
{
}

const char* STBuffer::ptr() const
{
    return m_data.data();
}

void STBuffer::resize(int newSize)
{
    m_data.resize(newSize);
}

int STBuffer::size() const
{
    return m_data.size();
}

void STBuffer::append(char *buf, int size)
{
    STDAssert(NULL != buf && size > 0);
    m_data += std::string(buf, size);
}

void STBuffer::append(const STBuffer &buf)
{
    this->m_data += buf.m_data;
}


