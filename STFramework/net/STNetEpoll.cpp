#include <vector>
#include <set>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <stdint.h>
#include <thread.h>
#include <atomic>

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

enum {
	WARING = 1, OK = 0, ERROR = -1
};
enum {
	NULLSTS = -1
};
enum {
	LISTEN = 1, RUN = 2
};

typedef std::atomic<std::int_least32_t> AI_int;
typedef struct {
	STString StrIP;
	int32 iPort;
} SNet;

typedef struct epoll_event SEvent;
typedef std::map<SNet, SEvent> SMapEvent;

//for listen or other
typedef struct NetThreadRes {
	int32 iThreadID;
	int32 isocketNum;
	int32 iThreadType;
	int32 m_eFd;
	int32 m_cmdPipe[2];
	SEvent m_pipEv;
	int m_iTimeOut;
	char m_szBuffer[CMDSIZE];
	STMutex m_mutex;
	SMapEvent sMapEvent;
	SMapEvent sValiable;
	SEvent m_happenEvent[MAXHAPPENEVENT];
	struct NetThreadRes * pNetThreadRes;
	NetThreadRes() {
		pNetThreadRes = NULL;
		isocketNum = 0;
	}
} SNetThreadRes;

typedef std::map<thread_t, SNetThreadRes> SRunEvent;

int initCmd(std::map<STString, void*>& mapCmdMapAct) {
	mapCmdMapAct["EXIT"] = NULL;
	mapCmdMapAct["CLOSE"] = NULL;
	return OK;
}

void setNonBlocking(int sockFd) {
	int opts;
	opts = fcntl(sockFd, F_GETFL);
	if (opts < 0) {
		perror(Error, "fcntl(sock,GETFL)");
		//log(Error, "fcntl(sock,GETFL)");
		return WARING;
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(sockFd, F_SETFL, opts) < 0) {
		perror("fcntl(sock,SETFL,opts)");
		//log("fcntl(sock,SETFL,opts)");
		return WARING;
	}
}

class STNetEpoll::Impl {
public:
	Impl(const STString& STStrConfigFile) {
	}
	~Impl() {
	}

	virtual int32 Init() {
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

	virtual int32 run() {
		int32 iRet = OK;

//				multithread();
//				runDealSock();
//				runListen();

		return iRet;
	}

	int32 close() {
		int32 iRet = OK;
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

	virtual int32 Destroy() {
		return close();
	}
public:

private:
	virtual int32 initListen() {
		std::map<thread_t, SNetThreadRes>::iterator it = m_listenEvent.begin();
		while (it != SRunEvent.end()) {
			iRet = initBase(it->second);
			if (iRet < 0) {
				//log
			}
			++it;
		}
		return iRet;
	}

	virtual int32 initDealSock() {
		int32 iRet = OK;
		std::map<thread_t, SNetThreadRes>::iterator it = m_runEvent.begin();
		while (it != SRunEvent.end()) {
			iRet = initBase(it->second);
			if (iRet < 0) {
				//log
			}
			++it;
		}
		return iRet;
	}

	int32 closeListen() {
		std::map<thread_t, SNetThreadRes>::iterator it = m_listenEvent.begin();
		while (it != m_listenEvent.end()) {
			closeBase(it->second);
			++it;
		}
		return OK;
	}

	int32 closeDealSock() {
		std::map<thread_t, SNetThreadRes>::iterator it = m_runEvent.begin();
		while (it != m_runEvent.end()) {
			closeBase(it->second);
			++it;
		}
		return OK;
	}
	int32 runListen() {
		int32 iRet = OK;
		std::map<thread_t, SNetThreadRes>::iterator it = m_listenEvent.begin();
		while (it != SRunEvent.end()) {
			iRet = runListenBase(it->second);
			if (iRet < 0) {
				//log
			}
			++it;
		}
		return iRet;
		return OK;
	}
	int32 runDealSock() {
		int32 iRet = OK;
		std::map<thread_t, SNetThreadRes>::iterator it = m_runEvent.begin();
		while (it != SRunEvent.end()) {
			iRet = runDealSockBase(it->second);
			if (iRet < 0) {
				//log
			}
			++it;
		}
		return iRet;
	}

private:
	virtual int32 runDealSockBase(SNetThreadRes & sNetThreadRes) {
		int32 iAccpEvNum = 0;
		int32 it = 0;
		int32 iReadNum = 0;
		int32 iRet = ERROR;
		int32 iErrorCount = 0;
		STString strBuffer;
		while (1) {
			int32 iRet = epoll_wait(sNetThreadRes.m_eFd,
					sNetThreadRes.m_happenEvent, MAXLISTENEVENT,
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
					iRet = dealCommand(sNetThreadRes.m_cmdPipe[0]);
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
								sNetThreadRes.m_szbuffer,
								BUFFSIZE - 1);
						if (iReadNum > 0) {	//read data
							m_szbuffer[iReadNum] = '\0';
							strBuffer += m_szbuffer;
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
				} else if (sNetThreadRes.m_happenEvent[it].events & EPOLLERR
						|| sNetThreadRes.m_happenEvent[it].events & EPOLLHUP) { //exception happen
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

	int32 runListenBase(SNetThreadRes & sNetThreadRes) {
		int32 it = ERROR;
		int32 iRet = ERROR;
		struct sockaddr clientaddr;
		const int32 iAddrSize = sizeof(struct sockaddr);
		iRet = initBase(sNetThreadRes);
		if (iRet < 0) {
			//log
			return iRet;
		}
		int32 iErrorCount = 0;
		while (1) {
			//At the same time an event occurs at most,Unless more than one handle add
			iRet = epoll_wait(sNetThreadRes.m_eFd, sNetThreadRes.m_happenEvent,
					MAXLISTENEVENT, -1);
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
			for (it = 0; it < iRet; i++) {
				if (sNetThreadRes.m_happenEvent[it].data.fd < 0) {
					continue;
				}
				if (sNetThreadRes.m_happenEvent[it].data.fd
						== sNetThreadRes.m_cmdPipe[0]) {
					iRet = dealCommand(sNetThreadRes.m_cmdPipe[0]);
					if (iRet < OK) {
						//log(ERROR, __FILE__, __LINE__, "run,dealCommand,Return:", iRet);
						return iRet;
					}
					continue;
				}
				if (sNetThreadRes.m_happenEvent[it].events & EPOLLIN) //have connection request
						{
					do {
						int iFd;
						iFd = accept(sNetThreadRes.m_happenEvent[it].data.fd,
								(sockaddr *) &clientaddr, &iAddrSize);
						if (iFd > 0) {
							//log(FALTER,__FILE__,__LINE__, "[conn] peer=%s:%d",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
							SNet sNet;
							sNet.StrIP = inet_ntoa(clientaddr.sin_addr);
							sNet.iPort = clientaddr.sin_port;
							setNonBlocking(iFd);
							pushSock(iFd, sNet);
						} else {
						if (errno == EAGAIN) || (errno == EINTR) //there is no connection request
						{
							continue;
						}
						else //In other cases the handle error, should be closed and listen again
						{
							//log(//log_ERROR,
							//		"STNetEpoll::Impl close listen because accept fail and errno not equal eagain or eintr.");
							//此时说明该描述字已经出错了,需要重新创建和监听

							close(sNetThreadRes.m_happenEvent[it].data.fd);
							m_listenEvent[it].data.fd = NULLSTS;
							iRet = epoll_ctl(sNetThreadRes.m_eFd,EPOLL_CTL_DEL,sNetThreadRes.m_happenEvent[it].data.fd,&sNetThreadRes.m_happenEvent[it]);
							if (iRet != OK ) {
								//log(ERROR,"");
							}

							iRet = initBase(sNetThreadRes);
							if (iRet != OK ) {
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
int32 pushSock(int iSock, SNet& sNet) {
	std::multimap<AI_int, thread_t>::iterator it = m_mapSize.begin();
	if (it == m_mapSize.end()) {
		log("");
		return ERROR;
	}
	++(it->first);
	thread_t threadID = it->second;
	std::map<thread_t, SNetThreadRes>::iterator itEven = m_runEvent.find(
			threadID);
	if (itEven == m_runEvent.end()) {
		log("");
		return ERROR;
	}
	itEven->second.m_mutex.lock();
	itEven->second.sValiable[sNet] = iSock;
	itEven->second.unlock()
	return OK;
}

virtual int32 initBase(SNetThreadRes & sNetThreadRes) {
	sNetThreadRes.m_eFd = epoll_create(MAXHAPPENEVENT);
	if (0 > m_eFd) {
		return ERROR;
	}
	int32 iRet = pipe(sNetThreadRes.m_cmdPipe);
	if (iRet != 0) {
		return ERROR;
	}
	sNetThreadRes.m_pipEv.data.fd = sNetThreadRes.m_cmdPipe[0];
	sNetThreadRes.m_pipEv.events = EPOLLIN | EPOLLHUP;

	int32 iRet = epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_ADD,
			sNetThreadRes.m_cmdPipe[0], &sNetThreadRes.m_pipEv);
	if (0 > iRet) {
		log("");
		return ERROR;
	}
	std::map<Snet, SEvent>::iterator itBegin = sNetThreadRes.sMapEvent.begin();
	std::map<Snet, SEvent>::iterator itEnd = sNetThreadRes.sMapEvent.end();

	while (itBegin != itEnd) {
		close(itBegin->second.data.fd);
		itBegin->second.data.fd = socket(AF_INET, SOCK_STREAM, 0);
		if (itBegin->second.data.fd < 0) {
			//log(ERROR, "");
			return iRet;
		}
		setNonBlocking(itBegin->second.data.fd);
		itBegin->second.events = EPOLLIN | EPOLLET;

		int32 iRet = epoll_ctl(sNetThreadRes.m_eFd, EPOLL_CTL_ADD,
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

int32 closeBase(SNetThreadRes & sNetThreadRes) {

	std::map<Snet, SEvent>::iterator itevbegin =
			sNetThreadRes.sMapEvent.begin();
	std::map<Snet, SEvent>::iterator itevend = sNetThreadRes.sMapEvent.begin();

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

int32 dealCommand(const SEvent & cmdEvent) {
	if (cmdEvent.events & EPOLLIN) {
		int32 iNum = read(cmdEvent.data.fd, &cmd, sizeof(cmd));
		if (iNum > 0) {
			if (NULL != strstr(cmd, "EXIT")) {
				//log("get command thread:Run to exit");
				iRet = OK;
				return iRet;
			} else if (NULL != strstr(cmd, "CLOSEFD")) {
				std::set<SEvent>::iterator itSet = m_setDoneEvent.begin();
				while (itSet != m_setDoneEvent.end()) {
					close(itSet->data.fd);
					itSet->data.fd = NULLSTS;
				}
				m_setDoneEvent.clear();
				//log("get command thread:Run closed all socked handle");
			} else {
				//log("other error happen thread:Run ");
				iRet = ERROR;
				return iRet;
			}
		}
	} else if (cmdEvent.events & EPOLLERR || cmdEvent.events & EPOLLHUP) {
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
}
int32 GetAddressBySocket(const int32 m_socket, SNet& sNet) {
	SOCKADDR_IN address;
	memset(&address, 0, sizeof(address));
	int nAddrLen = sizeof(address);
	if (::getpeername(m_socket, (SOCKADDR*) &address, &nAddrLen) != 0) {
		//log();
		return WARING;
	}
	sNet.StrIP = ::inet_ntoa(m_address.sin_addr);
	sNet.iPort = m_address.sin_port;
	return WARING;
}
private:

SRunEvent m_listenEvent;
SRunEvent m_runEvent;
std::multimap<AI_int, thread_t> m_mapSize;
//		STScopePtr<STThread> m_stThread;
};

