#include <stdlib.h>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <set>
#include <map>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <stdint.h>
#include <pthread.h>
#include <errno.h>
//#include <atomic>

#include "inifile.h"
#include "STNetEpoll.h"
#include "STThread.h"
#include "STPtr.h"
#include "STMutex.h"

#define		MAXERRORHAPPEN	3
#define		MAXHAPPENEVENT	512
#define  	BUFFSIZE		512
//#define		//log			//
#define		CMDSIZE			512
#define		MAXCMDSIZE		5120
#define		LISTENQ			32

enum {
	EXIT = 2, WARING = 1, OK = 0, ERROR = -1
};
enum {
	NULLSTS = -1
};
enum {
	LISTEN = 1, RUN = 2
};

class CEpollCfg;
class CEpollListenThread;
class CEpollRunThread;

typedef int32_t AI_int;
typedef struct {
	STString strIP;
	int32_t iPort;
} SNet;

typedef struct epoll_event SEvent;
typedef std::map<SNet, SEvent> SMapEvent;

//for listen or other
typedef struct NetThreadRes {
	int32_t m_iThreadID;
	int32_t m_iSocketNum;
	int32_t m_iThreadType;
	int32_t m_eFd;
	int32_t m_cmdPipe[2];
	SEvent m_pipEv;
	int m_iTimeOut;
	char m_szBuffer[CMDSIZE];
	STMutex m_mutex;
	SMapEvent sMapEvent;
	SMapEvent sValiable;
	SEvent m_happenEvent[MAXHAPPENEVENT];
	struct NetThreadRes * pNetThreadRes;
	NetThreadRes() {
		m_eFd = 0;
		m_iThreadID = 0;
		pNetThreadRes = NULL;
		m_iSocketNum = 0;
		m_iThreadType = 0;
		m_iTimeOut = 500;
	}
} SNetThreadRes;

typedef struct {
	char szIP[128];
	int32_t iPort;
} SNetData;

typedef struct NetCommonCfg {
	NetCommonCfg() {
		m_iNetNum = 0;
		strCommon = "Common";
		strNetNum = "IPNum";
	}
	STString strNetNum;
	STString strCommon;
	int32_t m_iNetNum;
} SNetCommonCfg;

typedef struct NetDataCfg {
	NetDataCfg() {
		strIP = "IP";
		strPort = "Port";
	}
	SNetCommon;
	STString strIP;
	STString strPort;
	std::vector<SNetData> m_vectorIP;
} SNetDataCfg;

typedef struct ThreadCfg {
	ThreadCfg() {
		strListenThread = "ListenThreadNum";
		strRunThread = "RunThreadNum";
		m_iListenThread = 0;
		m_iRunThread = 0;
	}
	SNetCommon;
	STString strListenThread;
	STString strRunThread;
	int32_t m_iListenThread;
	int32_t m_iRunThread;
} SThreadCfg;

typedef std::map<uint32_t, SNetThreadRes> SRunEvent;
//typedef std::map<uint32_t, STScopePtr<Impl> > SRunThread;

int initCmd(std::map<STString, void*>& mapCmdMapAct) {
	mapCmdMapAct["EXIT"] = NULL;
	mapCmdMapAct["CLOSE"] = NULL;
	return OK;
}

void setNonBlocking(int sockFd) {
	int opts;
	opts = fcntl(sockFd, F_GETFL);
	if (opts < 0) {
		perror("fcntl(sock,GETFL)");
		//log(Error, "fcntl(sock,GETFL)");
		return;
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(sockFd, F_SETFL, opts) < 0) {
		perror("fcntl(sock,SETFL,opts)");
		//log("fcntl(sock,SETFL,opts)");
		return;
	}
}

class STNetEpoll::Impl {
public:
	Impl() {
	}
	~Impl() {
	}

	int32_t init(const STString& STStrConfigFile) {
		int iRet = cEpollCfg.init(STStrConfigFile);
		if (iRet < OK) {
			//log("");
			return iRet;
		}
		iRet = cEpollCfg.run();
		if (iRet < OK) {
			//log("");
			return iRet;
		}
		//cEpollCfg.m_vectorIP
		int iRet = initDealSock();
		if (iRet < OK) {
			//log("");
			return iRet;
		}
		iRet = initListen();
		if (iRet < OK) {
			//log();
			return iRet;
		}
		return OK;
	}

	int32_t run() {
		int32_t iRet = OK;
		iRet = runDealSock();
		if (iRet < OK) {
			//log
			return ERROR;
		}
		iRet = runListen();
		if (iRet >= OK) {
			//log
			return ERROR;
		}
		return iRet;
	}

	int32_t closeAll() {
		int32_t iRet = OK;
		iRet = closeListen();
		if (iRet < OK) {
			return iRet;
		}
		iRet = closeDealSock();
		if (iRet < OK) {
			return iRet;
		}
		return OK;
	}

	int32_t destroy() {
		cEpollCfg.destroy();
		return closeAll();
	}
public:

private:
	int32_t initListen() {
		int32_t iRet = 0;
		std::map<uint32_t, SNetThreadRes>::iterator it = m_listenEvent.begin();
		while (it != m_listenEvent.end()) {
			iRet = initBase(it->second);
			if (iRet < 0) {
				//log
			}
			++it;
		}
		return iRet;
	}

	int32_t initDealSock() {
		int32_t iRet = OK;
		std::map<uint32_t, SNetThreadRes>::iterator it = m_runEvent.begin();
		while (it != m_runEvent.end()) {
			iRet = initBase(it->second);
			if (iRet < 0) {
				//log
			}
			++it;
		}
		return iRet;
	}

	int32_t closeListen() {
		std::map<uint32_t, SNetThreadRes>::iterator it = m_listenEvent.begin();
		while (it != m_listenEvent.end()) {
			closeBase(it->second);
			++it;
		}
		return OK;
	}

	int32_t closeDealSock() {
		std::map<uint32_t, SNetThreadRes>::iterator it = m_runEvent.begin();
		while (it != m_runEvent.end()) {
			closeBase(it->second);
			++it;
		}
		return OK;
	}
	int32_t runListen() {

		CEpollListenThread cEpollListenThread;
		int32_t iRet = OK;
		std::map<uint32_t, SNetThreadRes>::iterator it = m_listenEvent.begin();
		while (it != m_listenEvent.end()) {

			m_vctListenThread.push_back(cEpollListenThread);
			m_vctListenThread.back().init(it->second, this);
			iRet = m_vctListenThread.back().exec();
			if (iRet != 0) {
				//log
			}
			++it;
		}
		return iRet;
	}
	int32_t runDealSock() {

		CEpollRunThread cEpollRunThread;
		int32_t iRet = OK;
		std::map<uint32_t, SNetThreadRes>::iterator it = m_runEvent.begin();
		while (it != m_runEvent.end()) {
			m_vctRunThread.push_back(cEpollRunThread);
			m_vctRunThread.back().init(it->second, this);
			iRet = m_vctRunThread.back().exec();
			if (iRet != 0) {
				//log
			}
			++it;
		}
		return iRet;
	}

private:
	int32_t runDealSockBase(SNetThreadRes & sNetThreadRes) {
		int32_t iAccpEvNum = 0;
		int32_t it = 0;
		int32_t iReadNum = 0;
		int32_t iRet = ERROR;
		int32_t iErrorCount = 0;
		STString strBuffer;
		while (1) {
			if (sNetThreadRes.sValiable.size() > 0) {
				sNetThreadRes.m_mutex.lock();
				std::map<SNet, SEvent>::iterator itbegin =
						sNetThreadRes.sValiable.begin();
				std::map<SNet, SEvent>::iterator itend =
						sNetThreadRes.sValiable.end();
				while (itbegin != itend) {
					epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_ADD,
							itbegin->second.data.fd, &(itbegin->second));
					sNetThreadRes.sMapEvent[itbegin->first] = itbegin->second;
					++itbegin;
				}

				sNetThreadRes.m_mutex.unlock();
				std::map<uint32_t, AI_int>::iterator it = m_mapSize2thread.find(
						sNetThreadRes.m_iThreadID);
				if (it == m_mapSize2thread.end()) {
					//log
					return ERROR;
				}
				it->second = sNetThreadRes.sMapEvent.size();
				sNetThreadRes.sValiable.clear();

			}
			int32_t iRet = epoll_wait(sNetThreadRes.m_eFd,
					sNetThreadRes.m_happenEvent, MAXHAPPENEVENT,
					sNetThreadRes.m_iTimeOut);
			if (iRet < 0) {
				//log(WARING, "");
				++iErrorCount;
				if (iErrorCount >= MAXERRORHAPPEN) {
					exit (EXIT_FAILURE);
				}
				iRet = closeBase(sNetThreadRes);
				if (iRet < OK) {
					//log("");
					return iRet;
				}
				iRet = initBase(sNetThreadRes);
				if (iRet < OK) {
					//log("");
					return iRet;
				}
				continue;
			}
			iErrorCount = 0;
			for (it = 0; it < iRet; it++) {
				if (sNetThreadRes.m_happenEvent[it].data.fd < 0) {
					continue;
				}
				if (sNetThreadRes.m_happenEvent[it].data.fd
						== sNetThreadRes.m_cmdPipe[0]) {
					iRet = dealCommand(sNetThreadRes.m_happenEvent[it],
							sNetThreadRes);
					if (iRet < OK) {
						//log(ERROR, __FILE__, __LINE__, "run,dealCommand,Return:", iRet);
						return iRet;
					}
					continue;
				}
				if (sNetThreadRes.m_happenEvent[it].events & EPOLLIN) {
					strBuffer = "";
					do {
						memset(sNetThreadRes.m_szBuffer, 0x00, BUFFSIZE);
						iReadNum = read(sNetThreadRes.m_happenEvent[it].data.fd,
								sNetThreadRes.m_szBuffer,
								BUFFSIZE - 1);
						if (iReadNum > 0) {	//read data
							sNetThreadRes.m_szBuffer[iReadNum] = '\0';
							strBuffer += sNetThreadRes.m_szBuffer;
							if (strBuffer.size() > MAXCMDSIZE) {
								//log(buffer too much);
								break;
							}
						} else if (iReadNum < 0) { //Failed to read
							if ((errno == EAGAIN) || (errno == EINTR)) { //No data.
								//log(FALTER, __FILE__, __LINE__, "[data] n < 0, no data, errno= %d", errno);
								break;
							} else {	//The client is closed
								//log(FALTER, __FILE__, __LINE__, "[data] n < 0, peer close, errno=%d", errno);
								SNet sNet;
								iRet = GetAddressBySocket(
										sNetThreadRes.m_happenEvent[it].data.fd,
										sNet);
								if (iRet != 0) {
									//log
									continue;
								}

								epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_DEL,
										sNetThreadRes.m_happenEvent[it].data.fd,
										&sNetThreadRes.m_happenEvent[it]);
								close(sNetThreadRes.m_happenEvent[it].data.fd);
								sNetThreadRes.m_mutex.lock();
								sNetThreadRes.sMapEvent.erase(sNet);
								sNetThreadRes.m_mutex.unlock();
								break;
							}
						} else if (iReadNum == 0) { //The client is closed
							//log(FALTER, __FILE__, __LINE__, "[data] n = 0, peer close, errno=%d", errno);
							SNet sNet;
							iRet = GetAddressBySocket(
									sNetThreadRes.m_happenEvent[it].data.fd,
									sNet);
							if (iRet != 0) {
								//log
								continue;
							}
							epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_DEL,
									sNetThreadRes.m_happenEvent[it].data.fd,
									&sNetThreadRes.m_happenEvent[it]);
							close(sNetThreadRes.m_happenEvent[it].data.fd);
							sNetThreadRes.m_mutex.lock();
							sNetThreadRes.sMapEvent.erase(sNet);
							sNetThreadRes.m_mutex.unlock();
							break;
						}
					} while (1);
				} else if ((sNetThreadRes.m_happenEvent[it].events & EPOLLERR)
						|| (sNetThreadRes.m_happenEvent[it].events & EPOLLHUP)) { //exception happen
					//log (//loger, __FILE__, __LINE__, WARING, "[conn] close listen because epollerr or epollhup");
					SNet sNet;
					iRet = GetAddressBySocket(
							sNetThreadRes.m_happenEvent[it].data.fd, sNet);
					if (iRet != 0) {
						//log
						continue;
					}
					epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_DEL,
							sNetThreadRes.m_happenEvent[it].data.fd,
							&sNetThreadRes.m_happenEvent[it]);

					close(sNetThreadRes.m_happenEvent[it].data.fd);
					sNetThreadRes.m_mutex.lock();
					sNetThreadRes.sMapEvent.erase(sNet);
					sNetThreadRes.m_mutex.unlock();
				} else {
					//log(ERROR, __FILE__, __LINE__, "socket other errro happen  thread:Run to exit");
					iRet = ERROR;
					return iRet;
				}
			}
		}
	}

	int32_t runListenBase(SNetThreadRes & sNetThreadRes) {
		int32_t it = ERROR;
		int32_t iRet = ERROR;
		struct sockaddr_in clientaddr;
		unsigned int uiAddrSize = sizeof(struct sockaddr);
		iRet = initBase(sNetThreadRes);
		if (iRet < 0) {
			//log
			return iRet;
		}
		int32_t iErrorCount = 0;
		while (1) {
			//At the same time an event occurs at most,Unless more than one handle add
			iRet = epoll_wait(sNetThreadRes.m_eFd, sNetThreadRes.m_happenEvent,
			MAXHAPPENEVENT, -1);
			if (iRet < 0) {
				++iErrorCount;
				//log(WARING, "");

				if (iErrorCount >= MAXERRORHAPPEN) {
					exit (EXIT_FAILURE);
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
				if (sNetThreadRes.m_happenEvent[it].data.fd < 0) {
					continue;
				}
				if (sNetThreadRes.m_happenEvent[it].data.fd
						== sNetThreadRes.m_cmdPipe[0]) {
					iRet = dealCommand(sNetThreadRes.m_happenEvent[it],
							sNetThreadRes);
					if (iRet < OK) {
						//log(ERROR, __FILE__, __LINE__, "run,dealCommand,Return:", iRet);
						return iRet;
					}
					continue;
				}
				if (sNetThreadRes.m_happenEvent[it].events & EPOLLIN) //have connection request
						{
					SNet sNet;
					do {
						int iFd;
						iFd = accept(sNetThreadRes.m_happenEvent[it].data.fd,
								(struct sockaddr *) &clientaddr, &uiAddrSize);
						if (iFd > 0) {
							//log(FALTER,__FILE__,__LINE__, "[conn] peer=%s:%d",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
							sNet.strIP = inet_ntoa(clientaddr.sin_addr);
							sNet.iPort = clientaddr.sin_port;
							setNonBlocking(iFd);
							pushSock(iFd, sNet);
						} else {
							if ((errno == EAGAIN) || (errno == EINTR)) //there is no connection request
									{
								continue;
							} else //In other cases the handle error, should be closed and listen again
							{
								//log(//log_ERROR,
								//		"STNetEpoll::Impl close listen because accept fail and errno not equal eagain or eintr.");

								close(sNetThreadRes.m_happenEvent[it].data.fd);
								sNetThreadRes.m_happenEvent[it].data.fd =
										NULLSTS;
								iRet = epoll_ctl(sNetThreadRes.m_eFd,
										EPOLL_CTL_DEL,
										sNetThreadRes.m_happenEvent[it].data.fd,
										&sNetThreadRes.m_happenEvent[it]);
								if (iRet != OK) {
									//log(ERROR,"");
								}

								iRet = initBase(sNetThreadRes);
								if (iRet != OK) {
									//log(ERROR,"");
									exit(ERROR);
								}
								break;
							}
						}
					} while (1);
				}
			}
		}
		return ERROR;
	}

	int32_t pushSock(int iSock, SNet& sNet) {
		std::multimap<AI_int, uint32_t>::iterator it = m_mapthread2Size.begin();
		if (it == m_mapthread2Size.end()) {
//		log("");
			return ERROR;
		}

		std::multimap < uint32_t, AI_int > m_mapSize2thread;
		std::map < AI_int, uint32_t > m_mapthread2Size;

		std::map<uint32_t, SNetThreadRes>::iterator itEven = m_runEvent.find(
				it->second);
		if (itEven == m_runEvent.end()) {
//		log("");
			return ERROR;
		}
		itEven->second.m_mutex.lock();
		itEven->second.sValiable[sNet] = iSock;
		itEven->second.m_mutex.unlock();

		std::map<uint32_t, AI_int>::iterator itSize2thread =
				m_mapSize2thread.find(it->second);
		if (itSize2thread == m_mapSize2thread.end()) {
			//log("");
			return ERROR;
		}
		it->first = itSize2thread->second;

		return OK;
	}

	int32_t initBase(SNetThreadRes & sNetThreadRes) {
		sNetThreadRes.m_eFd = epoll_create(MAXHAPPENEVENT);
		if (0 > sNetThreadRes.m_eFd) {
			return ERROR;
		}
		int32_t iRet = pipe(sNetThreadRes.m_cmdPipe);
		if (iRet != 0) {
			return ERROR;
		}
		sNetThreadRes.m_pipEv.data.fd = sNetThreadRes.m_cmdPipe[0];
		sNetThreadRes.m_pipEv.events = EPOLLIN | EPOLLHUP;

		int32_t iRet = epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_ADD,
				sNetThreadRes.m_cmdPipe[0], &sNetThreadRes.m_pipEv);
		if (0 > iRet) {
//		log("");
			return ERROR;
		}
		std::map<SNet, SEvent>::iterator itBegin =
				sNetThreadRes.sMapEvent.begin();
		std::map<SNet, SEvent>::iterator itEnd = sNetThreadRes.sMapEvent.end();
		struct sockaddr_in serveraddr;
		while (itBegin != itEnd) {
			close(itBegin->second.data.fd);
			itBegin->second.data.fd = socket(AF_INET, SOCK_STREAM, 0);
			if (itBegin->second.data.fd < 0) {
				//log(ERROR, "");
				return iRet;
			}
			setNonBlocking(itBegin->second.data.fd);
			itBegin->second.events = EPOLLIN | EPOLLET;

			int32_t iRet = epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_ADD,
					itBegin->second.data.fd, &(itBegin->second));

			if (0 > iRet) {
				//log
				return ERROR;
			}
			bzero(&serveraddr, sizeof(serveraddr));
			serveraddr.sin_family = AF_INET;
			inet_aton(itBegin->first.strIP.c_str(), &(serveraddr.sin_addr));
			serveraddr.sin_port = htons(itBegin->first.iPort);
			bind(itBegin->second.data.fd, (sockaddr *) &serveraddr,
					sizeof(serveraddr));
			listen(itBegin->second.data.fd, LISTENQ);

			++itBegin;
		}
		return OK;
	}

	int32_t closeBase(SNetThreadRes & sNetThreadRes) {

		std::map<SNet, SEvent>::iterator itevbegin =
				sNetThreadRes.sMapEvent.begin();
		std::map<SNet, SEvent>::iterator itevend =
				sNetThreadRes.sMapEvent.begin();

		while (itevbegin != itevend) {
			close(itevbegin->second.data.fd);
			itevbegin->second.data.fd = NULLSTS;
			++itevbegin;
		}

		itevbegin = sNetThreadRes.sValiable.begin();
		itevend = sNetThreadRes.sValiable.begin();
		while (itevbegin != itevend) {
			close(itevbegin->second.data.fd);
			itevbegin->second.data.fd = NULLSTS;
			++itevbegin;
		}
		return OK;
	}

	int32_t dealCommand(const SEvent & cmdEvent,
			SNetThreadRes & sNetThreadRes) {
		int32_t iRet = 0;
		char szCmd[1024] = "";
		if (cmdEvent.events & EPOLLIN) {
			int32_t iNum = read(cmdEvent.data.fd, &szCmd, sizeof(szCmd));
			if (iNum > 0) {
				if (NULL != strstr(szCmd, "EXIT")) {
					//log("get command thread:Run to exit");
					closeBase(sNetThreadRes);
					close(sNetThreadRes.m_cmdPipe[0]);
					close(sNetThreadRes.m_cmdPipe[1]);
					iRet = EXIT;
					return iRet;

				} else if (NULL != strstr(szCmd, "CLOSEFD")) {
					iRet = closeBase(sNetThreadRes);
					//log("get command thread:Run closed all socked handle");
				} else if (NULL != strstr(szCmd, "INIT")) {
					iRet = closeBase(sNetThreadRes);
					if (iRet < OK) {
						return iRet;
					}
					close(sNetThreadRes.m_cmdPipe[0]);
					close(sNetThreadRes.m_cmdPipe[1]);
					iRet = initBase(sNetThreadRes);
					if (iRet < OK) {
						return iRet;
					}
					//log("other error happen thread:Run ");
				}
				return iRet;
			}
		} else if ((cmdEvent.events & EPOLLERR)
				|| (cmdEvent.events & EPOLLHUP)) {
			close(cmdEvent.data.fd);
			cmdEvent.data.fd = NULLSTS;
			//log(pipe read error happen thread:Run to exit");
			iRet = ERROR;
			return iRet;
		} else {
			close(cmdEvent.data.fd);
			cmdEvent.data.fd = NULLSTS;
			//log("pipe other errro happen  thread:Run to exit");
			iRet = ERROR;
			return iRet;
		}
		return OK;
	}
	int32_t GetAddressBySocket(const int32_t m_socket, SNet& sNet) {
		struct sockaddr_in address;
		memset(&address, 0, sizeof(address));
		unsigned int uiAddrLen = sizeof(address);
		if (::getpeername(m_socket, (sockaddr*) &address, &uiAddrLen) != 0) {
			//log();
			return WARING;
		}
		sNet.strIP = ::inet_ntoa(address.sin_addr);
		sNet.iPort = address.sin_port;
		return WARING;
	}
private:

	SRunEvent m_listenEvent;
	SRunEvent m_runEvent;

	std::map<pthread_t, uint32_t> m_mapSize2thread;
	std::multimap<uint32_t, pthread_t> m_mapthread2Size;

	std::vector<CEpollListenThread> m_vctListenThread;
	std::vector<CEpollRunThread> m_vctRunThread;

	CEpollCfg cEpollCfg;
};

class CEpollListenThread: public STThread {
public:

public:
	int32_t init(SNetThreadRes & sNetThreadRes, STNetEpoll::Impl* impl) {
		m_imp = impl;
		m_pNetThreadRes = &sNetThreadRes;
		return OK;
	}
	SNetThreadRes* m_pNetThreadRes;
	STNetEpoll::Impl * m_imp;
	void main() {
		m_imp->runListen();

	}
private:

};

class CEpollRunThread: public STThread {
public:
	int32_t init(SNetThreadRes & sNetThreadRes, STNetEpoll::Impl* impl) {
		if (NULL == impl) {
			//log
			return ERROR;
		}
		m_imp = impl;
		m_pNetThreadRes = &sNetThreadRes;

		return OK;
	}
	SNetThreadRes* m_pNetThreadRes;
	STNetEpoll::Impl * m_imp;
	void main() {
		m_imp->runDealSockBase(*m_pNetThreadRes);
	}
private:

};

class CEpollCfg {
public:
	int32_t init(const STString& strFile) {
		m_strFile = strFile;
		std::map<SNetCommonCfg, int>::iterator it = m_mapIPNum.begin();

		if (it != m_mapIPNum.end()) {
			m_sNetCommonCfg.m_iNetNum = read_profile_int(
					m_sNetCommonCfg.strCommon.c_str(),
					m_sNetCommonCfg.strNetNum.c_str(), 0, m_strFile.c_str());
			if (0 != m_sNetCommonCfg.m_iNetNum) {
				//log
				return ERROR;
			}
		}
		m_sThreadCfg.m_iListenThread = read_profile_int(
				it->first.strCommon.c_str(),
				m_sThreadCfg.strListenThread.c_str(), 0, m_strFile.c_str());
		if (0 != m_sThreadCfg.m_iListenThread) {
			//log
			return ERROR;
		}
		m_sThreadCfg.m_iRunThread = read_profile_int(
				it->first.strCommon.c_str(), m_sThreadCfg.strRunThread.c_str(),
				0, m_strFile.c_str());
		if (0 != m_sThreadCfg.m_iRunThread) {
			//log
			return ERROR;
		}
		return OK;
	}
	int32_t run() {
		SNetData sNetData;
		m_sNetDataCfg.m_vectorIP.clear();
		STString STStrNum;
		while (m_sNetCommonCfg.m_iNetNum > 0) {
			STStrNum << "IP_" << m_sNetCommonCfg.m_iNetNum;
			if (!read_profile_string(STStrNum.c_str(),
					m_sNetDataCfg.strIP.c_str(), sNetData.szIP,
					sizeof(sNetData.szIP), "", m_strFile.c_str())) {
				//log
				return ERROR;
			}
			if (!read_profile_int(STStrNum.c_str(),
					m_sNetDataCfg.strPort.c_str(), 0, m_strFile.c_str())) {
				//log
				return -ERROR;
			}
			m_sNetDataCfg.m_vectorIP.push_back(sNetData);
			--m_sNetCommonCfg.m_iNetNum;
		}
		return OK;
	}
	int32_t destroy() {
		return OK;
	}
public:

	STString m_strFile;
	SNetCommonCfg m_sNetCommonCfg;
	SThreadCfg m_sThreadCfg;
	std::map<NetCommonCfg, int> m_mapIPNum;
	SNetDataCfg m_sNetDataCfg;
};

