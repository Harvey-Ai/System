#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>

#define MAXLEN 1000
int main(int argc, char **argv)
{
	int myRank;
	int p;
	double start_time, end_time;
	double result = 0;
	int localSize;

	gettimeofday(&startTime, NULL);
	MPI_Init(&argc, &argv);

	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
	MPI_Comm_size(MPI_COMM_WORLD,&p);

	localSize = (MAXLEN + p - 1) / p;
	if (myRank == p - 1)
		localSize = MAXLEN - (p - 1) * localSize;

	int start = localSize * myRank;
	for(int i = 0; i < localSize;i++)
	{
		int temp = i % 2;
		if (temp)
		{
			if ((i + 1)% 2)
				result += 1.0 / i;
			else
				result -= 1.0 / i;
		}
	}
	MPI_Finalize();
	gettimeofday(&endTime, NULL);

	printf("total time is %lf\n", end.tv_sec - start.tv_sec + (double)(end.tv_usec-start.tv_usec) / 10000000.0);

	printf("pi = %lf\n", result * 4);
	return 0;
}
