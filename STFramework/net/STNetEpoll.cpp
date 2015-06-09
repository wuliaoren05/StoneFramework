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

//for command function
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
	CEpollListenThread& operator=(
			const CEpollListenThread& cEpollListenThread) {
		m_netThreadRes = cEpollListenThread.m_netThreadRes;
		m_netEpoll = cEpollListenThread.m_netEpoll;
		return *this;
	}
	CEpollListenThread(const CEpollListenThread& cEpollListenThread) :
			m_netThreadRes(cEpollListenThread.m_netThreadRes), m_netEpoll(
					cEpollListenThread.m_netEpoll) {
	}
	int32_t init(SNetThreadRes* sNetThreadRes, CNetEpoll* impl) {
		m_netEpoll = impl;
		m_netThreadRes = sNetThreadRes;
		return RETOK;
	}
	void main() {
		m_netThreadRes->m_iThreadID = pthread_self();
		pthread_detach(m_netThreadRes->m_iThreadID);
		m_netEpoll->runListenBase(*m_netThreadRes);
	}
	SNetThreadRes* m_netThreadRes;
	CNetEpoll* m_netEpoll;
};

class CEpollRunThread: public STThread {
public:
	CEpollRunThread() :
			m_netThreadRes(NULL), m_netEpoll(NULL) {
	}
	CEpollRunThread(const CEpollRunThread & cEpollRunThread) :
			m_netThreadRes(cEpollRunThread.m_netThreadRes), m_netEpoll(
					cEpollRunThread.m_netEpoll) {
	}
	CEpollRunThread& operator=(const CEpollRunThread& cEpollRunThread) {
		m_netThreadRes = cEpollRunThread.m_netThreadRes;
		m_netEpoll = cEpollRunThread.m_netEpoll;
		return *this;
	}
	int32_t init(SNetThreadRes & sNetThreadRes, CNetEpoll& impl) {
		m_netEpoll = &impl;
		m_netThreadRes = &sNetThreadRes;
		return RETOK;
	}

	void main() {
		m_netThreadRes->m_iThreadID = pthread_self();
		pthread_detach(m_netThreadRes->m_iThreadID);
		m_netEpoll->runDealSockBase(*m_netThreadRes);
	}
private:
	SNetThreadRes* m_netThreadRes;
	CNetEpoll* m_netEpoll;
};

CNetEpollImp::CNetEpollImp() {
	m_exitFlag = RUNFLAG;
}
CNetEpollImp::~CNetEpollImp() {
}

int32_t CNetEpollImp::init(const STString& STStrConfigFile) {
	m_exitFlag = RUNFLAG;
	int32_t iRet = m_cEpollCfg.init(STStrConfigFile);
	if (iRet < RETOK) {
		//log  m_cEpollCfg.init
		return iRet;
	}
	iRet = m_cEpollCfg.run();
	if (iRet < RETOK) {
		//log  m_cEpollCfg.run
		return iRet;
	}
	SNetThreadRes sNetThreadRes;
	SNet sNet;
	SEvent sEvent;
	SNetData sNetData;
	uint32_t iNumThread, iNum = 0;
	iNumThread = m_cEpollCfg.m_sThreadCfg.m_iRunThread;
	//log iNumThread
	for (uint32_t j = 1; j <= iNumThread; j++) {
		//log iNumThread
		m_runEvent.insert(
				std::make_pair<uint32_t, SNetThreadRes>(j, sNetThreadRes));
	}
	iNumThread = m_cEpollCfg.m_sThreadCfg.m_iListenThread;
	//log  iNumThread
	iNum = m_cEpollCfg.m_sNetDataCfg.m_vectorIP.size();
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
	iRet = initDealSock();
	if (iRet < RETOK) {
		//log("");
		return iRet;
	}
	//log
	iRet = initListen();
	if (iRet < RETOK) {
		//log
		return iRet;
	}
	return RETOK;
}

int32_t CNetEpollImp::run(CNetEpoll& CNetEpoll) {
	int32_t iRet = RETOK;
	iRet = runDealSock(CNetEpoll);
	if (iRet < RETOK) {
		//log
		return RETERROR;
	}
	iRet = runListen(CNetEpoll);
	if (iRet < RETOK) {
		//log
		return RETERROR;
	}
	return iRet;
}

int32_t CNetEpollImp::destroy() {
	m_cEpollCfg.destroy();
	exitThread();
	std::vector<CEpollListenThread>::iterator itBegin =
			m_vctListenThread.begin();
	std::vector<CEpollListenThread>::iterator itEnd = m_vctListenThread.end();
	while (itBegin != itEnd) {
		itBegin->askToStop();
		while (itBegin->isRunning()) {
			sleep(1);
			continue;
		}
		++itBegin;
	}
	std::vector<CEpollRunThread>::iterator itBeginRun = m_vctRunThread.begin();
	std::vector<CEpollRunThread>::iterator itEndRun = m_vctRunThread.end();
	while (itBeginRun != itEndRun) {
		itBeginRun->askToStop();
		while (itBeginRun->isRunning()) {
			sleep(1);
			continue;
		}
		++itBeginRun;
	}
	closeAll();
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
	while (it != m_listenEvent.end()) {
		iRet = initBase(it->second);
		if (iRet < 0) {
			//log
		}
		//log
		++it;
	}
	return iRet;
}

int32_t CNetEpollImp::initDealSock() {
	int32_t iRet = RETOK;
	std::map<uint32_t, SNetThreadRes>::iterator it = m_runEvent.begin();
	while (it != m_runEvent.end()) {
		//log
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
	CEpollListenThread cEpollListenThread;
	int32_t iRet = RETOK;
	std::map<uint32_t, SNetThreadRes>::iterator it = m_listenEvent.begin();
	while (it != m_listenEvent.end()) {
		cEpollListenThread.init(&(it->second), &cNetEpoll);
		m_vctListenThread.push_back(cEpollListenThread);
		++it;
	}
	std::vector<CEpollListenThread>::iterator itListen =
			m_vctListenThread.begin();
	while (itListen != m_vctListenThread.end()) {
		iRet = itListen->exec();
		if (iRet != 0) {
			//log("CNetEpollImp::runListen if\n");
		}
		++itListen;
	}
	return iRet;
}

int32_t CNetEpollImp::runDealSock(CNetEpoll& CNetEpoll) {
	CEpollRunThread cEpollRunThread;
	int32_t iRet = RETOK;
	std::map<uint32_t, SNetThreadRes>::iterator itRunEvent = m_runEvent.begin();
	while (itRunEvent != m_runEvent.end()) {
		//log
		cEpollRunThread.init(itRunEvent->second, CNetEpoll);
		m_vctRunThread.push_back(cEpollRunThread);
		++itRunEvent;
	}
	std::vector<CEpollRunThread>::iterator itRun = m_vctRunThread.begin();
	while (itRun != m_vctRunThread.end()) {
		//log
		iRet = itRun->exec();
		if (iRet != 0) {
			//log
		}
		++itRun;
	}

	itRun = m_vctRunThread.begin();
	while (itRun != m_vctRunThread.end()) {
		if (itRun->isRunning()) {
			++itRun;
		} else {
			sleep(1);
		}
	}

	SNumFlag sNumFlag;
	std::map<uint32_t, SNetThreadRes>::iterator itBegin = m_runEvent.begin();
	while (itBegin != m_runEvent.end()) {
		//log ThreadID
		sNumFlag.uEventNum = 0;
		sNumFlag.uThreadNum = itBegin->first;
		m_mapThread2Num.insert(
				std::make_pair<pthread_t, SNumFlag>(itBegin->second.m_iThreadID,
						sNumFlag));
		++itBegin;
	}
	return iRet;
}
//do some event
int32_t CNetEpollImp::runDealSockBase(SNetThreadRes& sNetThreadRes) {
	int32_t it = 0;
	int32_t iRet = RETERROR;
	int32_t iErrorCount = 0;
	STString strBuffer;
	SNet sNet;
	std::set<SNet> setSNetDisab;
	std::pair<std::map<SNet, SEvent>::iterator, bool> itRet;
	std::map<SNet, SEvent>::iterator itBegin;
	std::map<SNet, SEvent>::const_iterator itend;
	std::map<int32_t, SNet>::iterator itBeginS2N;
	std::map<int32_t, SNet>::const_iterator itEndS2N;
	while (1) {
		//log runDealSockBase;
		if (m_exitFlag != RUNFLAG) {
			break;
		}
		if (sNetThreadRes.sMapEventBef.size() > 0) {
			if (sNetThreadRes.m_mutexBef->tryLock()) {
				itBeginS2N = sNetThreadRes.sMapS2NetBef.begin();
				itEndS2N = sNetThreadRes.sMapS2NetBef.end();
				while (itBeginS2N != itEndS2N) {
					sNetThreadRes.sMapS2Net.erase(itBeginS2N->first);
					sNetThreadRes.sMapS2Net.insert(
							std::make_pair<int32_t, SNet>(itBeginS2N->first,
									itBeginS2N->second));
					++itBeginS2N;
				}
				itBegin = sNetThreadRes.sMapEventBef.begin();
				itend = sNetThreadRes.sMapEventBef.end();
				while (itBegin != itend) {
					sNetThreadRes.sMapEvent.erase(itBegin->first);
					itRet = sNetThreadRes.sMapEvent.insert(
							std::pair<SNet, SEvent>(itBegin->first,
									itBegin->second));
					int32_t iRet = 0;
					if (itRet.second) {
						int sflag = 0;
//						sNetThreadRes.sMapEventBef.erase(itBegin);

						setsockopt(itRet.first->second.data.fd, SOL_SOCKET,
								SO_KEEPALIVE, &sflag, sizeof(sflag));
						setsockopt(itRet.first->second.data.fd,
						IPPROTO_TCP,
						TCP_NODELAY, &sflag, sizeof(sflag));
						itRet.first->second.events = EPOLLOUT | EPOLLIN
								| EPOLLET;
						iRet = epoll_ctl(sNetThreadRes.m_eFd,
						EPOLL_CTL_ADD, itRet.first->second.data.fd,
								&(itRet.first->second));

					} else {
						//log runDealSockBase some error happen;
					}
					//log   iRet;
					++itBegin;
//					itBegin = sNetThreadRes.sMapEventBef.begin();
				}
				sNetThreadRes.sMapS2NetBef.clear();
				sNetThreadRes.sMapEventBef.clear();
				//log

				//set event num to thread struct
				std::map<pthread_t, SNumFlag>::iterator it =
						m_mapThread2Num.find(sNetThreadRes.m_iThreadID);
				{
					std::map<pthread_t, SNumFlag>::iterator it =
							m_mapThread2Num.begin();
					while (it != m_mapThread2Num.end()) {
						//log
						++it;
					}
				}
				if (it == m_mapThread2Num.end()) {
					//log
					sNetThreadRes.m_mutexBef->unlock();
					return RETERROR;
				}
				it->second.uEventNum = sNetThreadRes.sMapEvent.size();
				sNetThreadRes.m_mutexBef->unlock();
			}
		}

		iRet = epoll_wait(sNetThreadRes.m_eFd, sNetThreadRes.m_happenEvent,
		MAXHAPPENEVENT, sNetThreadRes.m_iTimeOut);
		//log
		if (iRet < 0) {
			//log
			++iErrorCount;
			if (iErrorCount >= MAXERRORHAPPEN) {
				exit(EXIT_FAILURE);
			}
			iRet = closeBase(sNetThreadRes);
			if (iRet < RETOK) {
				//log
				return iRet;
			}
			iRet = initBase(sNetThreadRes);
			if (iRet < RETOK) {
				//log
				return iRet;
			}
			continue;
		}
		iErrorCount = 0;
		for (it = 0; it < iRet; it++) {
			if (sNetThreadRes.m_happenEvent[it].data.fd < 0) {
				continue;
			}
			if ((sNetThreadRes.m_happenEvent[it].data.fd < 0)
					|| (sNetThreadRes.m_happenEvent[it].events & EPOLLERR)
					|| (sNetThreadRes.m_happenEvent[it].events & EPOLLHUP)) { //exception happen
				//log (loger, __FILE__, __LINE__, RETWARING, "[conn] close listen because epollerr or epollhup");

				epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_DEL,
						sNetThreadRes.m_happenEvent[it].data.fd,
						&(sNetThreadRes.m_happenEvent[it]));

				sNetThreadRes.m_mutex->lock();
				std::map<int32_t, SNet>::iterator itS2N =
						sNetThreadRes.sMapS2Net.find(
								sNetThreadRes.m_happenEvent[it].data.fd);
				if (itS2N != sNetThreadRes.sMapS2Net.end()) {
					sNetThreadRes.sMapEvent.erase(itS2N->second);
					sNetThreadRes.sMapS2Net.erase(itS2N);
				}
				close(sNetThreadRes.m_happenEvent[it].data.fd);
				sNetThreadRes.m_mutex->unlock();
				//log runDealSockBase EPOLLERR
				continue;
			}

			//log  sNetThreadRes.m_happenEvent[it].events
			if ((sNetThreadRes.m_happenEvent[it].events & EPOLLOUT)
					|| (sNetThreadRes.m_happenEvent[it].events & EPOLLWRNORM)
					|| (sNetThreadRes.m_happenEvent[it].events & EPOLLWRBAND)) {
				//log runDealSockBase
				{
					static int32_t iNum = 0;
					char szBuffer[BUFFSIZE] = "this is demon service ";
					sprintf(szBuffer, "this is demon beffer:%d \n", ++iNum);
					socketSend(sNetThreadRes.m_happenEvent[it].data.fd,
							szBuffer,
							BUFFSIZE);
				}
			}

			if ((sNetThreadRes.m_happenEvent[it].events & EPOLLIN)
					|| (sNetThreadRes.m_happenEvent[it].events & EPOLLRDBAND)
					|| (sNetThreadRes.m_happenEvent[it].events & EPOLLRDNORM)) {
				{
					char szBuffer[BUFFSIZE];
					memset(szBuffer, 0x00, sizeof(szBuffer));
					iRet = socketRecv(sNetThreadRes.m_happenEvent[it].data.fd,
							szBuffer,
							BUFFSIZE);
					printf("this is recv buffer%d:%s\n", iRet, szBuffer);
				}
				if (0 > iRet) {
					epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_DEL,
							sNetThreadRes.m_happenEvent[it].data.fd,
							&sNetThreadRes.m_happenEvent[it]);
					sNetThreadRes.m_mutex->lock();
					std::map<int32_t, SNet>::iterator itS2N =
							sNetThreadRes.sMapS2Net.find(
									sNetThreadRes.m_happenEvent[it].data.fd);
					if (itS2N != sNetThreadRes.sMapS2Net.end()) {
						sNetThreadRes.sMapEvent.erase(itS2N->second);
						sNetThreadRes.sMapS2Net.erase(itS2N);
					}
					close(sNetThreadRes.m_happenEvent[it].data.fd);
					sNetThreadRes.m_mutex->unlock();
				}
			}
		}
	}
	return iRet;
}
int32_t CNetEpollImp::socketSend(int32_t iSFd, char* szBuf, int32_t nLen) {
	int nLeft = nLen;
	int nWrite = 0;
	while (nLeft > 0) {
		nWrite = write(iSFd, szBuf, nLeft);
		if (nWrite < 0) {
			if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
				continue;
			//log
			return nWrite;                  // error, return <= 0
		} else if (nWrite == 0) {
			return -1;
		} else {
			nLeft -= nWrite;
			szBuf += nWrite;
		}
	}
	return (nLen - nLeft);          // return >= 0
}
int32_t CNetEpollImp::socketRecv(int sockfd, char* szBuff, int32_t nLen) {
	int nLeft = nLen;
	int nRead = 0;
	while (nLeft > 0) {
		nRead = read(sockfd, szBuff, nLeft);
		if (nRead < 0) {
			if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
				continue;
			//log
			return nRead;                  // error, return < 0
		} else if (nRead == 0) {
			return -1;;                                  // EOF
		} else {
			nLeft -= nRead;
			szBuff += nRead;
		}
	}
	return (nLen - nLeft);               // return >= 0
}
void CNetEpollImp::setNonBlocking(int32_t ISFd) {
	int opts;
	opts = fcntl(ISFd, F_GETFL);
	if (opts < 0) {
		//perror("fcntl(sock,GETFL)");
		//log
		return;
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(ISFd, F_SETFL, opts) < 0) {
		//perror("fcntl(sock,SETFL,opts)");
		//log
		return;
	}
}
int32_t CNetEpollImp::runListenBase(SNetThreadRes & sNetThreadRes) {
	int32_t it = RETERROR;
	int32_t iRet = RETERROR;
	struct sockaddr_in clientaddr;
	unsigned int uiAddrSize = sizeof(struct sockaddr);
	iRet = initBase(sNetThreadRes);
	if (iRet < 0) {
		//log
		return iRet;
	}
	int32_t iErrorCount = 0;
	//log
	while (1) {
		if (m_exitFlag != RUNFLAG) {
			break;
		}
		//At the same time an event occurs at most,Unless more than one handle add
		iRet = epoll_wait(sNetThreadRes.m_eFd, sNetThreadRes.m_happenEvent,
		MAXHAPPENEVENT, -1);
		//log iRet
		if (iRet < 0) {
			++iErrorCount;
			//log
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

			//log
			if (sNetThreadRes.m_happenEvent[it].data.fd < 0) {
				continue;
			}
			//log

			if (sNetThreadRes.m_happenEvent[it].events & EPOLLIN) //have connection request
			{
				//log
				do {
					int iSFd = 0;
					SEvent sEvent;
					iSFd = accept(sNetThreadRes.m_happenEvent[it].data.fd,
							(struct sockaddr *) &clientaddr, &uiAddrSize);
					if (iSFd > 0) {
						//log(FALTER,__FILE__,__LINE__, "[conn] peer=%s:%d",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
						SNet sNet;
						sNet.strIP = inet_ntoa(clientaddr.sin_addr);
						sNet.iPort = clientaddr.sin_port;
						setNonBlocking(iSFd);
						sEvent.data.fd = iSFd;
						pushSock(sEvent, sNet);
						//deal socket will do the socket
					} else {
						if (errno == EAGAIN || errno == EWOULDBLOCK) //there is no connection request
						{
							//log
							break;
							//continue;
						} else //In other cases the handle RETERROR, should be closed and listen again
						{
							//log_ERROR,
							close(sNetThreadRes.m_happenEvent[it].data.fd);
							sNetThreadRes.m_happenEvent[it].data.fd = NULLSTS;
							iRet = epoll_ctl(sNetThreadRes.m_eFd,
							EPOLL_CTL_DEL,
									sNetThreadRes.m_happenEvent[it].data.fd,
									&sNetThreadRes.m_happenEvent[it]);
							if (iRet != RETOK) {
								//log RETERROR
							}

							iRet = initBase(sNetThreadRes);
							if (iRet != RETOK) {
								//log RETERROR
								exit(RETERROR);
							}
							break;
						}
					}
				} while (1);
			}
			if (!((sNetThreadRes.m_happenEvent[it].events & EPOLLERR)
					|| (sNetThreadRes.m_happenEvent[it].events & EPOLLHUP))) {
				//log
			} else {
				//log
			}
		}
	}
	return RETERROR;
}

int32_t CNetEpollImp::pushSock(SEvent& sEvent, SNet& sNet) {
	uint32_t iMin = 999999;
	int32_t iRet = 0;
//thread to size
	std::map<pthread_t, SNumFlag>::iterator itTmp, it = m_mapThread2Num.begin();
	itTmp = it;
	while (it != m_mapThread2Num.end()) {
		if (iMin > it->second.uEventNum) {
			iMin = it->second.uEventNum;
			itTmp = it;
			//log itTmp->second.uThreadNum
		}
		++it;
	}
	std::map<uint32_t, SNetThreadRes>::iterator itEven = m_runEvent.find(
			itTmp->second.uThreadNum);
	if (itEven == m_runEvent.end()) {
		//log m_runEvent find RETERROR
		return RETERROR;
	}
	std::map<int32_t, SNet>::iterator itS2N = itEven->second.sMapS2NetBef.find(
			sEvent.data.fd);
	itEven->second.m_mutexBef->lock();
	if (itS2N != itEven->second.sMapS2NetBef.end()) {
		itEven->second.sMapEventBef.erase(itS2N->second);
		itEven->second.sMapS2NetBef.erase(sEvent.data.fd);
	}
	itEven->second.sMapS2NetBef.insert(
			std::pair<int32_t, SNet>(sEvent.data.fd, sNet));
	itEven->second.sMapEventBef.insert(std::pair<SNet, SEvent>(sNet, sEvent));
	itEven->second.m_mutexBef->unlock();
	return iRet;
}

int32_t CNetEpollImp::initBase(SNetThreadRes & sNetThreadRes) {
	sNetThreadRes.m_eFd = epoll_create(MAXHAPPENEVENT);
	if (0 > sNetThreadRes.m_eFd) {
		//log  initBase epoll_create
		return RETERROR;
	}
	int32_t iRet = 0;

	std::map<SNet, SEvent>::iterator itBegin = sNetThreadRes.sMapEvent.begin();
	struct sockaddr_in serverAddr;
	while (itBegin != sNetThreadRes.sMapEvent.end()) {
		close(itBegin->second.data.fd);
		itBegin->second.data.fd = socket(AF_INET, SOCK_STREAM, 0);
		if (itBegin->second.data.fd < 0) {
			//log create socket error
			return iRet;
		}
		setNonBlocking(itBegin->second.data.fd);
		itBegin->second.events = EPOLLIN | EPOLLET;

		int32_t iRet = epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_ADD,
				itBegin->second.data.fd, &(itBegin->second));

		if (0 > iRet) {
			//log epoll_ctl add error
			return RETERROR;
		}
		bzero(&serverAddr, sizeof(serverAddr));
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = inet_addr(itBegin->first.strIP.c_str());
		serverAddr.sin_port = htons(itBegin->first.iPort);
		bind(itBegin->second.data.fd, (sockaddr *) &serverAddr,
				sizeof(serverAddr));
		listen(itBegin->second.data.fd, LISTENQ);
		++itBegin;
	}
	return RETOK;
}

int32_t CNetEpollImp::closeBase(SNetThreadRes& sNetThreadRes) {
	std::map<SNet, SEvent>::iterator itBegin = sNetThreadRes.sMapEvent.begin();
	std::map<SNet, SEvent>::iterator itEnd = sNetThreadRes.sMapEvent.begin();
	while (itBegin != itEnd) {
		close(itBegin->second.data.fd);
		itBegin->second.data.fd = NULLSTS;
		++itBegin;
	}
	itBegin = sNetThreadRes.sMapEventBef.begin();
	itEnd = sNetThreadRes.sMapEventBef.begin();
	while (itBegin != itEnd) {
		close(itBegin->second.data.fd);
		itBegin->second.data.fd = NULLSTS;
		++itBegin;
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
				//log
				closeBase(sNetThreadRes);
				close(sNetThreadRes.m_cmdPipo[0]);
				close(sNetThreadRes.m_cmdPipo[1]);
				iRet = EXIT;
				return iRet;

			} else if (NULL != strstr(szCmd, "CLOSEFD")) {
				iRet = closeBase(sNetThreadRes);
				//log
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
				//log
			}
			return iRet;
		}
	} else if ((cmdEvent.events & EPOLLOUT)) {
		//log
	} else if ((cmdEvent.events & EPOLLERR) || (cmdEvent.events & EPOLLHUP)) {
		close(cmdEvent.data.fd);
		cmdEvent.data.fd = NULLSTS;
		//log
		iRet = RETERROR;
		return iRet;
	} else {
		close(cmdEvent.data.fd);
		cmdEvent.data.fd = NULLSTS;
		//log
		iRet = RETERROR;
		return iRet;
	}
	return RETOK;
}
int32_t CNetEpollImp::getAddressBySocket(const int32_t m_socket, SNet& sNet) {
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
	m_cNetEpollImp->destroy();
	return RETOK;
}
CNetEpoll::~CNetEpoll() {
}
