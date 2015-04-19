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

class STNetEpoll::Impl {
public:
	Impl(const STString& STStrConfigFile) {

	}
	~Impl() {
	}
	virtual bool Init() {
		m_efd = epoll_create1(0);
		if(-1 != m_efd)
		{
			return false;
		}
		m_listenfd = epoll_create1(0);
		if (-1 != m_efd)
		{
			return false;
		}
		int32 iRet = pipe(m_cmdPipe);
		if (iRet != 0)
		{
			return false;
		}

		m_event.data.fd = m_cmdPipe[0];
		m_event.events = EPOLLIN | EPOLLHUP;

		int32 iRet = epoll_ctl(m_efd, EPOLL_CTL_ADD, m_cmdPipe[0], &m_event);
		if (-1 != iRet)
		{
			return false;
		}

		return true;
	}
	virtual bool Run() {
		int iAccpEvNum = 0;
		int it = 0;
		int iReadNum = 0;
		ST_WHILE(1)
		{
			int32 iRet = epoll_wait(m_efd, m_eventArr, MaxEventCount,
					m_iTimeOut);
			ST_FOR( it=0; i < iRet;
					it++
					)
					{
						if  (m_eventArr[i].data.fd == m_cmdPipe[0])
						{
							if (m_eventArr[i].events & EPOLLIN)
							{
								ThreadCmd cmd = read(m_cmdPipe[0], &cmd, sizeof(cmd);
								if  (sizeof (cmd)) > 0)
								{
									if  (EXIT == cmd)
									{
										//ask thread to exit
										//	break;
									}
									else (CLOSEFD == cmd)
									{
										//	break;
										std::set<>::iterator itSet = m_DoneEvent.begin();
										while (itSet != m_DoneEvent.end())
										{
											close(itSet->data.fd);
										}
										m_DoneEvent.clear();
									}
//									continue;
								}
							}
							else if ()
							{

							}
							else
							{

							}
						}
						else if (events[i].events&EPOLLERR || events[i].events&EPOLLHUP)//有异常发生
						{
							cout << "[conn] close listen because epollerr or epollhup" << errno << endl;

							close(events[i].data.fd);
							epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&events[i]);
							m_DoneEvent.erase(events[i].data.fd);
						}
						else (eventArr[i].events & EPOLLIN)
						{
							do
							{
								iReadNum = read(eventArr[i].data.fd,m_szbuffer,sizeof(m_szbuffer));
								if (iReadNum > 0)	//读到数据
								{
									m_szbuffer[iReadNum] = '\0';

									//综合下面两种情况,在读到字节数大于0时必须继续读,不管读到字节数是否等于接收缓冲区大小,
									//也不管错误代码是否为EAGAIN,否则要么导致关闭事件丢失,要么导致后续数据的丢失
									if (n < MAXLINE)
									{
										//经过验证,如果对方发送完数据后就断开,即使判断是否错误代码为EAGAIN,也会导致close事件丢失,
										//必须继续读,以保证断开事件的正常接收
										cout << "[data] n > 0, read less recv buffer size, errno=" << errno << ",len=" << n << ", data=" << line << endl;
									}
									else
									{
										//经过验证,发送字节数大于等于接收缓冲区时,读到字节数为接收缓冲区大小,错误代码为EAGAIN,
										//必须继续读,以保证正常接收后续数据
										cout << "[data] n > 0, read equal recv buffer size, errno=" << errno << ",len=" << n << ", data=" << line << endl;
									}
								}
								else if (iReadNum < 0) //读取失败
								{
									if (errno == EAGAIN)	//没有数据了
									{
										cout << "[data] n < 0, no data, errno=" << errno << endl;

										break;
									}
									else if(errno == EINTR)	//可能被内部中断信号打断,经过验证对非阻塞socket并未收到此错误,应该可以省掉该步判断
									{
										cout << "[data] n < 0, interrupt, errno=" << errno << endl;
									}
									else	//客户端主动关闭
									{
										cout << "[data] n < 0, peer close, errno=" << errno << endl;

										close(events[i].data.fd);
										epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&events[i]);
										m_DoneEvent.erase(events[i].data.fd);
										break;
									}
								}
								else if (iReadNum == 0) //客户端主动关闭
								{
									cout << "[data] n = 0, peer close, errno=" << errno << endl;
									//同一连接可能会出现两个客户端主动关闭的事件,一个errno是EAGAIN(11),一个errno是EBADF(9),
									//对错误的文件描述符EBADF(9)进行关闭操作不会有什么影响,故可以忽略,以减少errno判断的开销
									close(events[i].data.fd);
									epoll_ctl(m_efd,EPOLL_CTL_DEL,events[i].data.fd,&events[i]);
									m_DoneEvent.erase(events[i].data.fd);
									break;
								}
							}while (1);
						}
					}

					iAccpEvNum = m_listEvent.size();
					if (iAccpEvNum > 0)
					{	//lock();
						for (it = 0; it < iAccpEvNum; it++) {
							m_listEvent[it].events = EPOLLIN | EPOLLET;
							epoll_ctl(epfd, EPOLL_CTL_ADD, connfd,
									&m_listEvent[it]);
						}
						m_DoneEvent.merge(m_listEvent);
						//unlock();
					}
				}
			}
			virtual bool Destroy() {
				std::string m_strRCmd, m_strWCmd;
				m_strListcmd = "close";
				write(m_cmdPipe, szcmd, sizeof(szcmd));
				std::string m_strRCmd, m_strWCmd;
				close(m_efd);
				close(m_cmdPipe[0]);
				close(m_cmdPipe[1]);
			}
		public:
			bool reStartAndListen(int32& iefd, const STString& ip, int port) {
				struct sockaddr_in serveraddr;
				m_listenfd = socket(AF_INET, SOCK_STREAM, 0);
				if (m_listenfd < 0)
				{
					return false;
				}
				setnonblocking(m_listenfd);
				ev.data.fd = m_listenfd;
				ev.events = EPOLLIN | EPOLLET;
				epoll_ctl(iefd, EPOLL_CTL_ADD, m_listenfd, &ev);

				bzero(&serveraddr, sizeof(serveraddr));
				serveraddr.sin_family = AF_INET;
				inet_aton(m_listenIp.c_str(), &(serveraddr.sin_addr)); //htons(SERV_PORT);
				serveraddr.sin_port = htons(SERV_PORT);
				bind(m_listenfd, (sockaddr *) &serveraddr, sizeof(serveraddr));
				listen(m_listenfd, LISTENQ);
				return true;
			}

			bool startListen(const STString& ip, int port) {
				//TBD
				reStartAndListen(m_efd, STString, port);
				int32 it = 0;
				while (1) {
					epoll_wait();
					int32 iRet = epoll_wait(m_efd, m_eventArr, MaxEventCount,
							-1);
					for (it=0;it<iRet;<i++)
					{
						if (m_eventArr[it].data.fd < 0)
						{
							continue;
						}
						if (m_eventArr[i].events&EPOLLIN)	//有连接到来
						{
							do
							{
								clilen = sizeof(struct sockaddr);
								connfd = accept(m_eventArr,(sockaddr *)&clientaddr, &clilen);
								if (connfd > 0)
								{
									cout << "[conn] peer=" << inet_ntoa(clientaddr.sin_addr) << ":" << ntohs(clientaddr.sin_port) << endl;
									log(LOG_ERROR,
											"STNetEpoll::Impl close listen because accept fail and errno not equal eagain or eintr.");

									//把socket设置为非阻塞方式
									setnonblocking(connfd);
									//设置用于读操作的文件描述符
									ev.data.fd=connfd;
									//设置用于注测的读操作事件
									ev.events=EPOLLIN|EPOLLET;
									//注册ev
									m_listEvent.push_bask(ev);
								}
								else
								{

									if (errno == EAGAIN)	//没有连接需要接收了
									{
										break;
									}
									else if (errno == EINTR)//可能被中断信号打断,,经过验证对非阻塞socket并未收到此错误,应该可以省掉该步判断
									{
									}
									else	//其它情况可以认为该描述字出现错误,应该关闭后重新监听
									{

										log(LOG_ERROR,
												"STNetEpoll::Impl close listen because accept fail and errno not equal eagain or eintr.");
										//此时说明该描述字已经出错了,需要重新创建和监听
										close(events[i].data.fd);

										epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&events[i]);

										reStartAndListen(m_efd, STString, port);
										//重新监听
										break;
									}
								}
							}while (1);
						}

					}
					if (iRet < 0) {
						if (errno == EINTR) {
							continue;
						}

						if (errno == EBADF) {
							log(LOG_ERROR,
									"STNetEpoll::Impl startListen:epfd is not a valid file descriptor.");
							reStartAndListen(m_efd, STString, port);
						}
						if (errno == EFAULT) {
							log("...");
							reStartAndListen(m_efd, STString, port);
						}
						if (errno == EINVAL) {
							log(LOG_ERROR,
									"STNetEpoll::Impl startListen:The memory area pointed to by events is not accessible with write permissions.");
							break;
						}

					}
				}
				return false;
			}

		public:
			int main() {

			}

		private:
			int32 m_efd;
			int32 m_listenfd;
			struct epoll_event m_event;
			struct epoll_event m_ev
			int32 m_cmdPipe[2];
			socket_t m_ListenCtl;
			socket_t m_EventCtl;
			epoll_event m_eventArr[MaxEventCount];
//	int16	m_nListenThreadNum;
//	int16	m_nEventThreadNum;
			STString m_listenIp;
			int32 m_listenPort;
			std::vector<epoll_event> m_listEvent;
			std::ser<epoll_event> m_DoneEvent;
			int m_iTimeOut;
			char m_szbuffer[256];

			STScopePtr<STThread> m_stThread;
		};

