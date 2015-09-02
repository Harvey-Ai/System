/*****************************************************
 * Author: Ai Hongwei 
 * Email: ufo008ahw@163.com
 * Last modified: 2015-08-28 04:18
 * Filename: server.cpp
 * Description: 
 ****************************************************/

#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <cstdlib>
#include <cstring>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/shm.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <stddef.h>
#include <fcntl.h>
#include <set>
#include <map>
#include <vector>
#include "scm_rights.h"
using namespace std;

#define bufferSize 1024
#define listenQLen 1024
#define addrSize 256

struct  shared_buffer {
	int num;
	int socket[1024];
};

int initServer(int sockfd, struct addrinfo *aip) {
	struct sockaddr_in *addrIn = (sockaddr_in *)aip->ai_addr;
	char addrStr[addrSize];
	inet_ntop(aip->ai_family, &(addrIn->sin_addr), addrStr, addrSize);
	printf("init server addr: %s, port: %d\n", addrStr, ntohs(addrIn->sin_port));

	if (bind(sockfd, aip->ai_addr, aip->ai_addrlen) < 0) {
		perror("bind adress error\n");
		return -1;
	}

	if (aip->ai_socktype == SOCK_STREAM) {
		if (listen(sockfd, listenQLen) < 0) {
			perror("listen error");
			return -1;
		}
	}

	return 0;
}

static int setnonblocking (int fd)
{
	int opts;

	if( fd<=0 )
		return -1;

	opts = fcntl( fd, F_GETFL );
	if( opts<0 )
		return -1;

	if( fcntl( fd, F_SETFL, opts|O_NONBLOCK ) < 0 )
		return -1;

	return 0;
}

void serve(int sockfd, struct addrinfo *aip) {
	int pid;
	char *serverName = "/home/ufo008ahw/serverSock";
	char *clientName = "/home/ufo008ahw/clientSock";

	if ((pid = fork()) < 0) {
		perror("fork conn server failed\n");
	} else if (pid > 0) {
		int unixfd, unixConnFd;
		if ((unixfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
			perror("unix socket error");
			exit(0);
		}
		unlink(serverName);

		struct sockaddr_un un;
		memset(&un, 0, sizeof(un));
		un.sun_family = AF_UNIX;
		strcpy(un.sun_path, serverName);
		int len = offsetof(struct sockaddr_un, sun_path) + strlen(serverName);
		if (bind(unixfd, (struct sockaddr *)&un, len) < 0) {
			perror("bind");
		}
		if (listen(unixfd, 10) < 0) {
			perror("listen");
		}

		struct sockaddr_un unixConnAddr;
		un.sun_family = AF_UNIX;
		socklen_t unixConnAddrLen = 1;
		if ((unixConnFd = accept(unixfd, (sockaddr *)&unixConnAddr, &unixConnAddrLen)) < 0) {
			perror("connect fail");
			exit(0);
		}

		setnonblocking(unixConnFd);
		// only connect client process
		/*
		   if (strcmp(unixConnAddr.sun_path, clientName) != 0) {
		   cout << unixConnAddrLen << endl;
		   cout << unixConnAddr.sun_path << endl;
		   close(unixConnFd);
		   cout <<"??" << endl;
		   } else {
		   */
		cout << "unix socket server connect" << endl;
		//	fcntl(unixConnFd, F_SETFL, O_NONBLOCK);
		//		break;
		//		}


		/*
		   int sendfd = socket(AF_INET, SOCK_STREAM, 0);
		   if (ioctl(temp, I_SENDFD, &sendfd) < 0) {
		   cout << (errno == EAGAIN) << endl;
		   cout << (errno == EBADF) << endl;
		   cout << (errno == EINVAL) << endl;
		   cout << (errno == ENXIO) << endl;
		   cout << (errno == ENOTTY) << endl;
		   cout << errno << endl;		
		   perror("server ioctl");
		   cout << "server ioctl: " << aa.sun_path << " " << temp  << " " << sendfd << endl;
		   printf("strerror: %s\n", strerror(errno));
		   } else {
		   cout << "god job!" << endl;
		   }
		   */

		struct sockaddr_in connAddr;
		socklen_t connAddrLen = addrSize;
		int connfd;

		vector<int> sockSendQ;
		// can't share info in multi process by global var 
		while((connfd = accept(sockfd, (sockaddr *)&connAddr, &connAddrLen)) > 0) {
			// print client adress info
			char addrStr[addrSize];
			inet_ntop(connAddr.sin_family, &(connAddr.sin_addr), addrStr, connAddrLen);
			printf("client conneted, addr: %s, port: %d\n", addrStr, ntohs(connAddr.sin_port));

			sockSendQ.push_back(connfd);

			if (write_fd(unixConnFd, sockSendQ.back()) >= 0) {
				printf("send socket %d succuess\n", sockSendQ.back());
				sockSendQ.pop_back();
				close(connfd);
			} else {
				perror("write fail");
			}

		}

		int status;
		waitpid(pid, &status, 0);
		//	shmctl(shm_id, IPC_RMID, NULL);
	} else {
		unlink(clientName);

		int unixSock = socket(AF_UNIX, SOCK_STREAM, 0);
		struct sockaddr_un un;
		memset(&un, 0, sizeof(un));
		un.sun_family = AF_UNIX;
		strcpy(un.sun_path, clientName);
		int len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
		
		// bind client sock
		if (bind(unixSock, (struct sockaddr *)&un, len) < 0) {
			perror("sock client bind");
			exit(0);
		}

		memset(&un, 0, sizeof(un));
		un.sun_family = AF_UNIX;
		strcpy(un.sun_path, serverName);
		len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
		if (connect(unixSock, (struct sockaddr *)&un, len) < 0) {
			perror("unix socket client error\n");
			exit(0);
		}

		// set sock to nonblock
		setnonblocking(unixSock);
		cout << "unix sock client connect" << endl;	
		//fcntl(unixSock, F_SETFL, O_NONBLOCK);

		/*
		   if (ioctl(fd, I_RECVFD, &recvfd) < 0) {
		   perror("client ioctl");
		   cout << "client ioctl: " << un.sun_path << " " << fd  << " " << recvfd.fd << endl;
		   printf("strerror: %s\n", strerror(errno));
		   }
		   */

		// set select to nonblock
		timeval selectTv;
		selectTv.tv_sec = 0;
		selectTv.tv_usec = 0;

		// init select fd
		map<int, string>  writeBuf;
		set<int> connSocketSet;
		fd_set readSet, writeSet;
		FD_ZERO(&readSet);
		FD_ZERO(&writeSet);

		/*
		// attach share memory
		int shm_id = shmget(shm_key, sizeof(shared_buffer), 0400 | IPC_CREAT);
		if (shm_id < 0) {
		perror("get shared memory error!\n");
		return;
		}
		shared_buffer *buffer_ptr = (shared_buffer *)shmat(shm_id, (void *)0, 0);
		if (buffer_ptr == (void *)-1)
		{
		printf("strerror: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
		}
		*/

		// handle data message
		while(1) {
			/*
			   int fd;
			   if (buffer_ptr->num > 0) {
			// !assume atomic
			fd = buffer_ptr->socket[buffer_ptr->num - 1];
			buffer_ptr->num--;

			int newfd = dup(fd);
			fd = recvfd.fd;
			connSocketSet.insert(fd);
			FD_SET(fd, &readSet);
			}
			*/

			//sleep(1);
			int fd = read_fd(unixSock);
			if (fd != -1) {
				printf("get socket %d\n", fd);
				connSocketSet.insert(fd);
			}

			for(set<int>::iterator it = connSocketSet.begin();it != connSocketSet.end();it++) {
				FD_SET(*it, &readSet);
			}

			if (connSocketSet.size() == 0) {
				continue;
			}

			// block, set last parameter to use nonBlock
			int nfd = select(*(--connSocketSet.end()) + 1, &readSet, &writeSet, NULL, &selectTv);
			if (nfd < 0) {
				perror("select error");
				continue;
			}

			for(set<int>::iterator it = connSocketSet.begin();it != connSocketSet.end();it++) {
				if (FD_ISSET(*it, &readSet)) {
					char recvBuf[bufferSize];
					memset(recvBuf, 0, sizeof(recvBuf));
					strcpy(recvBuf, "server get ");
					int prefixLen = strlen(recvBuf);
					int n;

					if ((n = recvfrom(*it, recvBuf + prefixLen, bufferSize - prefixLen - 1, MSG_DONTROUTE, NULL, NULL)) < 0) {
						perror("receive error");
					} else {
						recvBuf[n + prefixLen] = '\0';	

						if (n == 0 || strcmp(recvBuf, "server get end") == 0) {
							FD_CLR(*it, &readSet);
							FD_CLR(*it, &writeSet);
							connSocketSet.erase(it);
							close(*it);
						} else {
							writeBuf[*it] = string(recvBuf);
							FD_SET(*it, &writeSet);
						}
					}
				}

				if (FD_ISSET(*it, &writeSet)) {
					if (sendto(*it, writeBuf[*it].c_str(), writeBuf[*it].size(),  MSG_DONTROUTE, NULL, NULL) < 0) {
						perror("send error");
					}
					FD_CLR(*it, &writeSet);
				}
			}
		}
		// shmdt(buffer_ptr);
	}
}


int main(int argc,char **argv)
{
	if (argc != 3) {
		printf("Usage: ./server [addr] [port]");
		return 0;
	}

	// get socket
	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket() error!\n");
		exit(1);
	}

	// get client addinfo
	struct addrinfo hint;
	struct addrinfo *adList;
	hint.ai_family = AF_INET;
	hint.ai_socktype = SOCK_STREAM;
	hint.ai_flags = AI_CANONNAME;
	// argv[1]: host, argv[2]: serve
	if (getaddrinfo(argv[1], argv[2], &hint, &adList) != 0) {
		perror("Get addr info error\n");
		exit(1);
	}

	bool serverWork = false;
	for(addrinfo *adPtr = adList;adPtr != NULL;adPtr = adPtr->ai_next) {
		if (adPtr->ai_family == AF_INET) {
			if (initServer(sockfd, adPtr) == 0) {
				cout << "server start" << endl;
				serve(sockfd, adPtr);
				cout << "server end" << endl;
				serverWork = true;
				break;
			}
		}
	}

	if (!serverWork) {
		perror("No workable addr");
	}

	freeaddrinfo(adList);
	close(sockfd);
	return 0;
}        
