#include <sys/socket.h>    // for socket
#include <stdio.h>        // for printf
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero
#include <arpa/inet.h>
#include <unistd.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include  <netinet/in.h>
#include <netinet/tcp.h> // for TCP_NODELAY

#define BUFFER_SIZE 128

#define DELAYTIME	30
void handle_pipe(int sig)
{
	printf("catch sing pipe\n");
}
;

struct sigaction m_sigact;
void setNonBlocking(int sockFd)
{
	int opts;
	opts = fcntl(sockFd, F_GETFL);
	if (opts < 0) {
		perror("fcntl(sock,GETFL)");
		//log(RETERROR, "fcntl(sock,GETFL)");
		return;
	}
	opts = opts | O_NONBLOCK;
	if (fcntl(sockFd, F_SETFL, opts) < 0) {
		perror("fcntl(sock,SETFL,opts)");
		//log("fcntl(sock,SETFL,opts)");
		return;
	}
}

int32_t socketRecv(int sockfd, char* szBuff, int32_t nLen)
{
	int nLeft = nLen;
	int nRead = 0;
	int nCount = 0;
	while (nLeft > 0) {
		nRead = read(sockfd, szBuff, nLeft);
		if (nRead < 0) {
			if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) {
				++nCount;
				if (nCount > 100) return -1;
				continue;
			}
			//log
			return nRead;                  // error, return < 0
		}
		else if (nRead == 0) {
			return -1;;                                  // EOF
		}
		else {
			nCount = 0;
			nLeft -= nRead;
			szBuff += nRead;
		}
	}
	return (nLen - nLeft);               // return >= 0
}

int32_t socketSend(int32_t iSFd, char* szBuf, int32_t nLen)
{
	int nLeft = nLen;
	int nWrite = 0;
	while (nLeft > 0) {
		nWrite = write(iSFd, szBuf, nLeft);
		if (nWrite < 0) {
			if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN) continue;
			//log
			return nWrite;                  // error, return <= 0
		}
		else if (nWrite == 0) {
			return -1;
		}
		else {
			nLeft -= nWrite;
			szBuf += nWrite;
		}
	}
	return (nLen - nLeft);          // return >= 0
}

int conn_nonb(int sockfd, struct sockaddr* saptr, socklen_t salen, int nsec)
{
	int flags, n, error, code, opt;
	socklen_t len;
	fd_set wset;
	struct timeval tval;
	len = sizeof(unsigned int);
	flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
	len = sizeof(unsigned int);
	setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, &opt, len);
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, &opt, len);
	//EINPROCESS
	error = 0;
	if ((n == connect(sockfd, saptr, salen)) == 0) {
		goto done;
	}
	else if (n < 0 && errno != EINPROGRESS) {
		return (-1);
	}

	FD_ZERO(&wset);
	FD_SET(sockfd, &wset);
	tval.tv_sec = nsec;
	tval.tv_usec = 0;

	if ((n = select(sockfd + 1, NULL, &wset, NULL, nsec ? &tval : NULL)) == 0) {
		close(sockfd); /* timeout */
		errno = ETIMEDOUT;
		return (-1);
	}

	if (FD_ISSET(sockfd, &wset)) {
		len = sizeof(error);
		code = getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &len);

		if (code < 0 || error) {
			close(sockfd);
			if (error) errno = error;
			return (-1);
		}
	}
	else {
		fprintf(stderr, "select error: sockfd not set");
		exit(0);
	}

	done: fcntl(sockfd, F_SETFL, flags); /* restore file status flags */
	return (0);
}
int main(int argc, char **argv)
{
	if (argc != 3) {
		printf("Usage: ./%s ServerIPAddress Port\n", argv[0]);
		exit(1);
	}

	m_sigact.sa_handler = handle_pipe;
	sigemptyset(&m_sigact.sa_mask);
	m_sigact.sa_flags = 0;
	sigaction(SIGPIPE, &m_sigact, NULL);
	int iCount = 0;
	while (1) {
		char buffer[BUFFER_SIZE];
		int length = 0;
		int client_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (client_socket < 0) {
			printf("Create Socket Failed!\n");
			sleep(1);
			continue;
		}
		struct sockaddr_in server_addr;
		bzero(&server_addr, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		if (inet_aton(argv[1], &server_addr.sin_addr) == 0) {
			printf("Server IP Address Error!\n");
			exit(1);
		}
		server_addr.sin_port = htons(atoi(argv[2]));
		socklen_t server_addr_length = sizeof(server_addr);

		int res = conn_nonb(client_socket, (struct sockaddr*) &server_addr,
				server_addr_length, DELAYTIME);
		usleep(200000);
		if (res == 0) {
			printf(" Connect To success \n");
			while (1) {
				sprintf(buffer, "this is client : %d:\n", iCount);
				printf("send data: %s\n", buffer);
				length = socketSend(client_socket, buffer, sizeof(buffer));
				if (length < 0) {
					printf("socketSend happen some error \n");
					sleep(4);
					break;
				}
				printf("length:%d\n", length);
				bzero(buffer, BUFFER_SIZE);
				length = socketRecv(client_socket, buffer, BUFFER_SIZE);
				if (length < 0) {
					printf("socketRecv happen some error \n");
					sleep(4);
					break;
				}
				printf("length:%d\n", length);
				printf("Recieve Data From Server %d:%s\n", length, buffer);
				bzero(buffer, BUFFER_SIZE);
				++iCount;
				if (iCount % 5 == 0) break;
			}
			printf("Recieve From Server[%s] Finished\n", argv[1]);
		}
		else {
			printf("connect error \n");
		}
		sleep(1);
		close(client_socket);
	}
	return 0;
}

