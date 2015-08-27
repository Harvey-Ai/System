/*****************************************************
 * Author: Ai Hongwei 
 * Email: ufo008ahw@163.com
 * Last modified: 2015-08-27 03:43
 * Filename: server.cpp
 * Description: server part
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
using namespace std;

#define bufferSize 1024
#define listenQLen 1024
#define addrSize 256

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

void serve(int sockfd, struct addrinfo *aip) {
	struct sockaddr_in connAddr;
	socklen_t connAddrLen = addrSize;
	int connfd;

	while(1) {
		if ((connfd = accept(sockfd, (sockaddr *)&connAddr, &connAddrLen)) > 0) {
			char addrStr[addrSize];
			inet_ntop(connAddr.sin_family, &(connAddr.sin_addr), addrStr, connAddrLen);
			printf("client conneted, addr: %s, port: %d\n", addrStr, ntohs(connAddr.sin_port));

			int pid = fork();
			// start connect server
			if (pid < 0) {
				perror("fork conn server failed\n");
			}
			else if (pid == 0) {
				char sendBuf[bufferSize], recvBuf[bufferSize];
				memset(sendBuf, 0, sizeof(sendBuf));
				memset(recvBuf, 0, sizeof(recvBuf));
				strcpy(recvBuf, "server get ");
				int prefixLen = strlen(recvBuf);
				int n;

				while(strcmp(sendBuf, "server get end") != 0) {
					if ((n = recvfrom(connfd, recvBuf + prefixLen, bufferSize - prefixLen - 1, MSG_DONTROUTE, NULL, NULL)) < 0) {
						perror("receive error");
					}

					recvBuf[n + prefixLen] = '\0';
					strcpy(sendBuf, recvBuf);
					if (sendto(connfd, sendBuf, strlen(recvBuf),  MSG_DONTROUTE, NULL, NULL) < 0) {
						perror("send error");
					}
				}

				close(connfd);
				printf("client closed, addr: %s, port: %d\n", addrStr, connAddr.sin_port);
				exit(0);

			} else {
				int status;
				waitpid(pid, &status, 0);
				close(connfd);
			}
		}
	}
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

