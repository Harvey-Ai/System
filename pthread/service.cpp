#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <iostream>
#include <queue>
using namespace std;

int taskNum = 0;
double waitTime = 0;
queue <int> task;
queue <timeval> taskStart;
pthread_mutex_t mutex1;

void *serve(void *);

int main(void)
{
	int sfd; char buf[1024]; int n, i;
	struct sockaddr_in serv_addr, cli_addr;
	socklen_t len;
	timeval start;

	sfd = socket(AF_INET, SOCK_DGRAM, 0);

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(8000);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(sfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

	pthread_t threads[4];
	int threadID[4];
	for(int i = 0;i < 4;i++)
		pthread_create(&threads[i], NULL, serve, &threadID[i]);

	pthread_mutex_init(&mutex1, NULL);

	while(1)
	{
		len = sizeof(cli_addr);
		int length = recvfrom(sfd, buf, 1024, MSG_DONTWAIT, (struct sockaddr *)&cli_addr, &len);
		
		if (length > 0)
		{
			switch(buf[0])
			{
				case 'A' : {task.push(0);gettimeofday(&start, NULL);taskStart.push(start);}
				case 'B' : {task.push(1);gettimeofday(&start, NULL);taskStart.push(start);}
				case 'C' : {task.push(2);gettimeofday(&start, NULL);taskStart.push(start);}
				case 'D' : {task.push(3);gettimeofday(&start, NULL);taskStart.push(start);}
				case 'E' : break;

			}
		}
	}

	for(int i = 0; i < 4;i++)
		pthread_join(threads[i], NULL);

	pthread_mutex_destroy(&mutex1);
	printf("The average wait time is %lf.\n", waitTime / 1000000.0);

	return 0;
}

void *serve(void *argv)
{
	int myId = *((int *)argv);

	if (task.size() > 0 && task.front() == myId)
	{
		timeval end, start;
		
		gettimeofday(&end, NULL);
		pthread_mutex_lock(&mutex1);
		start = taskStart.front();
		task.pop();
		taskNum--;
		waitTime += end.tv_sec - start.tv_sec + (double)(end.tv_usec - start.tv_usec);
		pthread_mutex_unlock(&mutex1);

		sleep(myId + 1);
	}
}
