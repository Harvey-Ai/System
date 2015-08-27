#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

int main(int argc, char** argv)
{
  int  myRank, p,i,j;
  char *pStr;
  int  size,loc_size;
  double sum = 0, pi = 0;
  MPI_Status status;
  double start_time, end_time, *pC, *C, temp;
  timeval start, end;

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
	//  if (size<=0 || p<=1 ) printf("usage: mpirun -np number-of-processor mpi_ma size=matrixe-size\n");
  }
  else
	  MPI_Recv(&size, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
  
 // if (size<=0 || p<=1)   return 0;

  int stepSize = loc_size=(int)(size+p-1)/p;
  if(myRank==p-1)
    loc_size=size-myRank*loc_size;
  
  int startNum = myRank * stepSize;
  for(int i = 0; i < loc_size;i++)
   {
	if ((startNum + i) & 0x01)
	{
	    if (((startNum + i) / 2) & 0x01)
                sum -= 1.0 / (startNum + i);
           else
		   sum += 1.0 / (startNum + i);
         }
   }
  MPI_Reduce(&sum,&pi,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  if (myRank == 0)
  {
    printf("pi is %lf\n", pi * 4);
  }

  end_time=MPI_Wtime();
  printf(" runtime on processor %d  is %f seconds\n", myRank,  end_time-start_time);
  MPI_Finalize();
  return 0;
}

 runtime on processor 1  is 0.000074 seconds
 runtime on processor 5  is 0.000079 seconds
 runtime on processor 3  is 0.000090 seconds
 runtime on processor 2  is 0.001589 seconds
 runtime on processor 7  is 0.000079 seconds
 runtime on processor 8  is 0.000073 seconds
 runtime on processor 6  is 0.000092 seconds
 runtime on processor 4  is 0.003733 seconds
pi is 3.141573
 runtime on processor 0  is 0.005533 seconds






  
