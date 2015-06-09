/*
 * STOptCfg.cpp
 *
 *  Created on: May 24, 2015
 *      Author: root
 */

#ifndef _STOPTCFG_CPP_
#define  _STOPTCFG_CPP_

#include "net/STNetCfg.h"

CEpollCfg::CEpollCfg()
{
}
CEpollCfg::~CEpollCfg()
{
}
int32_t CEpollCfg::init(const STString& strFile)
{
	m_strFile = strFile;
	m_sNetCommonCfg.m_iNetNum = read_profile_int(
			m_sNetCommonCfg.strCommon.c_str(), m_sNetCommonCfg.strNetNum.c_str(), 0,
			m_strFile.c_str());

	printf("m_iNetNum:%d\n", m_sNetCommonCfg.m_iNetNum);
	if (0 >= m_sNetCommonCfg.m_iNetNum) {
		//log
		return RETERROR;
	}

	m_sThreadCfg.m_iListenThread = read_profile_int(
			m_sNetCommonCfg.strCommon.c_str(), m_sThreadCfg.strListenThread.c_str(),
			0, m_strFile.c_str());

	printf("m_sThreadCfg.m_iListenThread:%d\n", m_sThreadCfg.m_iListenThread);
	if (0 >= m_sThreadCfg.m_iListenThread) {
		//log
		return RETERROR;
	}
	m_sThreadCfg.m_iRunThread = read_profile_int(
			m_sNetCommonCfg.strCommon.c_str(), m_sThreadCfg.strRunThread.c_str(), 0,
			m_strFile.c_str());
	printf("m_sThreadCfg.m_iRunThread:%d\n", m_sThreadCfg.m_iRunThread);
	if (0 >= m_sThreadCfg.m_iRunThread) {
		//log
		return RETERROR;
	}

	return RETOK;
}
int32_t CEpollCfg::run()
{
	int32_t iRet = 0;
	SNetData sNetData;
	m_sNetDataCfg.m_vectorIP.clear();
	std::stringstream STStrNum;
	while (m_sNetCommonCfg.m_iNetNum > 0) {
		STStrNum.str("");
		STStrNum << "IP_" << m_sNetCommonCfg.m_iNetNum;
		printf("STStrNum.str().c_str():%s\n", STStrNum.str().c_str());
		iRet = read_profile_string(STStrNum.str().c_str(),
				m_sNetDataCfg.strIP.c_str(), sNetData.szIP, sizeof(sNetData.szIP), "",
				m_strFile.c_str());
		if (iRet < 0) {
			//log
			return RETERROR;
		}
		printf("sNetData-szIP:%s\n", sNetData.szIP);

		sNetData.iPort = read_profile_int(STStrNum.str().c_str(),
				m_sNetDataCfg.strPort.c_str(), 0, m_strFile.c_str());
		if (0 >= sNetData.iPort) {
			//log
			return RETERROR;
		}
		printf("sNetData.iPort:%d\n", sNetData.iPort);
		m_sNetDataCfg.m_vectorIP.push_back(sNetData);
		--m_sNetCommonCfg.m_iNetNum;
	}
	return RETOK;
}
int32_t CEpollCfg::destroy()
{
	return RETOK;
}
#endif
