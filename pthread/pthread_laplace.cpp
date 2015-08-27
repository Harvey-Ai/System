#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>

#define MAXThreadsNum 32
#define MAXN 500
#define N 100
#define ext 0.01

double matrix[MAXN][MAXN];
double temp[MAXN][MAXN];
pthread_mutex_t mutex1, mutex2;
pthread_cond_t cond;
long long flag;
int pthread_num;
double diff;
void *laplaceWork(void *tag);

int main(int argc, char **argv)
{
	pthread_t threads[MAXThreadsNum];
	timeval start, end;
	char *pStr;
	gettimeofday(&start, NULL);
	
	for(int i=1; i<argc; i++) {
		pStr=strstr(argv[i], "-p=");
		if ( pStr==NULL ) continue;
		sscanf(pStr, "-p=%d", &pthread_num);
	}
	if(pthread_num < 2) {
		printf("Please input the number of threads to be created by specifying -p=num_of_threads, where num_of_threads is a integer greater than 1\n");
		return 0;
	}
	for(int i = 1;i <= N;i++)
	{
		for(int j = 1;j <= N;j++)
			matrix[i][j] = (int)(random())%1000;
		matrix[i][0] = matrix[i][N + 1] = 0;
	}
	for(int j = 0;j <= N + 1;j++)
		matrix[0][j] = matrix[N + 1][j] = 0;

	flag = 0;
	if(pthread_num>MAXThreadsNum) pthread_num = MAXThreadsNum;
	if (pthread_mutex_init(&mutex1,NULL)>0) printf("error in initializing mutex1\n");
	if (pthread_mutex_init(&mutex2,NULL)>0) printf("error in initializing mutex2\n");
	pthread_cond_init(&cond, NULL);

//	printf("preare1\n");
	
	int parameter[MAXThreadsNum];
	for(int i = 0;i < pthread_num - 1;i++)
	{
		parameter[i] = i;
		pthread_create(&threads[i], NULL, laplaceWork, (void *)&parameter[i]);
	}

	for(int i = 0;i < pthread_num - 1;i++)
		pthread_join(threads[i], NULL);

	pthread_mutex_destroy(&mutex1);
	pthread_mutex_destroy(&mutex2);
	pthread_cond_destroy(&cond);

	gettimeofday(&end, NULL);
	double tcost = end.tv_sec - start.tv_sec + (double)(end.tv_usec-start.tv_usec)/1000000.0;
	printf("pThread time is %f seconds\n", tcost);

	return 0;
}

void *laplaceWork(void *tag)
{
	int myIndex = *(int *)tag;
	int splitLen = N / (pthread_num - 1);
	int startRow = splitLen * myIndex + 1;
	int endRow = startRow + splitLen - 1;
	int myCircle = 0;
	if (myIndex == pthread_num - 2)
		endRow = N;

	while(1)
	{
		double maxDiff = 0;
		for(int i = startRow;i <= endRow;i++)
			for(int j = 1;j <= N;j++)
			{
				temp[i][j] = (matrix[i][j - 1] + matrix[i][j + 1] + matrix[i - 1][j] + matrix[i + 1][j] + matrix[i][j]) / 5; 
				if (fabs(temp[i][j] - matrix[i][j]) > ext)
					maxDiff = fabs(temp[i][j] - matrix[i][j]);
			}
		
//		printf("okk if %d flag%lld\n", (1 << (pthread_num - 1)) - 1, flag);
		pthread_mutex_lock(&mutex1);
		flag |= 1 << myIndex;
		if (flag != (1 << (pthread_num - 1)) - 1)
			pthread_cond_wait(&cond, &mutex1);
		else
		{
			pthread_cond_broadcast(&cond);
			flag = 0;
			diff = maxDiff;
		}
		if (diff < maxDiff)
			diff = maxDiff;
		pthread_mutex_unlock(&mutex1);

		pthread_mutex_lock(&mutex1);
		flag |= 1 << myIndex;
		if (flag != (1 << (pthread_num - 1)) - 1)
			pthread_cond_wait(&cond, &mutex1);
		else
		{
			pthread_cond_broadcast(&cond);
			flag = 0;
		}
		pthread_mutex_unlock(&mutex1);

		if (diff < ext)
			break;
		for(int i = 1;i < N + 1;i++)
			for(int j = 1;j < N + 1;j++)
				matrix[i][j] = temp[i][j];
	}
	return (void *)NULL;
}
