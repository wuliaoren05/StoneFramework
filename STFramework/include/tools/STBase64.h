#ifndef ST_BASE64_H
#define ST_BASE64_H

#include "base/STCommonDefine.h"
#include "base/STBuffer.h"

class STBase64
{
public:
    static STString encode(const char* dataBuf, int dataLen);
    static STString encode(const STBuffer& dataBuf);

    static STBuffer decode(const STString& str);
};

#endif // ST_BASE64_H
