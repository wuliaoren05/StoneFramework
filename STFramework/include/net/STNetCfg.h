#ifndef _STOPTCFG_H_
#define _STOPTCFG_H_
#include <map>
#include <vector>
#include <stdint.h>
#include <sys/types.h>
#include<sstream>

#include "base/STCommonDefine.h"
#include "tools/inifile.h"

typedef struct {
	uint32_t uThreadNum;
	uint32_t uEventNum;
} SNumFlag;

typedef struct SNet_t {
	bool operator<(const SNet_t& sNet_t) const {
		if (this->iPort < sNet_t.iPort)
			return true;
		else {
			int iRet = strcmp(this->strIP.c_str(), sNet_t.strIP.c_str());
			if (iRet < 0)
				return true;
			else
				return false;
		}
	}
	STString strIP;
	int32_t iPort;
} SNet;
typedef struct SNetCommonCfg_t {
	SNetCommonCfg_t() {
		m_iNetNum = 0;
		strCommon = "Common";
		strNetNum = "IPNum";
	}
	STString strNetNum;
	STString strCommon;
	int32_t m_iNetNum;
} SNetCommonCfg;

typedef struct SNetData_t {
	char szIP[128];
	int32_t iPort;
} SNetData;
typedef struct SNetDataCfg_t {
	SNetDataCfg_t() {
		strIP = "IP";
		strPort = "Port";
	}
	STString strIP;
	STString strPort;
	std::vector<SNetData> m_vectorIP;
} SNetDataCfg;

typedef struct SThreadCfg_t {
	SThreadCfg_t() {
		strListenThread = "ListenThreadNum";
		strRunThread = "RunThreadNum";
		m_iListenThread = 0;
		m_iRunThread = 0;
	}
	STString strListenThread;
	STString strRunThread;
	int32_t m_iListenThread;
	int32_t m_iRunThread;
} SThreadCfg;

typedef std::map<int32_t, SNet> 	SMapS2Net;


class CEpollCfg {
public:
	CEpollCfg();
	~CEpollCfg();
	int32_t init(const STString& strFile);
	int32_t run();
	int32_t destroy();
public:
	STString m_strFile;
	SNetCommonCfg m_sNetCommonCfg;
	SThreadCfg m_sThreadCfg;
	std::map<SNetCommonCfg, int> m_mapIPNum;
	SNetDataCfg m_sNetDataCfg;
};


#endif
