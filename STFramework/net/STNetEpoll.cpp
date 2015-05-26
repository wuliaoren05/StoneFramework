#include <stdlib.h>
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include <vector>
#include <set>
#include <map>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <string>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <pthread.h>
#include <errno.h>

#include "net/STNetEpoll.h"


enum {
	NULLSTS = -1
};
enum {
	LISTEN = 1, RUN = 2
};

class CEpollCfg;

class CNetCmdOpt {
public:
	int initCmd(std::map<STString, void*>& mapCmdMapAct) {
		mapCmdMapAct["EXIT"] = NULL;
		mapCmdMapAct["CLOSE"] = NULL;
		return RETOK;
	}
private:
};


class CEpollListenThread: public STThread {
public:
	CEpollListenThread() :
			m_netThreadRes(NULL), m_netEpoll(NULL) {

	}
	CEpollListenThread& operator=(const CEpollListenThread& other) {
		m_netThreadRes = other.m_netThreadRes;
		m_netEpoll = other.m_netEpoll;
		return *this;
	}
	CEpollListenThread(const CEpollListenThread& other) :
			m_netThreadRes(other.m_netThreadRes), m_netEpoll(other.m_netEpoll) {

	}
	int32_t init(SNetThreadRes* sNetThreadRes, CNetEpoll* impl) {
		m_netEpoll = impl;
		m_netThreadRes = sNetThreadRes;
		return RETOK;
	}
	void main() {
		printf("m_imp->runListen\n");

		m_netThreadRes->m_iThreadID = pthread_self();
		printf("CEpollListenThread main m_pNetThreadRes->m_iThreadID:%lld\n ",
				m_netThreadRes->m_iThreadID);
		m_netEpoll->runListenBase(*(m_netThreadRes.get()));
	}
	STSharePtr<SNetThreadRes> m_netThreadRes;
	STSharePtr<CNetEpoll> m_netEpoll;
};

class CEpollRunThread: public STThread {
public:
	CEpollRunThread() :
		m_netThreadRes(NULL),m_netEpoll(NULL) {
	}
	CEpollRunThread(const CEpollRunThread & other) :
		m_netThreadRes(other.m_netThreadRes),m_netEpoll(other.m_netEpoll) {

	}
	CEpollRunThread& operator=(const CEpollRunThread& other) {
		m_netThreadRes = other.m_netThreadRes;
		m_netEpoll = other.m_netEpoll;
		return *this;
	}
	int32_t init(SNetThreadRes & sNetThreadRes,  CNetEpoll& impl) {
		m_netEpoll = &impl;
		m_netThreadRes = &sNetThreadRes;

		return RETOK;
	}

	void main() {
		m_netThreadRes->m_iThreadID = pthread_self();
		printf("CEpollRunThread main m_pNetThreadRes->m_iThreadID:%lld\n ",
				m_netThreadRes->m_iThreadID);

		m_netEpoll->runDealSockBase(*(m_netThreadRes.get()));
		printf("out CEpollRunThread main m_pNetThreadRes->m_iThreadID:%lld\n ",
				m_netThreadRes->m_iThreadID);
	}
private:
	STSharePtr<SNetThreadRes> m_netThreadRes;
	STSharePtr<CNetEpoll> m_netEpoll;

};

CNetEpollImp::CNetEpollImp() {

}
CNetEpollImp::~CNetEpollImp() {

}

int32_t CNetEpollImp::init(const STString& STStrConfigFile) {

	int32_t iRet = m_cEpollCfg.init(STStrConfigFile);
	if (iRet < RETOK) {
		printf("CNetEpollImp::init m_cEpollCfg.init\n");
		//log("");
		return iRet;
	}
	iRet = m_cEpollCfg.run();
	if (iRet < RETOK) {
		printf("CNetEpollImp::init m_cEpollCfg.run\n");
		//log("");
		return iRet;
	}

	SNetThreadRes sNetThreadRes;
	SNet sNet;
	SEvent sEvent;
	SNetData sNetData;
	int32_t iNumThread, iNum = 0;
	iNum = m_cEpollCfg.m_sNetDataCfg.m_vectorIP.size();

	iNumThread = m_cEpollCfg.m_sThreadCfg.m_iRunThread;
	printf("m_iRunThread:%d\n", iNumThread);

	for (uint32_t j = 1; j <= iNumThread; j++) {
		printf("m_iRunThread for (uint32_t j = 1; j <= iNumThread; j++) \n",
				iNumThread);
		m_runEvent.insert(
				std::make_pair<uint32_t, SNetThreadRes>(j, sNetThreadRes));
	}

	iNumThread = m_cEpollCfg.m_sThreadCfg.m_iListenThread;
	printf("iNumThread:%d\n", iNumThread);

	sNetData = m_cEpollCfg.m_sNetDataCfg.m_vectorIP[0];
	sNet.strIP = sNetData.szIP;
	sNet.iPort = sNetData.iPort;
	sNetThreadRes.sMapEvent.insert(std::pair<SNet, SEvent>(sNet, sEvent));
	for (uint32_t j = 1; j <= iNumThread; j++) {

		for (uint32_t i = 1; iNum > i * j; i++) {
			sNetData = m_cEpollCfg.m_sNetDataCfg.m_vectorIP[j * i];
			sNet.strIP = sNetData.szIP;
			sNet.iPort = sNetData.iPort;
			sNetThreadRes.sMapEvent.insert(
					std::pair<SNet, SEvent>(sNet, sEvent));
		}
		m_listenEvent[j] = sNetThreadRes;
		sNetThreadRes.sMapEvent.clear();
	}
//m_listenEvent
	iRet = initDealSock();
	if (iRet < RETOK) {
		//log("");
		return iRet;
	}
	printf("CNetEpollImp::init initListen begin\n");
	iRet = initListen();
	if (iRet < RETOK) {
		printf("CNetEpollImp::init initListen end\n");
		//log();
		return iRet;
	}
	return RETOK;
}

int32_t CNetEpollImp::run(CNetEpoll& CNetEpoll) {
	int32_t iRet = RETOK;
	printf(" CNetEpollImp::run  runDealSock\n");
	iRet = runDealSock(CNetEpoll);
	if (iRet < RETOK) {
		//log
		printf(" CNetEpollImp::run  runDealSock error\n");
		return RETERROR;
	}

	printf(" CNetEpollImp::run  runListen\n");
	iRet = runListen(CNetEpoll);
	if (iRet < RETOK) {
		//log
		printf(" CNetEpollImp::run  runListen error\n");
		return RETERROR;
	}
	printf(" out CNetEpollImp::run  runListen \n");
	return iRet;
}

int32_t CNetEpollImp::destroy() {
	m_cEpollCfg.destroy();
	closeAll();
	std::vector<CEpollListenThread>::iterator itBegin =
			m_vctListenThread.begin();
	std::vector<CEpollListenThread>::iterator itEnd = m_vctListenThread.end();
	while (itBegin != itEnd) {
		itBegin->askToStop();
		++itBegin;
	}
	std::vector<CEpollRunThread>::iterator itBeginRun = m_vctRunThread.begin();
	std::vector<CEpollRunThread>::iterator itEndRun = m_vctRunThread.end();
	while (itBeginRun != itEndRun) {
		itBeginRun->askToStop();
		++itBeginRun;
	}
	return RETOK;
}

int32_t CNetEpollImp::closeAll() {
	int32_t iRet = RETOK;
	iRet = closeListen();
	if (iRet < RETOK) {
		return iRet;
	}
	iRet = closeDealSock();
	if (iRet < RETOK) {
		return iRet;
	}
	return RETOK;
}

int32_t CNetEpollImp::initListen() {
	int32_t iRet = 0;
	std::map<uint32_t, SNetThreadRes>::iterator it = m_listenEvent.begin();
	printf("CNetEpollImp::initListen initBase___1\n");
	while (it != m_listenEvent.end()) {
		iRet = initBase(it->second);
		if (iRet < 0) {
			printf("CNetEpollImp::initListen initBase\n");
			//log
		}
		printf("CNetEpollImp::initListen initBase next\n");
		++it;
	}
	printf("CNetEpollImp::initListen initBase___2\n");
	return iRet;
}

int32_t CNetEpollImp::initDealSock() {
	int32_t iRet = RETOK;
	printf("CNetEpollImp::initDealSock while (it != m_runEvent.end())\n");
	std::map<uint32_t, SNetThreadRes>::iterator it = m_runEvent.begin();
	while (it != m_runEvent.end()) {
		printf(
				"in CNetEpollImp::initDealSock while (it != m_runEvent.end())\n");
		iRet = initBase(it->second);
		if (iRet < 0) {
			//log
		}
		++it;
	}
	return iRet;
}

int32_t CNetEpollImp::closeListen() {
	std::map<uint32_t, SNetThreadRes>::iterator it = m_listenEvent.begin();
	while (it != m_listenEvent.end()) {
		closeBase(it->second);
		++it;
	}
	return RETOK;
}

int32_t CNetEpollImp::closeDealSock() {
	std::map<uint32_t, SNetThreadRes>::iterator it = m_runEvent.begin();
	while (it != m_runEvent.end()) {
		closeBase(it->second);
		++it;
	}
	return RETOK;
}
int32_t CNetEpollImp::runListen(CNetEpoll& cNetEpoll) {
	printf("CNetEpollImp::runListen in\n");
	CEpollListenThread cEpollListenThread;
	int32_t iRet = RETOK;
	std::map<uint32_t, SNetThreadRes>::iterator it = m_listenEvent.begin();
	printf("m_listenEvent.size:%d\n", m_listenEvent.size());
	while (it != m_listenEvent.end()) {
		printf("CNetEpollImp::runListen while (it != m_listenEvent.end()) \n");
		m_vctListenThread.push_back(cEpollListenThread);
		m_vctListenThread.back().init(&(it->second), &cNetEpoll);
		iRet = m_vctListenThread.back().exec();
		if (iRet != 0) {
			//log
			printf("CNetEpollImp::runListen if\n");
		}
		++it;
	}
	sleep(60);
	printf("out CNetEpollImp::runListen while (it != m_listenEvent.end()) \n");
	return iRet;
}

int32_t CNetEpollImp::runDealSock(CNetEpoll& CNetEpoll) {
	printf("in CNetEpollImp::runDealSock555555\n");
	CEpollRunThread cEpollRunThread;
	int32_t iRet = RETOK;
	std::map<uint32_t, SNetThreadRes>::iterator it = m_runEvent.begin();
	while (it != m_runEvent.end()) {
		printf(
				"in CNetEpollImp::runDealSock while (it != m_runEvent.end())4444444\n");
		m_vctRunThread.push_back(cEpollRunThread);
		m_vctRunThread.back().init(it->second, CNetEpoll);
		iRet = m_vctRunThread.back().exec();
		if (iRet != 0) {
			//log
		}
		++it;
	}

	std::vector<CEpollRunThread>::iterator itRun = m_vctRunThread.begin();
	while (itRun != m_vctRunThread.end()) {
		if (itRun->isRunning()) {
			++itRun;
		} else {
			sleep(1);
		}

	}

	printf("in CNetEpollImp::runDealSock while (itbegin != itend)3333333\n");
	NumFlag numFlag;
	std::map<uint32_t, SNetThreadRes>::iterator itbegin = m_runEvent.begin();
	while (itbegin != m_runEvent.end()) {
		printf(
				"out CNetEpollImp::runDealSock while (itbegin != itend)22222:%lld\n",
				itbegin->second.m_iThreadID);

		numFlag.uEventNum = 0;
		numFlag.uThreadNum = itbegin->first;
		m_mapThread2Num.insert(
				std::make_pair<pthread_t, NumFlag>(itbegin->second.m_iThreadID,
						numFlag));
		++itbegin;
	}
	printf("out CNetEpollImp::runDealSock while (itbegin != itend)111111  \n");
	return iRet;
}
//do some event
int32_t CNetEpollImp::runDealSockBase(SNetThreadRes& sNetThreadRes) {
	int32_t iAccpEvNum = 0;
	int32_t it = 0;
	int32_t iReadNum = 0;
	int32_t iRet = RETERROR;
	int32_t iErrorCount = 0;
	STString strBuffer;
	SNet sNet;
	std::set<SNet> setSNetDisab;
	int32_t iSetSNet;
	std::pair<std::map<SNet, SEvent>::iterator, bool> itRet;
	std::map<SNet, SEvent>::iterator itbegin;
	std::map<SNet, SEvent>::iterator itend;
	std::pair<std::map<int32_t, SNet>::iterator, bool> itRetS2N;
	std::map<int32_t, SNet>::iterator itBeginS2N;
	std::map<int32_t, SNet>::iterator itEndS2N;
	std::set<SNet>::iterator itSetSNet;
	std::set<SNet>::iterator itSetSNetEnd;
	char szbuffer[BUFFSIZE];
	while (1) {
		printf("CNetEpollImp::runDealSockBase while (1)\n");
		if (sNetThreadRes.sMapEventBef.size() > 0) {
			if (sNetThreadRes.m_mutex->tryLock()) {
				setSNetDisab.clear();
				itBeginS2N = sNetThreadRes.sMapS2NetBef.begin();
				itEndS2N = sNetThreadRes.sMapS2NetBef.end();
				while (itBeginS2N != itEndS2N) {
					itRetS2N = sNetThreadRes.sMapS2Net.insert(
							std::make_pair<int32_t, SNet>(itBeginS2N->first,
									itBeginS2N->second));
					if (!itRetS2N.second) { //false
						close(itBeginS2N->first);
						setSNetDisab.insert(itBeginS2N->second);
					}
					++itBeginS2N;
				}
				iSetSNet = setSNetDisab.size();

				itbegin = sNetThreadRes.sMapEventBef.begin();
				itend = sNetThreadRes.sMapEventBef.end();

				while (itbegin != itend) {
					printf(
							"CNetEpollImp::runDealSockBase while (itbegin != itend)\n");
					if (iSetSNet > 0) {
						itSetSNet = setSNetDisab.find(itbegin->first);
						if (itSetSNet != setSNetDisab.end()) {
							++itbegin;
							continue;
						}
					}
					itRet = sNetThreadRes.sMapEvent.insert(
							std::pair<SNet, SEvent>(itbegin->first,
									itbegin->second));

					int32_t iRet = 0;
					if (itRet.second) {
						int sflag = 0;
						socklen_t optlen;
						int32_t iret = getsockopt(itRet.first->second.data.fd,
						SOL_SOCKET, SO_SNDBUF, &sflag, &optlen);
						setsockopt(itRet.first->second.data.fd, IPPROTO_TCP,
						TCP_NODELAY, &sflag, sizeof(sflag));
						itRet.first->second.events = EPOLLOUT | EPOLLIN
								| EPOLLET;
						iRet = epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_ADD,
								itRet.first->second.data.fd,
								&(itRet.first->second));
					} else {
						printf(
								"CNetEpollImp::runDealSockBase some error happen \n");
					}

					printf(
							"99999999CNetEpollImp::runDealSockBase could some error epoll_ctl:%d \n",
							iRet);

					++itbegin;
				}
				printf(
						"CNetEpollImp::runDealSockBase before sTMutex.unlock :%lld\n",
						sNetThreadRes.m_iThreadID);

				sNetThreadRes.m_mutex->unlock();
			}

			std::map<pthread_t, NumFlag>::iterator it = m_mapThread2Num.find(
					sNetThreadRes.m_iThreadID);
			{
				std::map<pthread_t, NumFlag>::iterator it =
						m_mapThread2Num.begin();
				while (it != m_mapThread2Num.end()) {
					printf(
							"CNetEpollImp::runDealSockBase while (it != m_mapThread2Size.end()) :%lld",
							it->first);
					++it;
				}

			}
			if (it == m_mapThread2Num.end()) {
				//log
				printf(
						"CNetEpollImp::runDealSockBase if (it == m_mapThread2Size.end())21212121212\n");
				return RETERROR;
			}

			it->second.uEventNum = sNetThreadRes.sMapEvent.size();
			sNetThreadRes.sMapEventBef.clear();
		}
		int32_t iRet = epoll_wait(sNetThreadRes.m_eFd,
				sNetThreadRes.m_happenEvent, MAXHAPPENEVENT,
				sNetThreadRes.m_iTimeOut);
		printf("CNetEpollImp::runDealSockBase epoll_wait :%d\n", iRet);
		if (iRet < 0) {
			printf(
					"wwwwwwwwwwwwww  CNetEpollImp::runDealSockBase epoll_wait\n");
			//log(RETWARING, "");
			++iErrorCount;
			if (iErrorCount >= MAXERRORHAPPEN) {
				exit(EXIT_FAILURE);
			}
			iRet = closeBase(sNetThreadRes);
			if (iRet < RETOK) {
				//log("");
				return iRet;
			}
			iRet = initBase(sNetThreadRes);
			if (iRet < RETOK) {
				//log("");
				return iRet;
			}
			continue;
		}
		iErrorCount = 0;
		for (it = 0; it < iRet; it++) {
			if (sNetThreadRes.m_happenEvent[it].events & EPOLLMSG) {
				printf("CNetEpollImp::runDealSockBase  & EPOLLMSG) \n");
			}
			if ((sNetThreadRes.m_happenEvent[it].events & EPOLLERR)
					|| (sNetThreadRes.m_happenEvent[it].events & EPOLLHUP)) { //exception happen
				//log (//loger, __FILE__, __LINE__, RETWARING, "[conn] close listen because epollerr or epollhup");
				printf("CNetEpollImp::runDealSockBase  & EPOLLERR) \n");
				sNetThreadRes.m_mutex->lock();

				std::map<int32_t, SNet>::iterator itS2N =
						sNetThreadRes.sMapS2Net.find(
								sNetThreadRes.m_happenEvent[it].data.fd);
				if (itS2N != sNetThreadRes.sMapS2Net.end()) {
					sNet = itS2N->second;
					sNetThreadRes.sMapS2Net.erase(itS2N);
				}
				printf("1111122222GetAddressBySocket:%d sNet port:%d\n", iRet,
						sNet.iPort);
				std::map<SNet, SEvent>::iterator itMapEvent =
						sNetThreadRes.sMapEvent.find(sNet);
				if (itMapEvent != sNetThreadRes.sMapEvent.end()) {
					epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_DEL,
							itMapEvent->second.data.fd, &(itMapEvent->second));
					close(itMapEvent->second.data.fd);
					sNetThreadRes.sMapEvent.erase(sNet);
				}
				sNetThreadRes.m_mutex->unlock();
				printf("CNetEpollImp::runDealSockBase EPOLLERR) \n");
				continue;
			}
			if (sNetThreadRes.m_happenEvent[it].events & EPOLLPRI) {
				printf(
						"9999999966666 CNetEpollImp::runDealSockBase } else {:%d \n",
						sNetThreadRes.m_happenEvent[it].events);

				socket_send(sNetThreadRes.m_happenEvent[it].data.fd,
						sNetThreadRes.m_szBuffer,
						strlen(sNetThreadRes.m_szBuffer));
				//log(RETERROR, __FILE__, __LINE__, "socket other errro happen  thread:Run to exit");
				//			iRet = RETERROR;
				//			return iRet;
			}
			/*			if (sNetThreadRes.m_happenEvent[it].data.fd < 0) {
			 printf(
			 "CNetEpollImp::runDealSockBase if (sNetThreadRes.m_happenEvent[it].data.fd < 0) epoll_wait :%d\n",
			 it);
			 continue;
			 }
			 */

			if (sNetThreadRes.m_happenEvent[it].data.fd
					== sNetThreadRes.m_cmdPipo[1]) {
				iRet = dealCommand(sNetThreadRes.m_happenEvent[it],
						sNetThreadRes);
				if (iRet < RETOK) {
					//log(RETERROR, __FILE__, __LINE__, "run,dealCommand,Return:", iRet);
					return iRet;
				}
				continue;
			}

			printf("CNetEpollImp::runDealSockBase EPOLLIN:%d \n",
					sNetThreadRes.m_happenEvent[it].events);
			if ((sNetThreadRes.m_happenEvent[it].events & EPOLLOUT)
					|| (sNetThreadRes.m_happenEvent[it].events & EPOLLWRNORM)
					|| (sNetThreadRes.m_happenEvent[it].events & EPOLLWRBAND)) {
				printf(
						"CNetEpollImp::runDealSockBase if (sNetThreadRes.m_happenEvent[it].events & EPOLLOUT EPOLLWRNORM EPOLLWRBAND) \n");
				static int ii = 0;
				++ii;

				sprintf(szbuffer,
						"hahah111111111111111111111111111111111aah:%d", ii);
				socket_send(sNetThreadRes.m_happenEvent[it].data.fd, szbuffer,
						sizeof(szbuffer));
			}
			if ((sNetThreadRes.m_happenEvent[it].events & EPOLLIN)
					|| (sNetThreadRes.m_happenEvent[it].events & EPOLLRDBAND)
					|| (sNetThreadRes.m_happenEvent[it].events & EPOLLRDNORM)) {
				printf("CNetEpollImp::runDealSockBase inininininininiin \n");
				iRet = socket_recv(sNetThreadRes.m_happenEvent[it].data.fd,
						szbuffer, sizeof(szbuffer));
				if (RETSOCKETCLOSE == iRet) {

					epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_DEL,
							sNetThreadRes.m_happenEvent[it].data.fd,
							&sNetThreadRes.m_happenEvent[it]);

					sNetThreadRes.m_mutex->lock();
					std::map<int32_t, SNet>::iterator itS2N =
							sNetThreadRes.sMapS2Net.find(
									sNetThreadRes.m_happenEvent[it].data.fd);
					if (itS2N != sNetThreadRes.sMapS2Net.end()) {
						std::map<SNet, SEvent>::iterator it =
								sNetThreadRes.sMapEvent.find(itS2N->second);
						if (it != sNetThreadRes.sMapEvent.end()) {
							sNetThreadRes.sMapEvent.erase(it);
						}
						sNetThreadRes.sMapS2Net.erase(itS2N);
					}
					close(sNetThreadRes.m_happenEvent[it].data.fd);
					sNetThreadRes.m_mutex->unlock();

				}
//---------------------------------------------------
			}
		}
	}
	printf("CNetEpollImp::runDealSockBase } else {: \n");
}
int32_t CNetEpollImp::socket_recv(int iSFd, char* szBuf, int32_t iBufLen) {
	int32_t iRNum = 0;
	do {
		printf("2222read fd buffer:memset\n");
		memset(szBuf, 0x00, iBufLen);
		iRNum = read(iSFd, szBuf, iBufLen - 1);
		printf("CNetEpollImp::runDealSockBase 222 read read:%d \n", iRNum);
		printf("2222read fd buffer read\n");
		if (iRNum > 0) {	//read data
			printf("2222read fd buffer:%s\n", szBuf);
			/*
			 sNetThreadRes.m_szBuffer[iReadNum] = '\0';
			 strBuffer += sNetThreadRes.m_szBuffer;
			 if (strBuffer.size() > MAXCMDSIZE) {
			 //log(buffer too much);
			 break;
			 }
			 */
		} else if (iRNum < 0) { //Failed to read
			printf("} else if (iReadNum < 0) {\n");
			if (errno == EAGAIN) { //No data.
				//log(FALTER, __FILE__, __LINE__, "[data] n < 0, no data, errno= %d", errno);
				break;
			} else {	//The client is closed
				printf("} else {\n");
				//log(FALTER, __FILE__, __LINE__, "[data] n < 0, peer close, errno=%d", errno);
				//set socket fd error flag
				return RETSOCKETCLOSE;
			}
		} else if (iRNum == 0) { //The client is closed
			printf("else if (iReadNum == 0)\n");
			//log(FALTER, __FILE__, __LINE__, "[data] n = 0, peer close, errno=%d", errno);
			return RETSOCKETCLOSE;
		}
	} while (1);
	return 0;
}
int32_t CNetEpollImp::socket_send(int sockfd, char* buffer, int32_t buflen) {
	ssize_t tmp;
	size_t total = buflen;
	const char *p = buffer;
	while (1) {
		printf("socket_send tmp:while (1):%d\n", total);
		printf("socket_send tmp:while (1):%s\n", p);
		printf("socket_send tmp:while (1) data:%d\n", p[0]);
		tmp = write(sockfd, p, total);
		printf("socket_send tmp:while (1):write\n");
		printf("socket_send tmp:while (1):%s\n", p);
		if (tmp < 0) {
			printf("socket_send tmp:%d errno:%d \n", tmp, errno);
			//		if (errno == EINTR)
			//			return -1;
			printf("55555 socket_send tmp:while (1)\n");
			if (errno == EAGAIN) {
				usleep(1000);
				continue;
			} else {
				printf("11111 socket_send tmp:%d errno:%d \n", tmp, errno);
				return RETSOCKETCLOSE;
			}
		}
		printf("2222 socket_send tmp:while (1)\n");
		if ((size_t) tmp == total)
			return buflen;
		printf("333333 socket_send tmp:while (1)\n");
		total -= tmp;
		p += tmp;
	}
	printf("44444 socket_send tmp:while (1)\n");
	return tmp;
}
void CNetEpollImp::setNonBlocking(int ISFd) {
	int opts;
	opts = fcntl(ISFd, F_GETFL);
	if (opts < 0) {
		//perror("fcntl(sock,GETFL)");
		//log(RETERROR, "fcntl(sock,GETFL)");
		return;
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(ISFd, F_SETFL, opts) < 0) {
		//perror("fcntl(sock,SETFL,opts)");
		//log("fcntl(sock,SETFL,opts)");
		return;
	}
}
int32_t CNetEpollImp::runListenBase(SNetThreadRes & sNetThreadRes) {
	int32_t it = RETERROR;
	int32_t iRet = RETERROR;
	struct sockaddr_in clientaddr;
	SNet sNet;
	unsigned int uiAddrSize = sizeof(struct sockaddr);
	iRet = initBase(sNetThreadRes);
	if (iRet < 0) {
		//log
		return iRet;
	}
	int32_t iErrorCount = 0;
	printf("CNetEpollImp::runListenBase(SNetThreadRes & sNetThreadRes)  \n");
	while (1) {
		printf(
				"in CNetEpollImp::runListenBase(SNetThreadRes & sNetThreadRes)  \n");
		//At the same time an event occurs at most,Unless more than one handle add
		iRet = epoll_wait(sNetThreadRes.m_eFd, sNetThreadRes.m_happenEvent,
		MAXHAPPENEVENT, -1);
		printf("in CNetEpollImp::runListenBase get epoll_wait iRet:%d\n ",
				iRet);
		if (iRet < 0) {
			++iErrorCount;
			//log(RETWARING, "");

			if (iErrorCount >= MAXERRORHAPPEN) {
				exit(EXIT_FAILURE);
			}
			iRet = initListen();
			//log();
			if (iRet < 0) {
				//log();
				return iRet;
			}
		}
		iErrorCount = 0;
		for (it = 0; it < iRet; it++) {
			printf(
					"in CNetEpollImp::runListenBase for (it = 0; it < iRet; it++) \n ");
			if (sNetThreadRes.m_happenEvent[it].data.fd < 0) {
				continue;
			}
			printf(
					"in CNetEpollImp::runListenBase if (sNetThreadRes.m_happenEvent[it].data.fd \n ");
			if (sNetThreadRes.m_happenEvent[it].data.fd
					== sNetThreadRes.m_cmdPipo[1]) {
				iRet = dealCommand(sNetThreadRes.m_happenEvent[it],
						sNetThreadRes);
				if (iRet < RETOK) {
					//log(RETERROR, __FILE__, __LINE__, "run,dealCommand,Return:", iRet);
					return iRet;
				}
				continue;
			}
			printf(
					"CNetEpollImp::runListenBase if (sNetThreadRes.m_happenEvent[it].events & EPOLLIN) \n ");

			if (sNetThreadRes.m_happenEvent[it].events & EPOLLIN) //have connection request
//			if (!((sNetThreadRes.m_happenEvent[it].events & EPOLLERR)||
//					(sNetThreadRes.m_happenEvent[it].events & EPOLLHUP)))
			{
				printf(
						"in CNetEpollImp::runListenBase if (sNetThreadRes.m_happenEvent[it].events & EPOLLIN) \n ");
				do {
					int iSFd;
					SEvent sEvent;
					iSFd = accept(sNetThreadRes.m_happenEvent[it].data.fd,
							(struct sockaddr *) &clientaddr, &uiAddrSize);
//					printf("accept CNetEpollImp::runListenBase if (sNetThreadRes.m_happenEvent[it].events & EPOLLIN) \n ");
					if (iSFd > 0) {
						printf(
								"in CNetEpollImp::runListenBase [conn] peer=%s:%d  \n ",
								inet_ntoa(clientaddr.sin_addr),
								ntohs(clientaddr.sin_port));
						//log(FALTER,__FILE__,__LINE__, "[conn] peer=%s:%d",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
						sNet.strIP = inet_ntoa(clientaddr.sin_addr);
						sNet.iPort = clientaddr.sin_port;
						setNonBlocking(iSFd);
						sEvent.data.fd = iSFd;
						pushSock(sEvent, sNet);
						//deal socket will do zhe socket
					} else {
						if ((errno == EAGAIN)) //there is no connection request
						{
							printf(
									"CNetEpollImp::runListenBase continue2222\n");
//							sleep(1);
							break;
							//continue;
						} else //In other cases the handle RETERROR, should be closed and listen again
						{
							//log(//log_ERROR,
							//		"CNetEpollImp close listen because accept fail and errno not equal eagain or eintr.");
							printf("CNetEpollImp::runListenBase else11111\n");
							close(sNetThreadRes.m_happenEvent[it].data.fd);
							sNetThreadRes.m_happenEvent[it].data.fd = NULLSTS;
							iRet = epoll_ctl(sNetThreadRes.m_eFd,
							EPOLL_CTL_DEL,
									sNetThreadRes.m_happenEvent[it].data.fd,
									&sNetThreadRes.m_happenEvent[it]);
							if (iRet != RETOK) {
								//log(RETERROR,"");
							}

							iRet = initBase(sNetThreadRes);
							if (iRet != RETOK) {
								//log(RETERROR,"");
								exit(RETERROR);
							}
							break;
						}
					}
				} while (1);
			} else if (!((sNetThreadRes.m_happenEvent[it].events & EPOLLERR)
					|| (sNetThreadRes.m_happenEvent[it].events & EPOLLHUP))) {
				printf(
						"else if (!((sNetThreadRes.m_happenEvent[it].events & EPOLLERR)||\n");
			} else {
				printf("CNetEpollImp::runListenBase epoll_wait else \n");
			}
		}
	}
	return RETERROR;
}

int32_t CNetEpollImp::pushSock(SEvent& sEvent, SNet& sNet) {
	uint32_t iMin = 999999;
	int32_t iRet = 0;
	//thread to size
	std::map<pthread_t, NumFlag>::iterator itTmp, it = m_mapThread2Num.begin();
	printf("in CNetEpollImp::pushSock \n");
	itTmp = it;
	while (it != m_mapThread2Num.end()) {
//		log("");
		if (iMin > it->second.uEventNum) {
			iMin = it->second.uEventNum;
			itTmp = it;
			printf("in CNetEpollImp::pushSock m_mapThread2Num: %lld \n",
					itTmp->second.uThreadNum);
		}
		++it;
	}

	std::map<uint32_t, SNetThreadRes>::iterator itEven = m_runEvent.find(
			itTmp->second.uThreadNum);
	printf("in CNetEpollImp::pushSock if (itEven == m_runEvent.end()) \n");
	if (itEven == m_runEvent.end()) {
//		log("");
		printf(
				"in CNetEpollImp::pushSock if (itEven == m_runEvent.end()) return RETERROR \n");
		return RETERROR;
	}
	iRet = GetAddressBySocket(sEvent.data.fd, sNet);
	if (iRet != RETOK) {
		printf("CNetEpollImp::pushSock GetAddressBySocket\n");
		return RETWARING;
	}
	std::map<int32_t, SNet>::iterator itS2N = itEven->second.sMapS2NetBef.find(
			sEvent.data.fd);
	printf("1 must have have lock\n");
	itEven->second.m_mutexBef->lock();
	if (itS2N != itEven->second.sMapS2NetBef.end()) {
		SNet tmpNet;
		tmpNet = itS2N->second;
		itEven->second.sMapS2NetBef.erase(sEvent.data.fd);
		itEven->second.sMapEventBef.erase(tmpNet);
	}
	printf("3 must have have lock\n");
	itEven->second.sMapS2NetBef.insert(
			std::pair<int32_t, SNet>(sEvent.data.fd, sNet));
	printf("4 must have have lock\n");
	itEven->second.sMapEventBef.insert(std::pair<SNet, SEvent>(sNet, sEvent));
	printf("5 must have have lock\n");
	itEven->second.m_mutexBef->unlock();
	printf("6 must have have lock\n");
	return RETOK;
}

int32_t CNetEpollImp::initBase(SNetThreadRes & sNetThreadRes) {
	printf("CNetEpollImp::initBase\n");
	sNetThreadRes.m_eFd = epoll_create(MAXHAPPENEVENT);
	if (0 > sNetThreadRes.m_eFd) {
		printf("CNetEpollImp::initBase epoll_create\n");
		return RETERROR;
	}
	printf("CNetEpollImp::initBase pipe \n");
	int32_t iRet = 0;
	sNetThreadRes.m_pipEv.data.fd = sNetThreadRes.m_cmdPipo[0];
	sNetThreadRes.m_pipEv.events = EPOLLIN | EPOLLET | EPOLLOUT;

	printf("CNetEpollImp::initBase pipe \n");
	iRet = pipe(sNetThreadRes.m_cmdPipo);
	if (iRet != 0) {
		printf("CNetEpollImp::initBase pipe\n");
		return RETERROR;
	}
	sNetThreadRes.m_pipEv.data.fd = sNetThreadRes.m_cmdPipo[0];
	sNetThreadRes.m_pipEv.events = EPOLLIN | EPOLLET | EPOLLOUT;

	printf("CNetEpollImp::initBase epoll_ctl add1 \n");
	iRet = epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_ADD,
			sNetThreadRes.m_cmdPipo[1], &sNetThreadRes.m_pipEv);
	if (0 > iRet) {
		printf("CNetEpollImp::initBase epoll_ctl\n");
//		log("");
		return RETERROR;
	}
	std::map<SNet, SEvent>::iterator itBegin = sNetThreadRes.sMapEvent.begin();
//	std::map<SNet, SEvent>::iterator itEnd = sNetThreadRes.sMapEvent.end();
	struct sockaddr_in serveraddr;
	printf("CNetEpollImp::initBase while (itBegin != itEnd)  \n");
	while (itBegin != sNetThreadRes.sMapEvent.end()) {

		close(itBegin->second.data.fd);
		itBegin->second.data.fd = socket(AF_INET, SOCK_STREAM, 0);
		printf("CNetEpollImp::initBase port :%d\n", itBegin->first.iPort);
		printf("CNetEpollImp::initBase ip:%s\n", itBegin->first.strIP.c_str());
		printf("CNetEpollImp::initBase :%d\n", itBegin->second.data.fd);
		if (itBegin->second.data.fd < 0) {
			printf("CNetEpollImp::initBase socket error\n");
			//log(RETERROR, "");
			return iRet;
		}
		setNonBlocking(itBegin->second.data.fd);
		itBegin->second.events = EPOLLIN | EPOLLET;

		int32_t iRet = epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_ADD,
				itBegin->second.data.fd, &(itBegin->second));

		if (0 > iRet) {
			printf("CNetEpollImp::initBase epoll_ctl add error\n");
			//log
			return RETERROR;
		}
		bzero(&serveraddr, sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = inet_addr(itBegin->first.strIP.c_str());
//		struct sockaddr_in addr;
//		addr.sin_addr.s_addr = inet_addr(host);
//		inet_aton(itBegin->first.strIP.c_str(), &(serveraddr.sin_addr));
		serveraddr.sin_port = htons(itBegin->first.iPort);
		bind(itBegin->second.data.fd, (sockaddr *) &serveraddr,
				sizeof(serveraddr));
		listen(itBegin->second.data.fd, LISTENQ);

		++itBegin;
	}
	return RETOK;
}

int32_t CNetEpollImp::closeBase(SNetThreadRes & sNetThreadRes) {

	std::map<SNet, SEvent>::iterator itevbegin =
			sNetThreadRes.sMapEvent.begin();
	std::map<SNet, SEvent>::iterator itevend = sNetThreadRes.sMapEvent.begin();

	while (itevbegin != itevend) {
		close(itevbegin->second.data.fd);
		itevbegin->second.data.fd = NULLSTS;
		++itevbegin;
	}

	itevbegin = sNetThreadRes.sMapEventBef.begin();
	itevend = sNetThreadRes.sMapEventBef.begin();
	while (itevbegin != itevend) {
		close(itevbegin->second.data.fd);
		itevbegin->second.data.fd = NULLSTS;
		++itevbegin;
	}
	return RETOK;
}

int32_t CNetEpollImp::dealCommand(SEvent & cmdEvent,
		SNetThreadRes & sNetThreadRes) {
	int32_t iRet = 0;
	char szCmd[1024] = "";
	if (cmdEvent.events & EPOLLIN) {
		int32_t iNum = read(cmdEvent.data.fd, &szCmd, sizeof(szCmd));
		if (iNum > 0) {
			if (NULL != strstr(szCmd, "EXIT")) {
				//log("get command thread:Run to exit");
				closeBase(sNetThreadRes);
				close(sNetThreadRes.m_cmdPipo[0]);
				close(sNetThreadRes.m_cmdPipo[1]);
				iRet = EXIT;
				return iRet;

			} else if (NULL != strstr(szCmd, "CLOSEFD")) {
				iRet = closeBase(sNetThreadRes);
				//log("get command thread:Run closed all socked handle");
			} else if (NULL != strstr(szCmd, "INIT")) {
				iRet = closeBase(sNetThreadRes);
				if (iRet < RETOK) {
					return iRet;
				}
				close(sNetThreadRes.m_cmdPipo[0]);
				close(sNetThreadRes.m_cmdPipo[1]);
				iRet = initBase(sNetThreadRes);
				if (iRet < RETOK) {
					return iRet;
				}
				//log("other RETERROR happen thread:Run ");
			}
			return iRet;
		}
	} else if ((cmdEvent.events & EPOLLOUT)) {
		printf("other do\n");
	} else if ((cmdEvent.events & EPOLLERR) || (cmdEvent.events & EPOLLHUP)) {
		close(cmdEvent.data.fd);
		cmdEvent.data.fd = NULLSTS;
		//log(pipe read RETERROR happen thread:Run to exit");
		iRet = RETERROR;
		return iRet;
	} else {
		close(cmdEvent.data.fd);
		cmdEvent.data.fd = NULLSTS;
		//log("pipe other errro happen  thread:Run to exit");
		iRet = RETERROR;
		return iRet;
	}
	return RETOK;
}
int32_t CNetEpollImp::GetAddressBySocket(const int32_t m_socket, SNet& sNet) {
	struct sockaddr_in address;
	memset(&address, 0, sizeof(address));
	unsigned int uiAddrLen = sizeof(address);
	if (::getpeername(m_socket, (sockaddr*) &address, &uiAddrLen) != 0) {
		//log();
		return RETWARING;
	}
	sNet.strIP = ::inet_ntoa(address.sin_addr);
	sNet.iPort = ntohs(address.sin_port);
	return RETOK;
}

CNetEpoll::CNetEpoll() :
		m_cNetEpollImp(new CNetEpollImp) {
}
int32_t CNetEpoll::init(const STString& strFileName) {
	m_cSigOpt.ignalSig();
	return m_cNetEpollImp->init(strFileName);
}
int32_t CNetEpoll::run() {
	return m_cNetEpollImp->run(*this);
}
int32_t CNetEpoll::runDealSockBase(SNetThreadRes & sNetThreadRes) {
	return m_cNetEpollImp->runDealSockBase(sNetThreadRes);
}
int32_t CNetEpoll::runListenBase(SNetThreadRes & sNetThreadRes) {
	return m_cNetEpollImp->runListenBase(sNetThreadRes);
}
int32_t CNetEpoll::destroy() {

	return m_cNetEpollImp->destroy();
}
CNetEpoll::~CNetEpoll() {
}
