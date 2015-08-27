#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "mpi.h"

int main(int argc, char *argv[])
{
  int  myRank, p,i,j;
  char *pStr;
  int  size,loc_size;
  double *pA, *pB, *A, *B;
  MPI_Status status;
  double start_time, end_time, *pC, *C, temp;
	
  MPI_Init(&argc, &argv);
  start_time=MPI_Wtime();

  MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
  MPI_Comm_size(MPI_COMM_WORLD,&p);


  if ( myRank==0 ) {
	  size = 0;
	  for ( i=1; i<argc; i++ ) {
		  pStr=strstr(argv[i], "size=");
		  if ( pStr==NULL) continue;
		  sscanf(pStr, "size=%d", &size);
	  }
	  for (i=1; i<p; i++)
		  MPI_Send(&size, 1, MPI_INT, i, 10, MPI_COMM_WORLD);
	  if (size<=0 || p<=1 ) printf("usage: mpirun -np number-of-processor mpi_ma size=matrixe-size\n");
  }
  else
	  MPI_Recv(&size, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
  
  if (size<=0 || p<=1)   return 0;
  loc_size=(int)(size+p-1)/p;
  if(myRank==p-1)
    loc_size=size-myRank*loc_size;


  pA= new double[size*loc_size];
  pB= new double[size*loc_size];
  pC= new double[size*loc_size];
  A=pA;B=pB;C=pC;
  for (i=0; i<size; i++)
   for (j=0; j<loc_size; j++)
    {
     temp = (double) rand()*rand();
     *A = sqrt(temp);
     temp = (double) rand()*rand();
     *B = sqrt(temp);
     *C = (*A)+(*B);
     A++; B++;  C++;
    }
  delete[] pA;
  delete[] pB;
  delete[] pC;

  end_time=MPI_Wtime();
  printf(" runtime on processor %d  is %f seconds\n", myRank,  end_time-start_time);
  MPI_Finalize();
  return 0;
}
