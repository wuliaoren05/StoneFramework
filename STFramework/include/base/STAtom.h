#ifndef ST_ATOM_H
#define ST_ATOM_H

//#define WIN32

class STAtom
{
public:

    //increase
    inline static int Inc(int * var)
    {
#ifdef WIN32
        return _InterlockedIncrement((long volatile *)(var));
#else
        return __sync_add_and_fetch(var, 1);
#endif
    }

    //decline
    inline static int Dec(int * var)
    {
#ifdef WIN32
        return _InterlockedDecrement((long volatile *)(var));
#else
        return __sync_add_and_fetch(var, -1);
#endif
    }

    //add one value
    inline static int Add(int * var, const int value)
    {
#ifdef WIN32
        return _InterlockedExchangeAdd((long volatile *)(var),(long)value);
#else
        return __sync_fetch_and_add(var, value);
#endif
    }


    //set value
    inline static void Set(int * var, const int value)
    {
#ifdef WIN32
        _InterlockedExchange((long volatile *)(var), (long)value);
#else
        __sync_lock_test_and_set(var, value);
#endif
    }

    //get value
    inline static int Get(int * var)
    {
#ifdef WIN32
        return _InterlockedExchangeAdd((long volatile *)(var), 0);
#else
        return __sync_fetch_and_add(var, 0);
#endif
    }


private:
    STAtom(void);
    ~STAtom(void);
};


#endif // ST_ATOM_H
