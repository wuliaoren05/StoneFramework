#ifndef ST_NETEPOLL_H
#define ST_NETEPOLL_H

#include "STNetMode.h"

STMutex::STMutex()
    : m_data(new PrivateData)
{
}

class STScopePtr;
class STNetEpoll: public STNetMode {
public:
	virtual int32_t init(const STString& strFileName) {
		void* ptr = new Impl;
		if (NULL == ptr) {
			//log
			return ERROR;
		}

		m_impl= ptr;
		return m_impl->init(strFileName);
	};
	virtual int32_t run() {
		return m_impl->run();
	}
	;
	virtual int32_t destroy() {
		return m_impl->destroy();
	}
	;
private:
	~ STNetEpoll() {
	}
private:
	class Impl;
	STScopePtr<Impl> m_impl;
};
#endif
