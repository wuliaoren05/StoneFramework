#include <stdlib.h>
#include <sys/time.h>

#include "tools/STRandom.h"




STString STRandom::randomLetterString(int len)
{
    STDAssert(len > 0);
    STString ret;
    srand(time(NULL));
    while (len>0) {
        char a = randomInt('a', 'z');
        if (randomInt(0, 1) == 0) {
            a -= ('a' - 'A');
        }
        ret.push_back(a);
        --len;
    }
    return ret;
}

int STRandom::randomInt(int min, int max)
{
    STDAssert(min <= max);
    timeval tv;
    gettimeofday (&tv , NULL);
    srand(tv.tv_usec);
    return (rand() % (max-min+1)) + min;
}
