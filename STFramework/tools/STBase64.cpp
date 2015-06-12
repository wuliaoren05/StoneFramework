#include "tools/STBase64.h"



STString STBase64::encode(const char *dataBuf, int dataLen)
{
    static const char Base64HashTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    STString ret;

    if (dataLen <= 0 || NULL == dataBuf) {
        return ret;
    }

    unsigned char tmpArr[4] = {0};
    int lineLength =0 ;
    for(int i=0; i<(int)(dataLen / 3); i++) {
        tmpArr[1] = *dataBuf++;
        tmpArr[2] = *dataBuf++;
        tmpArr[3] = *dataBuf++;
        ret += Base64HashTable[tmpArr[1] >> 2];
        ret += Base64HashTable[((tmpArr[1] << 4) | (tmpArr[2] >> 4)) & 0x3F];
        ret += Base64HashTable[((tmpArr[2] << 2) | (tmpArr[3] >> 6)) & 0x3F];
        ret += Base64HashTable[tmpArr[3] & 0x3F];
        if (lineLength += 4, 76 == lineLength) {
            ret += "\r\n";
            lineLength = 0;
        }
    }

    int modNum = dataLen % 3;
    switch (modNum) {
    case 1:
    {
        tmpArr[1] = *dataBuf++;
        ret += Base64HashTable[(tmpArr[1] & 0xFC) >> 2];
        ret += Base64HashTable[((tmpArr[1] & 0x03) << 4)];
        ret += "==";
        break;
    }
    case 2:
    {
        tmpArr[1] = *dataBuf++;
        tmpArr[2] = *dataBuf++;
        ret += Base64HashTable[(tmpArr[1] & 0xFC) >> 2];
        ret += Base64HashTable[((tmpArr[1] & 0x03) << 4) | ((tmpArr[2] & 0xF0) >> 4)];
        ret += Base64HashTable[((tmpArr[2] & 0x0F) << 2)];
        ret += "=";
        break;
    }
    default:
        //can not go to here
        break;
    }

    return ret;
}

STString STBase64::encode(const STBuffer &dataBuf)
{
    return encode(dataBuf.ptr(), dataBuf.size());
}

STBuffer STBase64::decode(const STString &str)
{
    static const char Base64DecodeTable[] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        62, // '+'
        0, 0, 0,
        63, // '/'
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
        0, 0, 0, 0, 0, 0, 0,
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
        13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
        0, 0, 0, 0, 0, 0,
        26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
        39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
    };

    STBuffer ret(0);

    int nValue;
    size_t i = 0;
    while (i < str.size()) {
        if (str.at(i)!= '\r' && str.at(i) != '\n') {
            nValue = Base64DecodeTable[(int)str.at(i++)] << 18;
            nValue += Base64DecodeTable[(int)str.at(i++)] << 12;
            char tmpValue = (nValue & 0x00FF0000) >> 16;
            ret.append(&tmpValue, sizeof(tmpValue));
            if (str.at(i) != '=') {
                nValue += Base64DecodeTable[(int)str.at(i++)] << 6;
                tmpValue = (nValue & 0x0000FF00) >> 8;
                ret.append(&tmpValue, sizeof(tmpValue));
                if (str.at(i) != '=') {
                    nValue += Base64DecodeTable[(int)str.at(i++)];
                    tmpValue = nValue & 0x000000FF;
                    ret.append(&tmpValue, sizeof(tmpValue));
                }
                else {
                    break;
                }
            }
            else {
                break;
            }
        }
        else {
            ++i;
        }
     }

    return ret;
}























