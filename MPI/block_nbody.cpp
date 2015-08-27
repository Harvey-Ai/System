/*****************************************************/
/*作业7,利用MPI阻塞式通信实现第五讲中的Nbody问题并行程序*/
/******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

int BodyNum=0;
int TimeSteps=0;

int main(int argc, char** argv )
{  	
	char *pStr;	
	int n, t, i, j, k, aveNum, extraNum, locNum,source, dest, count,proc, myRank;
  	double *pLocBody, *pRmtBody,*pForce;
  	double fac, fx, fy, fz;
  	double dx, dy, dz, sq, dist;
	double startTime, endTime, runTime;
  	
  	FILE *fResult;

  	MPI_Status status;
  	
  	MPI_Init(&argc, &argv);
	startTime = MPI_Wtime();
  	MPI_Comm_size(MPI_COMM_WORLD, &proc);
  	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);

  	if(myRank==0) {
          	for(i=1; i<argc; i++) {
                  	pStr=strstr(argv[i], "-s=");
                  	if ( pStr!=NULL) sscanf(pStr, "-s=%d", &BodyNum);
                  	pStr=strstr(argv[i], "-t=");
                  	if(pStr!=NULL) sscanf(pStr, "-t=%d", &TimeSteps);
          	}

          	for(i=1;i<proc; i++){
                  	MPI_Send(&BodyNum, 1, MPI_INT, i, 10, MPI_COMM_WORLD);
                 	MPI_Send(&TimeSteps, 1, MPI_INT, i, 20, MPI_COMM_WORLD);
          	}

          	if(BodyNum*TimeSteps==0) {
                  	printf("usage: ser_nbody -s=number-of-bodies -t=number-of-steps\n");
                  	MPI_Finalize();
                  	return 0;
          	}
  	}else {
          	MPI_Recv(&BodyNum, 1, MPI_INT, 0, 10, MPI_COMM_WORLD, &status);
          	MPI_Recv(&TimeSteps, 1, MPI_INT, 0, 20, MPI_COMM_WORLD, &status);
          	if (BodyNum*TimeSteps==0){
                  	MPI_Finalize();
                  	return 0;
          	}
  	}

  	aveNum = BodyNum/proc;
  	extraNum = BodyNum%proc;
  	if(myRank<extraNum){
		 locNum = aveNum+1;
	}else{
		 locNum = aveNum;	
	}

	pLocBody = (double *)malloc(locNum * 4 * sizeof(double));
	pRmtBody = (double *)malloc((aveNum + 1) * 4 * sizeof(double));
	pForce = (double *)malloc(locNum * 3 * sizeof(double));

	//初始化
 	if (myRank == 0)  {
         	i=0;
         	for ( j=0; j<locNum; j++)  {
                 	*(pLocBody+4*j) = 10.05 + i;
                 	*(pLocBody+4*j+1) = 30.0*i;
                 	*(pLocBody+4*j+2) = 20.0*i;
                 	*(pLocBody+4*j+3) = 10.0*i;
                 	i++;
         	}
         	for (k=1; k<proc;k++){
                 	if(k<extraNum) j = aveNum+1;
                 	else j = aveNum;
                 	for ( t=0; t<j; t++) {
                         	*(pRmtBody+4*t) = 10.05 + i;
                         	*(pRmtBody+4*t+1) = 30.0*i;
                         	*(pRmtBody+4*t+2) = 20.0*i;
                         	*(pRmtBody+4*t+3) = 10.0*i;
                         	i++;
                 	}
                 	MPI_Send(pRmtBody, j*4, MPI_DOUBLE, k, j, MPI_COMM_WORLD);
         	}
 	}else {
          	MPI_Recv(pLocBody, 4*(aveNum+1), MPI_DOUBLE, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
 	}

  	t = 0;
  	while(t<TimeSteps){
          	for(i=0; i<3*locNum; i++)  *(pForce+i) = 0;

          	for(i=0; i<locNum; i++ )
                  	for(j=0; j<locNum; j++ ) {
                                if(i==j) continue;
                                dx = *(pLocBody+4*i+1) - *(pLocBody+4*j+1);
                                dy = *(pLocBody+4*i+2) - *(pLocBody+4*j+2);
                                dz = *(pLocBody+4*i+3) - *(pLocBody+4*j+3);
                                sq = dx*dx + dy*dy + dz*dz;
                                dist = sqrt(sq);
                                fac = (*(pLocBody+4*i)) * (*(pLocBody+4*j)) / ( dist * sq );
                                fx = fac * dx;
                                fy = fac * dy;
                                fz = fac * dz;

                                *(pForce+3*i) = *(pForce+3*i) - fx;
                                *(pForce+3*i+1) = *(pForce+3*i+1) - fy;
                                *(pForce+3*i+2) = *(pForce+3*i+2) - fz;
                        }

                for ( k=1; k<proc; k++) {
                        source = (myRank - k + proc) % proc;
                        dest = (myRank + k)% proc;
                        MPI_Send(pLocBody, 4*locNum, MPI_DOUBLE, dest, locNum, MPI_COMM_WORLD);
                        MPI_Recv(pRmtBody, 4*(aveNum+1),  MPI_DOUBLE, source, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

                        count = status.MPI_TAG;
                        for ( i=0; i<locNum; i++ )
                                for ( j=0; j<count; j++ ) {
                                        dx = *(pLocBody+4*i+1) - *(pRmtBody+4*j+1);
                                        dy = *(pLocBody+4*i+2) - *(pRmtBody+4*j+2);
                                        dz = *(pLocBody+4*i+3) - *(pRmtBody+4*j+3);
                                        sq = dx*dx + dy*dy + dz*dz;
                                        dist = sqrt(sq);
                                        fac = (*(pLocBody+4*i)) * (*(pRmtBody+4*j)) / ( dist * sq );
                                        fx = fac * dx;
                                        fy = fac * dy;
                                        fz = fac * dz;
                                        *(pForce+3*i) = *(pForce+3*i) - fx;
                                        *(pForce+3*i+1) = *(pForce+3*i+1) - fy;
                                        *(pForce+3*i+2) = *(pForce+3*i+2) - fz;
                                }
                }
                for ( i=0; i<locNum; i++ ) {
                        *(pLocBody+4*i+1) = *(pLocBody+4*i+1) + (*(pForce+3*i)) / (*(pLocBody+4*i));
                        *(pLocBody+4*i+2) = *(pLocBody+4*i+2) + (*(pForce+3*i+1)) / (*(pLocBody+4*i));
                        *(pLocBody+4*i+3) = *(pLocBody+4*i+3) + (*(pForce+3*i+2)) / (*(pLocBody+4*i));
                }
                t++;
  	}

  	if(myRank==0) {
           	fResult=fopen("result_mpi_nbody.txt", "w");
           	char result[50];
           	for (i=0; i<locNum; i++)  {
                   	sprintf(result, "(%10.4f %10.4f %10.4f %10.4f)\n", *(pLocBody+4*i), *(pLocBody+4*i+1),
					 *(pLocBody+4*i+2), *(pLocBody+4*i+3));
                   	fwrite(result, sizeof(char), strlen(result), fResult);
           	}
           	for(j=1; j<proc; j++) {
                   	MPI_Recv(pRmtBody, 4*(aveNum+1), MPI_DOUBLE, j, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                   	count = status.MPI_TAG;
                   	for (i=0; i<count; i++)  {
                           	sprintf(result, "(%10.4f %10.4f %10.4f %10.4f)\n", *(pRmtBody+4*i), 
					*(pRmtBody+4*i+1), *(pRmtBody+4*i+2), *(pRmtBody+4*i+3));
                           	fwrite(result, sizeof(char), strlen(result), fResult);
                   	}
          	}
           	fclose(fResult);

  	}else{
		 MPI_Send(pLocBody, 4*locNum, MPI_DOUBLE, 0, locNum, MPI_COMM_WORLD);
	}

	free(pLocBody);
	pLocBody=NULL;
	free(pForce);
	pForce=NULL;
	free(pRmtBody);
	pRmtBody=NULL;

  	endTime = MPI_Wtime();
  	MPI_Finalize();
  	runTime = endTime - startTime;
 	printf("runtime is : %f\n", runTime);

	return 0;
}
