#ifndef ST_RANDOM_H
#define ST_RANDOM_H

#include "base/STCommonDefine.h"

class STRandom
{
public:
    //return an random string with the length of len
    static STString randomLetterString(int len=1);
    //return an random integer which value is in [min, max]
    static int      randomInt(int min=0, int max=10000);
};

#endif // ST_RANDOM_H
