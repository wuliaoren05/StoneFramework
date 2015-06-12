#ifndef ST_BUFFER_H
#define ST_BUFFER_H

#include <string>

class STBuffer
{
public:
    STBuffer(int size=0);
    STBuffer(char* buf, int size);
    ~STBuffer();

    const char* ptr() const;
    void resize(int newSize);
    int size() const;

    void append(char* buf, int size);
    void append(const STBuffer& buf);

private:
    std::string m_data;
};

#endif // ST_BUFFER_H
