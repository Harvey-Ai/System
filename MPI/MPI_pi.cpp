#include "mpi.h"
#include <stdio.h> 
#include <time.h> 
int main(int argc,char ** argv )
{   
	int i,rank,size,n=100000; 
	double x,pi,sum=0,step=1.0/n;   
	clock_t t=0;  
	MPI_Init(&argc,&argv); 
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	MPI_Comm_size(MPI_COMM_WORLD,&size);  
	
	t-=clock();    
	printf("%lf\n",t);
	for(i=rank;i<n;i=i+size)
	{      
		x=(i+0.5)*step;  
		sum+=4/(1+x*x);
	}    
	MPI_Reduce(&sum,&pi,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);  
	if(rank==0) 
	{     
		pi=step*pi;
		t+=clock(); 
		printf("PI is %.10f, run time is %10f\n",pi,t/1000.0);  
	}     
	MPI_Finalize();
	return 0;
}
