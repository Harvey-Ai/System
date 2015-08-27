#include <iostream>
#include <cstdio>
#include <pthread.h>
#include <sys/file.h>
using namespace std;

pthread_mutex_t myMutex[5];
int FileThread[5];
FILE *fileList[5];
const int workIt = 10000;

void* fun1(void *);
void* fun2(void *);
void* fun3(void *);
void* fun4(void *);

int main()
{
	fileList[1] = fopen("1.txt", "w");
	fileList[2] = fopen("2.txt", "w");
	fileList[3] = fopen("3.txt", "w");
	fileList[4] = fopen("4.txt", "w");

	FileThread[1] = 1;
	FileThread[2] = 2;
	FileThread[3] = 3;
	FileThread[4] = 4;

	pthread_t myThread[4];
	pthread_mutex_init(&myMutex[1], NULL);
	pthread_mutex_init(&myMutex[2], NULL);
	pthread_mutex_init(&myMutex[3], NULL);
	pthread_mutex_init(&myMutex[4], NULL);

	pthread_create(&myThread[0], NULL, fun1, NULL);
	pthread_create(&myThread[1], NULL, fun2, NULL);
	pthread_create(&myThread[2], NULL, fun3, NULL);
	pthread_create(&myThread[3], NULL, fun4, NULL);

	for(int i = 0;i < 4;i++)
		pthread_join(myThread[i], NULL);

	pthread_mutex_destroy(&myMutex[1]);
	pthread_mutex_destroy(&myMutex[2]);
	pthread_mutex_destroy(&myMutex[3]);
	pthread_mutex_destroy(&myMutex[4]);

	return 0;
}

void* fun1(void* arg)
{
	int itNum = 0;
	for(int i = 1;i <= 4 && itNum < workIt;i = (i + 1) % 5)
	{
		if (i == 0)
			i = 1;
		pthread_mutex_lock(&myMutex[i]);
		if (FileThread[i] == 1)
		{
			FileThread[i] = 2;
			fprintf(fileList[i], "1");
			itNum++;
		}
		pthread_mutex_unlock(&myMutex[i]);
	}
}

void* fun2(void* arg)
{
	int itNum = 0;
	for(int i = 1;i <= 4 && itNum < workIt;i = (i + 1) % 5)
	{
		if (i == 0)
			i = 1;
		pthread_mutex_lock(&myMutex[i]);
		if (FileThread[i] == 2)
		{
			FileThread[i] = 3;
			fprintf(fileList[i], "2");
			itNum++;
		}
		pthread_mutex_unlock(&myMutex[i]);
	}
}

void* fun3(void* arg)
{
	int itNum = 0;
	for(int i = 1;i <= 4 && itNum < workIt;i = (i + 1) % 5)
	{
		if (i == 0)
			i = 1;
		pthread_mutex_lock(&myMutex[i]);
		if (FileThread[i] == 3)
		{
			FileThread[i] = 4;
			fprintf(fileList[i], "3");
			itNum++;
		}
		pthread_mutex_unlock(&myMutex[i]);
	}
}

void* fun4(void* arg)
{
	int itNum = 0;
	for(int i = 1;i <= 4 && itNum < workIt;i = (i + 1) % 5)
	{
		if (i == 0)
			i = 1;
		pthread_mutex_lock(&myMutex[i]);
		if (FileThread[i] == 4)
		{
			FileThread[i] = 1;
			fprintf(fileList[i], "4");
			itNum++;
		}
		pthread_mutex_unlock(&myMutex[i]);
	}
}
