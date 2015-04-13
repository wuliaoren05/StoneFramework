#ifndef ST_NETEPOLL_H
#define ST_NETEPOLL_H

#include "STNetMode.h"

class STScopePtr;

class STNetEpoll: public STNetMode
{
public:
	virtual bool Init();
	virtual bool Run();
	virtual bool Destroy();
private:
	class Impl;
	STScopePtr<Impl> m_impl;
};
#endif
