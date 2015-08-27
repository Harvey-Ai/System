/*************************************/
/*作业4 实现第五讲中的Nbody问题并行程序*/
/*************************************/

#include <stdio.h>
#include <math.h>
#include <sys/time.h>
#include <string.h>
#include <pthread.h>
#define MAXThreadsNum 32  //允许启动的最大线程数量


#define myMax(a,b) (((a)>(b))?(a):(b))
#define myMin(a,b) (((a)<(b))?(a):(b))

int BodyNum=0;
int TimeSteps=0; 

double *pBody;//存储粒子的基本信息，每个粒子占用4个连续的浮点数：mass、x、y、z
double *pForce;//存储粒子的受力，每个粒子占用3个连续的浮点数：Fx、Fy、Fz
 

int thread_num=0, progress=0;
void *calcForce(void *);
pthread_mutex_t mutex1, mutex2;
pthread_cond_t cond;
                                                                  
int main(int argc, char** argv )
{
     
  	int n, t, i, threadID[MAXThreadsNum];
  	pthread_t threads[MAXThreadsNum];
  	double run_time;
  	char *pStr;
  	FILE *fResult;
  
  	timeval start, end;
  
  	gettimeofday(&start, NULL);

  	for ( i=1; i<argc; i++ ) {
	  	pStr=strstr(argv[i], "-s=");
	  	if ( pStr!=NULL) sscanf(pStr, "-s=%d", &BodyNum);
	  	pStr=strstr(argv[i], "-t=");
	  	if ( pStr!=NULL) sscanf(pStr, "-t=%d", &TimeSteps);
	  	pStr=strstr(argv[i], "-p=");
	  	if ( pStr!=NULL ) sscanf(pStr, "-p=%d", &thread_num);
  	}

  	if ( BodyNum*TimeSteps*thread_num==0) {
	  	printf("usage: ser_nbody -s=number-of-bodies -t=number-of-steps -p=number-of-threads\n");
	  	return 0;
  	}
  
  	if(thread_num<2) {
	  	printf("Please input the number of threads to be created by specifying -p=num_of_threads, where num_of_threads is a integer greater than 1\n");
	  	return 0;
  	}
  	if(thread_num>MAXThreadsNum) thread_num = MAXThreadsNum;

  	pForce = new double[3*BodyNum];
  	pBody = new double[4*BodyNum];

  	/*  Initialize mass and positions in array p to make a test case
      	Initialize force to 0
  	*/
  	for ( i=0; i<BodyNum; i++){
      		*(pBody+4*i) = 10.05 + i;
      		*(pBody+4*i+1) = 30.0*i;
      		*(pBody+4*i+2) = 20.0*i;
      		*(pBody+4*i+3) = 10.0*i;
      		*(pForce+3*i) = 0;
      		*(pForce+3*i+1) = 0;
      		*(pForce+3*i+2) = 0;
    	}

  	//并行计算
		if (pthread_mutex_init(&mutex1,NULL)>0) printf("error in initializing mutex1\n");
		if (pthread_mutex_init(&mutex2,NULL)>0) printf("error in initializing mutex2\n");
	
	pthread_cond_init(&cond, NULL);
	//创建线程
	for(i=0; i<thread_num; i++) {
		threadID[i] = i;
		pthread_create(&threads[i],NULL, calcForce, &threadID[i]);
	}

	//全部线程结束
	for(i=0; i<thread_num; i++){
		pthread_join(threads[i],NULL);
	} 
		

	pthread_mutex_destroy(&mutex1);
	pthread_mutex_destroy(&mutex2);
	pthread_cond_destroy(&cond);


  	fResult=fopen("result_pthread_nbody2_zyt.txt", "w");

  	char result[50];
  	for (i=0; i<BodyNum; i++)   { 	  
	  	sprintf(result, "(%10.4f %10.4f %10.4f %10.4f)\n", *(pBody+4*i), *(pBody+4*i+1), *(pBody+4*i+2), *(pBody+4*i+3));
	  	fwrite(result, sizeof(char), strlen(result), fResult);
   	}
   	fclose(fResult);

  	delete[] pForce;
  	delete[] pBody;  
  
  	gettimeofday(&end, NULL);
  	run_time = end.tv_sec - start.tv_sec + (double)(end.tv_usec-start.tv_usec)/1000000.0; 
  	printf("runtime is : %f\n", run_time);
	}


void *calcForce(void *arg) {
	int myID = *((int*)arg);
	double fac, fx, fy, fz;
	double dx, dy, dz, sq, dist; 
	int t = 0, i, j,a,b;

  while ( t<TimeSteps){
    /*  Loop over points calculating force between each pair.*/
    
    
    
    	for ( i=myID; i<BodyNum; i+=thread_num ){
        	a=myMin(i+(BodyNum+1)/2,BodyNum);  //计算任务Wia 
        	b=myMax(0,i-(BodyNum+1)/2); //计算任务Wbi 
        	for ( j=i+1; j<a; j++ ) {/*Calculate force between particle i and j according to Newton's Law*/
		     	if ( i==j ) continue;
             		dx = *(pBody+4*i+1) - *(pBody+4*j+1);
             		dy = *(pBody+4*i+2) - *(pBody+4*j+2);
             		dz = *(pBody+4*i+3) - *(pBody+4*j+3);
             		sq = dx*dx + dy*dy + dz*dz;
             		dist = sqrt(sq);
             		fac = (*(pBody+4*i)) * (*(pBody+4*j)) / ( dist * sq );
             		fx = fac * dx;
             		fy = fac * dy;
             		fz = fac * dz;

             		/*Add in force and opposite force to particle i and j */
		      	pthread_mutex_lock(&mutex1);
              		*(pForce+3*i) = *(pForce+3*i) - fx;
              		*(pForce+3*i+1) = *(pForce+3*i+1) - fy;
              		*(pForce+3*i+2) = *(pForce+3*i+2) - fz;
              		*(pForce+3*j) = *(pForce+3*j) + fx;
              		*(pForce+3*j+1) = *(pForce+3*j+1) + fy;
              		*(pForce+3*j+2) = *(pForce+3*j+2) + fz;
 		      	pthread_mutex_unlock(&mutex1);
         	}
         	for ( j=b; j<i; j++ ){/*Calculate force between particle i and j according to Newton's Law*/
		     	if ( i==j ) continue;
             		dx = *(pBody+4*i+1) - *(pBody+4*j+1);
             		dy = *(pBody+4*i+2) - *(pBody+4*j+2);
             		dz = *(pBody+4*i+3) - *(pBody+4*j+3);
             		sq = dx*dx + dy*dy + dz*dz;
             		dist = sqrt(sq);
             		fac = (*(pBody+4*i)) * (*(pBody+4*j)) / ( dist * sq );
             		fx = fac * dx;
             		fy = fac * dy;
             		fz = fac * dz;

             		/*Add in force and opposite force to particle i and j */
		     	pthread_mutex_lock(&mutex1);
              		*(pForce+3*i) = *(pForce+3*i) - fx;
              		*(pForce+3*i+1) = *(pForce+3*i+1) - fy;
              		*(pForce+3*i+2) = *(pForce+3*i+2) - fz;
              		*(pForce+3*j) = *(pForce+3*j) + fx;
              		*(pForce+3*j+1) = *(pForce+3*j+1) + fy;
              		*(pForce+3*j+2) = *(pForce+3*j+2) + fz;
 		      	pthread_mutex_unlock(&mutex1);
         	}
    	}

	//synchronization: to ensure that the computation of force on each body has been completed before updating its state
	pthread_mutex_lock (&mutex2);
	progress++;
	if ( progress!=thread_num){
		pthread_cond_wait( &cond, &mutex2);
	}else {
		pthread_cond_broadcast(&cond);
		progress = 0;
	}
	pthread_mutex_unlock (&mutex2);

    	for ( i=myID; i<BodyNum; i+=thread_num ){ 
	  	*(pBody+4*i+1) = *(pBody+4*i+1) + (*(pForce+3*i)) / (*(pBody+4*i));
	  	*(pForce+3*i) = 0;
	  	*(pBody+4*i+2) = *(pBody+4*i+2) + (*(pForce+3*i+1)) / (*(pBody+4*i));
	  	*(pForce+3*i+1) = 0;
	  	*(pBody+4*i+3) = *(pBody+4*i+3) + (*(pForce+3*i+2)) / (*(pBody+4*i));
	  	*(pForce+3*i+2) = 0;
    	}

	//synchronization: to ensure that each thread has completed the update of body states before the next step simulation
	pthread_mutex_lock (&mutex2);
	progress++;
	if ( progress!=thread_num){
		pthread_cond_wait( &cond, &mutex2);
	}else {
		pthread_cond_broadcast(&cond);
		progress = 0;
	}
	
	pthread_mutex_unlock (&mutex2);
    	t++;
  }

  return((void*)NULL);
}
