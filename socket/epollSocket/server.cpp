/*****************************************************
 * Author: Ai Hongwei 
 * Email: ufo008ahw@163.com
 * Last modified: 2015-09-02 16:49
 * Filename: server.cpp
 * Description: 
 ****************************************************/

#include <cstdio>
#include <cstring>
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
#include <sys/epoll.h>
#include <sys/shm.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/un.h>
#include <stddef.h>
#include <fcntl.h>
#include <stropts.h>
#include <map>
#include <vector>
using namespace std;

#define BufferSize 1024
#define ListenQLen 1024
#define AddrSize 256
#define MaxEventNum 1000

int initServer(int sockfd, struct addrinfo *aip) {
	struct sockaddr_in *addrIn = (sockaddr_in *)aip->ai_addr;
	char addrStr[AddrSize];
	inet_ntop(aip->ai_family, &(addrIn->sin_addr), addrStr, AddrSize);
	printf("init server addr: %s, port: %d\n", addrStr, ntohs(addrIn->sin_port));

	if (bind(sockfd, aip->ai_addr, aip->ai_addrlen) < 0) {
		perror("bind adress error\n");
		return -1;
	}

	if (aip->ai_socktype == SOCK_STREAM) {
		if (listen(sockfd, ListenQLen) < 0) {
			perror("listen error");
			return -1;
		}
	}

	return 0;
}

void serve(int sockfd, struct addrinfo *aip) {

	// create epoll & register sockfd listen event
	int epollFd = epoll_create(10);
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = sockfd;
	epoll_ctl(epollFd, EPOLL_CTL_ADD, sockfd, &ev);

	// read buffer
	char readBuf[BufferSize];
	strcpy(readBuf, "server get");
	int prefixLen = strlen(readBuf);

	// write buffer
	map<int, string> writeBuf;

	struct epoll_event events[MaxEventNum];
	int eventNum;
	while(1) {

		eventNum = epoll_wait(epollFd, events, MaxEventNum, 1000);
		for(int i = 0;i < eventNum;i++) {
			int eventFd = events[i].data.fd;

			// listen event
			if (eventFd == sockfd) {
				struct sockaddr_in connAddr;
				socklen_t connAddrLen = AddrSize;
				int connfd = accept(sockfd, (sockaddr *)&connAddr, &connAddrLen);

				if (connfd > 0) {
					// print client info
					char addrStr[AddrSize];
					inet_ntop(connAddr.sin_family, &(connAddr.sin_addr), addrStr, connAddrLen);
					printf("client conneted, addr: %s, port: %d\n", addrStr, ntohs(connAddr.sin_port));

					// add read event, edge triger
					ev.events =	EPOLLIN | EPOLLET;
					ev.data.fd = connfd;
					if (epoll_ctl(epollFd, EPOLL_CTL_ADD, connfd, &ev) < 0) {
						perror("epoll event register error");
					}
				}

			} else {
				if (events[i].events & EPOLLIN) {
					int n;
					if ((n = recvfrom(eventFd, readBuf + prefixLen, BufferSize - prefixLen - 1, MSG_DONTROUTE, NULL, NULL)) < 0) {
						perror("recv error");
					} else if (n > 0) {
						// recv don't append 0
						readBuf[prefixLen + n] = '\0';

						if (writeBuf.find(eventFd) == writeBuf.end()) {
							writeBuf[eventFd] = string(readBuf);
						} else {
							writeBuf[eventFd] += "\n" + string(readBuf);
						}
					} else {
						// connect close, remove epoll event
						epoll_ctl(epollFd, EPOLL_CTL_DEL, eventFd, NULL);
						close(eventFd);
					}

				}
			}
		}

		// loop request to handler reader
		for(map<int, string>::iterator it = writeBuf.begin();it != writeBuf.end();it++) {
			int n;
			// use non block
			if ((n = sendto(it->first, it->second.c_str(), it->second.size(),  MSG_DONTWAIT, NULL, NULL)) < 0) {
				perror("send error");
			} else {
				// get left buffer
				if (n == it->second.size()) {
					writeBuf.erase(it);
				} else {
					it->second = it->second.substr(n);
				}
			}
		}
	}

	return;
}


int main(int argc,char **argv)
{
	if (argc != 3) {
		perror("Usage: ./server [addr] [port]");
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
