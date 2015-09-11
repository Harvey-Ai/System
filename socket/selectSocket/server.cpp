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
#include <stropts.h>
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
		perror("bind adress error");
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

void serve(int sockfd, struct addrinfo *aip) {
	int pid;
	char *serverName = "./serverSock";
	char *clientName = "./clientSock";

	if ((pid = fork()) < 0) {
		perror("fork conn server failed\n");
	} else if (pid > 0) {
		int unixfd, unixConnFd;
		if ((unixfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
			perror("unix domain sock error");
			exit(0);
		}
		unlink(serverName);

		struct sockaddr_un un;
		memset(&un, 0, sizeof(un));
		un.sun_family = AF_UNIX;
		strcpy(un.sun_path, serverName);
		int len = offsetof(struct sockaddr_un, sun_path) + strlen(serverName);
		if (bind(unixfd, (struct sockaddr *)&un, len) < 0) {
			perror("unix domain sock bind error");
			exit(1);
		}
		if (listen(unixfd, 1000) < 0) {
			perror("unix domain sock listen error");
			exit(1);
		}

		struct sockaddr_un unixConnAddr;
		socklen_t unixConnAddrLen;
		if ((unixConnFd = accept(unixfd, (sockaddr *)&unixConnAddr, &unixConnAddrLen)) > 0) {

			// only connect client process
			if (strcmp(unixConnAddr.sun_path, clientName) != 0) {
				close(unixConnFd);
			} else {
				cout << "get unix socket connect" << endl;
				fcntl(unixConnFd, F_SETFL, O_NONBLOCK);
			}
		}

		struct sockaddr_in connAddr;
		socklen_t connAddrLen = addrSize;
		int connfd;

		vector<int> sockSendQ;
		// can't share info in multi process by global var 
		while((connfd = accept(sockfd, (sockaddr *)&connAddr, &connAddrLen) > 0)) {
			// print client adress info
			char addrStr[addrSize];
			inet_ntop(connAddr.sin_family, &(connAddr.sin_addr), addrStr, connAddrLen);
			printf("client conneted, addr: %s, port: %d\n", addrStr, ntohs(connAddr.sin_port));

			sockSendQ.push_back(connfd);

			if (write_fd(unixConnFd, sockSendQ.back()) >= 0) {
				printf("send socket %d succuess\n", sockSendQ.back());
				sockSendQ.pop_back();
			}

		}

		int status;
		waitpid(pid, &status, 0);
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
			perror("unix domain sock client bind error");
			exit(0);
		}

		memset(&un, 0, sizeof(un));
		un.sun_family = AF_UNIX;
		strcpy(un.sun_path, serverName);
		len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
		if (connect(unixSock, (struct sockaddr *)&un, len) < 0) {
			perror("unix socket client error\n");
			exit(0);
		} else {
			cout << "get unix sock connect" << endl;
			fcntl(unixSock, F_SETFL, O_NONBLOCK);
		}

		map<int, string>  writeBuf;
		set<int> connSocketSet;
		fd_set readSet, writeSet;
		FD_ZERO(&readSet);
		FD_ZERO(&writeSet);

		// handle data message
		while(1) {
			int fd = read_fd(unixSock);
			if (fd != -1) {
				printf("get socket %d\n", fd);
				connSocketSet.insert(fd);
				FD_SET(fd, &readSet);
			}

			if (connSocketSet.size() == 0) {
				continue;
			}

			// block, set last parameter to use nonBlock
			int nfd = select(*(--connSocketSet.end()) + 1, &readSet, NULL, NULL, NULL);
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

					cout << *it << endl;
					if ((n = recvfrom(*it, recvBuf + prefixLen, bufferSize - prefixLen - 1, MSG_DONTROUTE, NULL, NULL)) < 0) {
						perror("receive error");
					}
					recvBuf[n + prefixLen] = '\n';	

					cout << recvBuf << endl;
					if (strcmp(recvBuf, "server get end") == 0) {
						FD_CLR(*it, &readSet);
						FD_CLR(*it, &writeSet);
						close(*it);
					} else {
						writeBuf[*it] = string(recvBuf);
						FD_SET(*it, &writeSet);
					}
				}

				if (FD_ISSET(*it, &writeSet)) {
					if (sendto(*it, writeBuf[*it].c_str(), writeBuf[*it].size(),  MSG_DONTROUTE, NULL, 0) < 0) {
						perror("send error");
					}
				}
			}
		}
	}
}


int main(int argc,char **argv)
{
	if (argc != 3) {
		printf("Usage: ./server [addr] [port] \n \n");
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
		perror("Get addr info error \n");
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

