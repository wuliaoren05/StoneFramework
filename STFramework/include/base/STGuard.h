#ifndef ST_GUARD_H
#define ST_GUARD_H

#include "base/STPtr.h"

class STGuard
{
public:
    STGuard();
    STGuard(const STGuard& other);
    ~STGuard();

    STGuard& operator =(const STGuard& other);

    bool isDeleted() const;
    STGuard getListeenGuard()  const;

private:
    bool m_isMainObj;
    STSharePtr<bool> m_isMainObjDeleted;
};

#endif // ST_GUARD_H
