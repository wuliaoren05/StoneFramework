#include <vector>
#include <set>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <stdint.h>

#include "STNetEpoll.h"
#include "STThread.h"
#include "STPtr.h"
#include "STMutex.h"

#define		MAXERRORHAPPEN	3
#define  	MAXSOCKETEVENT	512
#define		MAXLISTENEVENT	512
#define  	BUFFSIZE		512
#define		log			//enum {WARING = 1, OK = 0, ERROR = -1};typedef struct {	STString StrIP;
	int32 iPort;
} Snet;
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
		log(Error, "fcntl(sock,GETFL)");
		return WARING;
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(sockFd, F_SETFL, opts) < 0) {
		perror("fcntl(sock,SETFL,opts)");
		log("fcntl(sock,SETFL,opts)");
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
		m_epollEFd = epoll_create(MAXSOCKETEVENT);
		if (-1 != m_socketFd) {
			return ERROR;
		}
		m_listenEFd = epoll_create(MAXLISTENEVENT);
		if (-1 != m_listenFd) {
			return ERROR;
		}
		int32 iRet = pipe(m_cmdPipe);
		if (iRet != 0) {
			return ERROR;
		}
		m_event.data.fd = m_cmdPipe[0];
		m_event.events = EPOLLIN | EPOLLHUP;

		int32 iRet = epoll_ctl(m_efd, EPOLL_CTL_ADD, m_cmdPipe[0], &m_event);
		if (-1 != iRet) {
			return ERROR;
		}
		return true;
	}
	int32 dealCommand(const epoll_event & cmdEvent) {
		if (cmdEvent.events & EPOLLIN) {
			int32 iNum = read(cmdEvent.data.fd, &cmd, sizeof(cmd));
			if (iNum > 0) {
				if (NULL != strstr(cmd, "EXIT")) {
					log("get command thread:Run to exit");
					iRet = OK;
					return iRet;
				} else if (NULL != strstr(cmd, "CLOSEFD")) {
					std::set<epoll_event>::iterator itSet =
							m_setDoneEvent.begin();
					while (itSet != m_setDoneEvent.end()) {
						close(itSet->data.fd);
					}
					m_setDoneEvent.clear();
					log("get command thread:Run closed all socked handle");
				} else {
					log("other error happen thread:Run ");
					iRet = ERROR;
					return iRet;
				}
			}
		} else if (cmdEvent.events & EPOLLERR || cmdEvent.events & EPOLLHUP) {
			close(cmdEvent.data.fd);
			log(pipe read error happen thread:Run to exit");
					iRet = ERROR;
					return iRet;
				} else {
					close(cmdEvent.data.fd);
					log("pipe other errro happen  thread:Run to exit");
					iRet = ERROR;
					return iRet;
				}
			}
			int initCmd(std::map<STString, void*>& mapCmdMapAct) {
				mapCmdMapAct["EXIT"] = NULL;
				mapCmdMapAct["CLOSE"] = NULL;
				return OK;
			}
			virtual bool run() {
				int32 iAccpEvNum = 0;
				int32 it = 0;
				int32 iReadNum = 0;
				int32 iRet = ERROR;
				int32 iErrorCount = 0;
				while (1) {
					int32 iRet = epoll_wait(m_epollEFd, m_socketEvent,
					MAXLISTENEVENT, m_iTimeOut);
					if (iRet < 0) {
						log(WARING, "");
						++iErrorCount;
						if (iErrorCount >= MAXERRORHAPPEN) {
							exit (EXIT_FAILURE);
						}
						m_epollEFd = epoll_create(MAXLISTENEVENT);
						if (m_listenEFd < 0) {
							log(ERROR, "");
							return ERROR;
						}
						struct epoll_event m_connEv;
						std::set<epoll_event>::iterator it =
								m_setDoneEvent.begin();
						while (it != m_setDoneEvent.end()) {
							if (epoll_ctl(m_epollEFd, EPOLL_CTL_ADD,
									(*it).data.fd, &(*it)) == -1) {
								log(ERROR, "epoll_ctl: listen_sock");
								return EXIT_FAILURE;
							}
							++it;
						}
						continue;
					}
					iErrorCount = 0;
					for (it = 0; it < iRet; it++) {
						if (m_socketEvent[it].data.fd == m_cmdPipe[0]) {
							iRet = dealCommand(m_socketEvent);
							if (iRet < OK) {
								log(ERROR, __FILE__, __LINE__, "run,dealCommand,Return:", iRet);
								return iRet;
							}
						} else if (m_socketEvent[it].events & EPOLLIN) {
							do {
								iReadNum = read(m_socketEvent[it].data.fd,
										m_szbuffer, BUFFSIZE - 1);
								if (iReadNum > 0)	//读到数据
										{
									m_szbuffer[iReadNum] = '\0';
									//
								} else if (iReadNum < 0) //Failed to read
										{
									if (errno == EAGAIN)	//No data.
											{
										log(FALTER, __FILE__, __LINE__, "[data] n < 0, no data, errno= %d", errno);
										break;
									} else if (errno == EINTR)//可能被内部中断信号打断,经过验证对非阻塞socket并未收到此错误,应该可以省掉该步判断
											{
										log(FALTER, __FILE__, __LINE__, "[data] n < 0, interrupt, errno=%d", errno);
										continue;
									} else	//The client is closed
									{
										log(FALTER, __FILE__, __LINE__, "[data] n < 0, peer close, errno=%d", errno);
										close(m_socketEvent[it].data.fd);
										epoll_ctl(m_epollFd, EPOLL_CTL_DEL,
												m_socketEvent[it].data.fd,
												&m_socketEvent[it]);
										m_setDoneEvent.erase(m_socketEvent[it]);
										break;
									}
								} else if (iReadNum == 0) //The client is closed
										{
									log(FALTER, __FILE__, __LINE__, "[data] n = 0, peer close, errno=%d", errno);
									close(events[i].data.fd);
									epoll_ctl(m_efd, EPOLL_CTL_DEL,
											events[i].data.fd, &events[i]);
									m_setDoneEvent.erase(events[i]);
									break;
								}
							} while (1);
						} else if (events[i].events & EPOLLERR
								|| events[i].events & EPOLLHUP) //exception happen
										{
							log (loger, __FILE__, __LINE__, WARING, "[conn] close listen because epollerr or epollhup");
							epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd,
									&events[i]);
							close(events[i].data.fd);
							m_setDoneEvent.erase(events[i]);
						} else {
							log(ERROR, __FILE__, __LINE__, "socket other errro happen  thread:Run to exit");
							iRet = ERROR;
							return iRet;
						}
					}
				}
			}
			int32 closeAll() {
				int32 iRet = ERROR;
				iAccpEvNum = m_setDoneEvent.size();
				if (iAccpEvNum > 0) {
					m_mutex.lock();
					for (it = 0; it < iAccpEvNum; it++) {
						m_listEvent[it].events = EPOLLIN | EPOLLET;
						iRet = epoll_ctl(epfd, EPOLL_CTL_DEL,
								m_setDoneEvent[it].data.fd,
								&m_setDoneEvent[it]);
						if (iRet < 0) {
							log(WARING, __FILE__, __LINE__, "closeAll,epoll_ctl,iRet:%d", iRet);
						}
						m_setDoneEvent.erase(m_setDoneEvent[it]);
					}
					m_mutex.unlock();
				}
				return OK;
			}
			virtual int32 Destroy() {
				iRet = closeAll();
				close (m_socketFd);
				close(m_listenFd);
				close(m_cmdPipe[0]);
				close(m_cmdPipe[1]);
				return iRet;
			}
		public:
			bool restartAndListen(int32& iEFd) {
				int iRet = ERROR;
				struct sockaddr_in serveraddr;
				std::map<Snet, epoll_event>::iterator it =
						m_mapListenEvent.begin();
				while (it != m_mapListenEvent.end) {
					close(it->second.data.fd);
					it->second.data.fd = socket(AF_INET, SOCK_STREAM, 0);
					if (it->second.data.fd < 0) {
						log(ERROR, "");
						return iRet;
					}
					setNonBlocking(it->second.data.fd);
					it->second.events = EPOLLIN | EPOLLET;
					epoll_ctl(iEFd, EPOLL_CTL_ADD, m_listenEv.data.fd,
							&m_listenEv);
					bzero(&serveraddr, sizeof(serveraddr));
					serveraddr.sin_family = AF_INET;
					inet_aton(it->first.strIP.c_str(), &(serveraddr.sin_addr)); //htons(SERV_PORT);
					serveraddr.sin_port = htons(it->first.iPort);
					bind(m_listenEv.data.fd, (sockaddr *) &serveraddr,
							sizeof(serveraddr));
					listen(m_listenEv.data.fd, LISTENQ);
					++it;
				}
				return true;
			}

			bool startListen() {
				int32 it = ERROR;
				int32 iRet = ERROR;
				struct sockaddr clientaddr;
				int32 iAddSize = sizeof(struct sockaddr);
				restartAndListen (m_efd);
				int32 iErrorCount = 0;
				while (1) {
					//At the same time an event occurs at most,Unless more than one handle add
					iRet = epoll_wait(m_listenEFd, m_listenEvent,
					MAXLISTENEVENT, -1);
					if (iRet < 0) {
						++iErrorCount;
						log(WARING, "");
						m_listenEFd = epoll_create(MAXLISTENEVENT);
						if (m_listenEFd < 0) {
							log(ERROR, "");
							return ERROR;
						}
						//restartAndListen(m_listenEFd, strIP, iPort);

						if (iErrorCount >= MAXERRORHAPPEN) {
							exit (EXIT_FAILURE);
						}
						std::set<epoll_event>::iterator it =
								m_setListenEvent.begin();
						while (it != m_setListenEvent.end()) {
							iRet = epoll_ctl(m_listenEFd, EPOLL_CTL_DEL,
									*it.data.fd, &(*it));
							++it;
						}
					}
					iErrorCount = 0;
				for (it=0;it<iRet;<i++)
				{
					if (m_listenEvent[it].data.fd < 0)
					{
						continue;
					}
					if (m_listenEvent[i].events&EPOLLIN) //have connection request
					{
						do
						{
							m_connEv.data.fd = accept(m_listenEvent[it].data.fd,(sockaddr *)&clientaddr, &iAddSize);
							if (m_connEv.data.fd > 0)
							{
								log(FALTER,__FILE__,__LINE__, "[conn] peer=%s:%d",inet_ntoa(clientaddr.sin_addr),ntohs(clientaddr.sin_port));
								setNonBlocking(m_connEv.data.fd);
								m_connEv.events=EPOLLIN|EPOLLET;
								m_listEvent.push_bask(m_connEv);
							}
							else
							{
								if (errno == EAGAIN) //there is no connection request
								{
									continue;
								}
								else if (errno == EINTR) //可能被中断信号打断,,经过验证对非阻塞socket并未收到此错误,应该可以省掉该步判断
								{
									continue;
								}
								else //In other cases the handle error, should be closed and listen again
								{
									log(LOG_ERROR,
											"STNetEpoll::Impl close listen because accept fail and errno not equal eagain or eintr.");
									//此时说明该描述字已经出错了,需要重新创建和监听
									close(m_listenEvent[it].data.fd);
									iRet = epoll_ctl(m_listenEFd,EPOLL_CTL_DEL,m_listenEvent[it].data.fd,&m_listenEvent[it]);
									if (iRet != OK ) {
										log(ERROR,"");
									}
									iRet = restartAndListen(m_listenEFd, STString, port);
									if (iRet != OK ) {
										log(ERROR,"");
										exit(ERROR);
									}
									break;
								}
							}
						}while (1);
					}
				}
			}
			return ERROR;
		}
	protected:
		int32 CMD

	private:
		int32 m_epollEFd;
		int32 m_listenEFd;

		struct epoll_event m_listenEv;
		struct epoll_event m_connEv;

		int32 m_cmdPipe[2];
		int32 m_listenFd;

		epoll_event m_socketEvent[MAXSOCKETEVENT];
		epoll_event m_listenEvent[MAXLISTENEVENT];

		STString m_strIP;
		int32 m_listenPort;

		std::vector<epoll_event> m_listEvent;
		std::set<epoll_event> m_setDoneEvent;
		std::map<Snet, epoll_event> m_mapListenEvent;
		typedef struct epoll_event m_listenEv  SEvent;
		typedef std::map<Snet, epoll_event> MapEvent;
		std::map<SEvent,MapEvent> m_epollListEvent;

		int m_iTimeOut;
		char m_szbuffer[BUFFSIZE];
		STMutex m_mutex;

//			STScopePtr<STThread> m_stThread;
	};

