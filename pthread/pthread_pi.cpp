#include <iostream>
#include <pthread.h>
#include <string>
#include <cstdio>
#include <string>
#include <time.h>
using namespace std;

/*
 * Tips: Pi = 1 - 1/3 + 1/5 - 1/7 + 1/9....
 *
 * 1. Shared var need be in global or heap storage.
 *
 */

#define MAXLEN 10000000
#define MAXThreadNum 16
#define BLOCKLEN 100

void *calculate(void *);
int low_bound;
double pi = 0;

pthread_mutex_t mutex1;

int main(int argc, char **argv)
{
	int threadNum;
	time_t start, end;
	pthread_t threads[MAXThreadNum];

	sscanf(argv[1], "-p = %d", &threadNum);
	if (threadNum > MAXThreadNum)
	{
		threadNum = MAXThreadNum;
		cout << "Too many threads" << endl;
	}

	low_bound = 1;
	if (pthread_mutex_init(&mutex1, NULL) != 0)
		perror("create mutex1 error!\n");

	time(&start);

	for(int i = 0;i < threadNum;i++)
		pthread_create(&threads[i], NULL, calculate, NULL); 
	for(int i = 0;i < threadNum;i++)
		pthread_join(threads[i], NULL);

	time(&end);
	pthread_mutex_destroy(&mutex1);

	printf("total time is :%lf seconds\n", difftime(end, start));
	cout << "pi = " << pi * 4 << endl;

	return 0;
}

void *calculate(void *a)
{
	int myLow;

	while(1)
	{
		// Critical section resource need lock
		pthread_mutex_lock(&mutex1);
		myLow = low_bound;
		low_bound += BLOCKLEN;
		pthread_mutex_unlock(&mutex1);

		if (myLow >= MAXLEN)
			return ((void *) NULL);

		for(int i = myLow;i < myLow + BLOCKLEN;i += 4)
			pi += 1.0 / i - 1.0 / (i + 2);
	}

	return ((void *) NULL);
}
