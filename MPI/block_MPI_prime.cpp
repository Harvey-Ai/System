#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#define up_bound 15000000

int primes[10000000];
int localPrime[10000000];
int main(int argc, char **argv)
{
	double start_time, end_time;
	int myRank, pNum;
	int chunkSize;
	MPI_Status status;

	MPI_Init(&argc, &argv);
	start_time = MPI_Wtime();

	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	MPI_Comm_size(MPI_COMM_WORLD, &pNum);


	if (pNum <= 2)
	{
		perror("threadNum can't less than 2\n");
		exit(0);
	}

	int primeNum = 4;
	primes[0] = 2;
	primes[1] = 3;
	primes[2] = 5;
	primes[3] = 7;

	if (myRank == 0)
	{
		int unSearchedLow = 11;
		int unSearchedUp = (unSearchedLow - 1) * (unSearchedLow - 1) ;
		while(unSearchedLow < up_bound)
		{
			if (unSearchedUp > up_bound)
				unSearchedUp = up_bound;
			int splitSize = (unSearchedUp - unSearchedLow + 1) / (pNum - 1);
			int localLow = unSearchedLow;
			int localUp = localLow + splitSize;
			for(int i = 1;i < pNum - 1;i++)
			{
				MPI_Send(&localLow, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				MPI_Send(&localUp, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
				localLow += splitSize;
				localUp += splitSize;
			}
			localUp = unSearchedUp;
			MPI_Send(&localLow, 1, MPI_INT, pNum - 1, 0, MPI_COMM_WORLD);
			MPI_Send(&localUp, 1, MPI_INT, pNum - 1, 0, MPI_COMM_WORLD);

			for(int i = 1;i < pNum;i++)
			{
				int localPrimeNum;
				MPI_Recv(&localPrimeNum, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
				MPI_Recv(localPrime, localPrimeNum, MPI_INT, i, 0, MPI_COMM_WORLD, &status);
				memcpy(&primes[primeNum], localPrime, localPrimeNum * sizeof(int));
				primeNum += localPrimeNum;
			}

			MPI_Bcast(&primeNum, 1, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(primes, primeNum, MPI_INT, 0, MPI_COMM_WORLD);

			unSearchedLow = unSearchedUp + 1;
			unSearchedUp = (unSearchedLow - 1) * (unSearchedLow - 1);
			MPI_Bcast(&unSearchedLow, 1, MPI_INT, 0, MPI_COMM_WORLD);	
		}
		printf("get %d primes\n", primeNum);
		//for(int i = 0;i < primeNum;i++)
	//		printf("%d ", primes[i]);
	//	putchar('\n');
	}
	else
	{
		int unSearchedLow = 10;
		while(unSearchedLow < up_bound)
		{
			int foundPrimeNum = 0;
			int localLow, localUp;
			MPI_Recv(&localLow, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
			MPI_Recv(&localUp, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

		//	printf("%d %d\n", localLow, localUp);
			if (localLow % 2 == 0) localLow++;
			for(int i = localLow;i <= localUp;i += 2)
			{
				int j;
				for(j = 0;j < primeNum;j++)
				{
					if (i % primes[j] == 0)
						break;
				}
				if (j >= primeNum)
					localPrime[foundPrimeNum++] = i;
			}

			MPI_Send(&foundPrimeNum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
			MPI_Send(localPrime, foundPrimeNum, MPI_INT, 0, 0, MPI_COMM_WORLD);
			MPI_Bcast(&primeNum, 1, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(primes, primeNum, MPI_INT, 0, MPI_COMM_WORLD);
			MPI_Bcast(&unSearchedLow, 1, MPI_INT, 0, MPI_COMM_WORLD);
		}
	}
	end_time = MPI_Wtime();
	printf("runtime on processor %d is %lf\n", myRank, end_time - start_time);
	MPI_Finalize();
	return 0;
}
