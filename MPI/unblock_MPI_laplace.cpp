#include <mpi.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <stdlib.h>

#define N 100
#define ext 0.01
int main(int argc, char **argv)
{
	int myRank;
	int pNum;
	double start_time, end_time;
	double *matrix;
	MPI_Status stat;
	MPI_Request req1[300], req2[300];


	MPI_Init(&argc, &argv);
	start_time = MPI_Wtime();

	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm_size(MPI_COMM_WORLD, &pNum);

	if (myRank == 0)
	{
		double buf[N+5];
		while(1)
		{
			double diff;
			int flag = 0;
			for(int i = 1;i < pNum;i++)
			{
				MPI_Recv(&diff, 1, MPI_DOUBLE, i, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
				if (diff > ext)
					flag = 1;
			}
			MPI_Bcast(&flag, 1, MPI_INT, 0, MPI_COMM_WORLD);
			if (flag == 0)
				break;
		}
	}
	else
	{
		// init calculate model
		int local_size = N / (pNum - 1) + 2;
		if (myRank == pNum - 1)
			local_size = N - (local_size - 2) * (pNum - 2) + 2;
		
		//printf("local size: %d\n", local_size);
		double temp[local_size][N + 2], temp2[local_size][N + 2];
		for(int i = 1;i < local_size - 1;i++)
		{
			for(int j = 1;j < N + 1;j++)
				temp[i][j] = (int)(random())% 1000;
			temp[i][0] = temp[i][N + 1] = 0;
		}	
		for(int j = 0;j < N + 2;j++)
			temp[0][j] = temp[local_size - 1][j] = 0;

		double maxDiff = ext + 1;
		while(1)
		{
			maxDiff = ext;
			// pass value
			int sendNum = 0, recNum = 0;
			if (myRank != 1)
				MPI_Isend(temp[1], N + 2, MPI_DOUBLE, myRank - 1, 0, MPI_COMM_WORLD, &req1[sendNum++]);
			if (myRank != pNum - 1)
				MPI_Isend(temp[local_size - 2], N + 2, MPI_DOUBLE, myRank + 1, 0, MPI_COMM_WORLD, &req1[sendNum++]);
			double preBuf[N], nextBuf[N];
			if (myRank != 1)
			{
				MPI_Irecv(temp[0], N + 2, MPI_DOUBLE, myRank - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &req2[recNum++]);
				//memcpy(temp[0], preBuf, N + 2);
			}
			if (myRank != pNum - 1)
			{
				MPI_Irecv(temp[local_size - 1], N + 2, MPI_DOUBLE, myRank + 1, MPI_ANY_TAG, MPI_COMM_WORLD, &req2[recNum++]);
				//memcpy(temp[local_size - 1], nextBuf, N + 2);
			}
			//calculate
			for(int i = 1;i < local_size - 1;i++)
				for(int j = 1;j <= N;j++)
				{
					temp2[i][j] = (temp[i - 1][j] + temp[i + 1][j] + temp[i][j - 1] + temp[i][j + 1] + temp[i][j]) / 5;  
					if (fabs(temp2[i][j] - temp[i][j]) > maxDiff)
						maxDiff = fabs(temp2[i][j] - temp[i][j]);
				}

			for(int i = 0;i < recNum;i++)
				MPI_Wait(&req2[i], &stat);
//			printf("id:%d diff %lf localSize %d\n", myRank, maxDiff, local_size);
			MPI_Send(&maxDiff, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
			int flag;
			MPI_Bcast(&flag, 1, MPI_INT, 0, MPI_COMM_WORLD);
			for(int i = 1;i < local_size - 1;i++)
				for(int j = 1;j <= N;j++)
					temp[i][j] = temp2[i][j];
			if (flag == 0)
				break;
		//	printf("rank:%d ok diff %lf\n", myRank, maxDiff);
		}
		//for(int j = 1;j < local_size - 1;j++)
		//	MPI_Send(&temp[i][1], N, MPI_DOUBLE, 0, myRank, MPI_COMM_WORLD, &stat);
	}
	end_time = MPI_Wtime();
	printf("rank: %d, runtime is %fs\n", myRank, end_time - start_time);
	MPI_Finalize();
	return 0;
}
