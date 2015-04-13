#include <vector>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <stdint.h>

#include "STNetEpoll.h"
#include "STThread.h"
class STNetEpoll::Impl {
public:
	Impl(const STString& STStrConfigFile) {

	}
	~Impl() {
	}
	virtual bool Init() {
        m_efd = epoll_create1(0);
        ST_IF (-1 != m_efd) {
        	return false;
        	}

        int32 iRet =  pipe(m_cmdPipe);
        ST_IF(iRet != 0) {
        	return false;
        }

        m_event.data.fd = m_cmdPipe[0];
        m_event.events = EPOLLIN | EPOLLHUP;

        int32 iRet = epoll_ctl(m_efd, EPOLL_CTL_ADD, m_cmdPipe[0], &m_event);
        ST_IF (-1 != iRet) {
        	return false;
        	}

		return true;
	}
	virtual bool Run() {
		ST_WHILE (1) {
			int32 iRet = epoll_wait(m_efd, m_eventArr, MaxEventCount, -1);
			ST_FOR(int32 i=0; i<n; i++) {
		    	ST_IF ((m_eventArr[i].data.fd == m_cmdPipe[0]) && (m_eventArr[i].events & EPOLLIN)) {
		    		ThreadCmd cmd = ThreadCmd_Count;
		            ST_IF (read(m_cmdPipe[0], &cmd, sizeof(cmd)) > 0) {
		            	ST_IF (ThreadCmd_Exit == cmd) {
		                            //ask thread to exit
		            		break;
		                }
		            }

		                  ST_IF (m_eventArr.size() > 0) {
		                        m_fdTaskLock.lock();
		                        ST_FOR (unsigned int32 j=0; j<m_eventArr.size(); ++j) {
		                        	ST_SWITCH (m_eventArr[j].cmd) {
		                            case FdCmd_Add:
		                                epoll_event event;
		                                event.data.fd = m_eventArr[j].fd;
		                                event.events = EPOLLIN | EPOLLHUP;//just care about in and disconnect

		                                epoll_ctl(m_efd, EPOLL_CTL_ADD, m_eventArr[j].fd, &event);
		                                break;
		                            case FdCmd_Remove:
		                                epoll_ctl(m_efd, EPOLL_CTL_DEL, m_eventArr[j].fd, NULL);
		                                break;
		                            default:
		                                break;
		                            }
		                        }
		                        m_fdTaskLock.unlock();
		                    }
		                }
		            	ST_ELIF (eventArr[i].events & EPOLLIN) {
		                    m_owner->fdChanged(eventArr[i].data.fd, STFdListeenerBase::FdChangeType_CanRead);
		                }
		            	ST_ELIF (eventArr[i].events & EPOLLHUP) {
		                    m_owner->fdChanged(eventArr[i].data.fd, STFdListeenerBase::FdChangeType_UnAvailable);
		                }
		            }
		        }
	}
	virtual bool Destroy() {
		close(m_efd);
	}
public:

	bool startListen(const STString& ip, int port) {
		//TBD
		struct sockaddr_in serveraddr;
		m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
		ST_IF (m_listenfd < 0)
		{
			return false;
		}
		setnonblocking(m_listenfd);
		ev.data.fd = m_listenfd;
		ev.events = EPOLLIN | EPOLLET;
		epoll_ctl(epfd, EPOLL_CTL_ADD, m_listenfd, &ev);

		bzero(&serveraddr, sizeof(serveraddr));
		    serveraddr.sin_family = AF_INET;
		inet_aton(m_listenIp.c_str(), &(serveraddr.sin_addr)); //htons(SERV_PORT);
		serveraddr.sin_port = htons(SERV_PORT);
		bind(m_listenfd, (sockaddr *)&serveraddr, sizeof(serveraddr));
		listen(m_listenfd, LISTENQ);
		while (1)
		{
			epoll_wait
		}



		return false;
	}

public:
	bool startEvent() {
		return false;
	}

private:
	int32 m_efd;
	int32 m_listenfd;
	struct epoll_event m_event;
	struct epoll_event m_ev
	int32 m_cmdPipe[2];
	socket_t m_ListenCtl;
	socket_t m_EventCtl;
	epoll_event  m_eventArr[MaxEventCount];
//	int16	m_nListenThreadNum;
//	int16	m_nEventThreadNum;
	STString m_listenIp;
	int32 m_listenPort;
};

/*
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
 */
