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
						else if (events[i].events&EPOLLERR || events[i].events&EPOLLHUP)//���쳣����
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
								if (iReadNum > 0)	//��������
								{
									m_szbuffer[iReadNum] = '\0';

									//�ۺ������������,�ڶ����ֽ�������0ʱ���������,���ܶ����ֽ����Ƿ���ڽ��ջ�������С,
									//Ҳ���ܴ�������Ƿ�ΪEAGAIN,����Ҫô���¹ر��¼���ʧ,Ҫô���º������ݵĶ�ʧ
									if (n < MAXLINE)
									{
										//������֤,����Է����������ݺ�ͶϿ�,��ʹ�ж��Ƿ�������ΪEAGAIN,Ҳ�ᵼ��close�¼���ʧ,
										//���������,�Ա�֤�Ͽ��¼�����������
										cout << "[data] n > 0, read less recv buffer size, errno=" << errno << ",len=" << n << ", data=" << line << endl;
									}
									else
									{
										//������֤,�����ֽ������ڵ��ڽ��ջ�����ʱ,�����ֽ���Ϊ���ջ�������С,�������ΪEAGAIN,
										//���������,�Ա�֤�������պ�������
										cout << "[data] n > 0, read equal recv buffer size, errno=" << errno << ",len=" << n << ", data=" << line << endl;
									}
								}
								else if (iReadNum < 0) //��ȡʧ��
								{
									if (errno == EAGAIN)	//û��������
									{
										cout << "[data] n < 0, no data, errno=" << errno << endl;

										break;
									}
									else if(errno == EINTR)	//���ܱ��ڲ��ж��źŴ��,������֤�Է�����socket��δ�յ��˴���,Ӧ�ÿ���ʡ���ò��ж�
									{
										cout << "[data] n < 0, interrupt, errno=" << errno << endl;
									}
									else	//�ͻ��������ر�
									{
										cout << "[data] n < 0, peer close, errno=" << errno << endl;

										close(events[i].data.fd);
										epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&events[i]);
										m_DoneEvent.erase(events[i].data.fd);
										break;
									}
								}
								else if (iReadNum == 0) //�ͻ��������ر�
								{
									cout << "[data] n = 0, peer close, errno=" << errno << endl;
									//ͬһ���ӿ��ܻ���������ͻ��������رյ��¼�,һ��errno��EAGAIN(11),һ��errno��EBADF(9),
									//�Դ�����ļ�������EBADF(9)���йرղ���������ʲôӰ��,�ʿ��Ժ���,�Լ���errno�жϵĿ���
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
						if (m_eventArr[i].events&EPOLLIN)	//�����ӵ���
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

									//��socket����Ϊ��������ʽ
									setnonblocking(connfd);
									//�������ڶ��������ļ�������
									ev.data.fd=connfd;
									//��������ע��Ķ������¼�
									ev.events=EPOLLIN|EPOLLET;
									//ע��ev
									m_listEvent.push_bask(ev);
								}
								else
								{

									if (errno == EAGAIN)	//û��������Ҫ������
									{
										break;
									}
									else if (errno == EINTR)//���ܱ��ж��źŴ��,,������֤�Է�����socket��δ�յ��˴���,Ӧ�ÿ���ʡ���ò��ж�
									{
									}
									else	//�������������Ϊ�������ֳ��ִ���,Ӧ�ùرպ����¼���
									{

										log(LOG_ERROR,
												"STNetEpoll::Impl close listen because accept fail and errno not equal eagain or eintr.");
										//��ʱ˵�����������Ѿ�������,��Ҫ���´����ͼ���
										close(events[i].data.fd);

										epoll_ctl(epfd,EPOLL_CTL_DEL,events[i].data.fd,&events[i]);

										reStartAndListen(m_efd, STString, port);
										//���¼���
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

