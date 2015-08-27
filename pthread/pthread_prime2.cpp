/******************************/
/*
作业4 实现第五讲中的求素数问题并行程序
2012.11.11
*/
/******************************/

#include <stdio.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <sys/time.h>

#define maximumPrimes 1200000
#define chunkSize 20000
#define MAXThreadsNum 32
#define baseScope 80


void *searchPrimes(void *);  
int primes[maximumPrimes];  
int totalPrimes;      
int PtotalPrimes;   
int thread_num;
int maxPrime,PmaxPrime;
long unsearchedScope_low_bound=0;
long unsearchedScope_up_bound=0; 
int search_scope_up_bound=15000000;  
int progress=0;
int chunkNo=0;
pthread_mutex_t mutex1,mutex2;
pthread_cond_t cond;

int main(int argc,char **argv)
{
 
  FILE *fResult;
  pthread_t threads[MAXThreadsNum];
  double tcost;
  timeval start,end;
  char result[20], *pStr;
  int i,upBound,k;

  gettimeofday(&start,NULL);
  for(i=1;i<argc;i++){
    	pStr=strstr(argv[i],"-p=");
    	if(pStr==NULL) continue;
    	sscanf(pStr,"-p=%d",&thread_num);
  }
  if(thread_num<2) {
    	printf("Please input the number of threads to be created by specifying -p=num_of_threads, where num_of_threads is a integer greater than 1\n");
    	return 0;
  }


  if(thread_num>MAXThreadsNum) thread_num = MAXThreadsNum;

  totalPrimes = 1;
  primes[0] = 2;
  upBound = baseScope;
  if(upBound > search_scope_up_bound) upBound = search_scope_up_bound;

  for(i=3; i<upBound+1; i+=2){  
     for(k=0; primes[k]*primes[k]<i; k++)         if( i % primes[k] == 0) break;
     if(primes[k]*primes[k]>i){ 
	primes[totalPrimes] = i;
	totalPrimes++;
     }
  }
  unsearchedScope_low_bound=upBound+1;
  maxPrime=PmaxPrime=primes[totalPrimes-1];
  unsearchedScope_up_bound=PmaxPrime*PmaxPrime;
  PtotalPrimes=totalPrimes;
  chunkNo=(unsearchedScope_up_bound+chunkSize-1)/chunkSize;

  if(pthread_mutex_init(&mutex1,NULL)>0) printf("error in initializing mutex1\n");
  if(pthread_mutex_init(&mutex2,NULL)>0) printf("error in initializing mutex2\n");
  pthread_cond_init(&cond, NULL);

  unsearchedScope_low_bound=baseScope+1;

    for(i=0; i<thread_num; i++) 
    pthread_create(&threads[i],NULL,searchPrimes,&i);


   for(i=0; i<thread_num; i++)
    pthread_join(threads[i],NULL);

  pthread_mutex_destroy(&mutex1);
  pthread_mutex_destroy(&mutex2);
  pthread_cond_destroy(&cond);

   gettimeofday(&end, NULL);

   fResult=fopen("result_pThread_prime2_zyt.txt", "wr");
  for (i=0; i<totalPrimes; i++){
    if (i%10 == 0 ) fputc('\n', fResult);
    sprintf(result, "%d   ", primes[i]);
    fwrite(result, sizeof(char), strlen(result), fResult);
  }
  fclose(fResult);

    tcost = end.tv_sec - start.tv_sec + (double)(end.tv_usec-start.tv_usec)/1000000.0;
  printf("pThread time is %f seconds, found %d primes and the results are saved in result_pThread_prime2_zyt.txt\n\n", tcost, totalPrimes);

  return(0);
}

void *searchPrimes(void *arg)
{
 
  int locTotal;
  int i,k;
  long loc_low_bound,loc_up_bound;
  int locPrimes[maximumPrimes];
  int threadid=*((int *)arg);
 
  while(1){
  L1:
       pthread_mutex_lock(&mutex2);
    loc_low_bound=unsearchedScope_low_bound;
    unsearchedScope_low_bound=unsearchedScope_low_bound+chunkSize;
    loc_up_bound=loc_low_bound+chunkSize;
    if( loc_up_bound>unsearchedScope_up_bound)
      loc_up_bound=unsearchedScope_up_bound;
    if( unsearchedScope_low_bound > unsearchedScope_up_bound)
      unsearchedScope_low_bound=unsearchedScope_up_bound+2;
    if( loc_low_bound>unsearchedScope_up_bound){
      if(unsearchedScope_up_bound<search_scope_up_bound){
	pthread_cond_wait(&cond,&mutex2);
	loc_low_bound=0;
      }
      else
	loc_low_bound=-1;
    }
    pthread_mutex_unlock(&mutex2);

    if( loc_low_bound==0)
      goto L1;
    if(loc_low_bound==-1)
      break;
    locTotal=0;
    
   
    for(i=loc_low_bound;i<loc_up_bound;i+=2){
      for(k=0;k<PtotalPrimes;k++)
	if(i%primes[k] ==0)
	  break;
      if(k>=PtotalPrimes){
	locPrimes[locTotal]=i;
	locTotal++;
      }
    }

        pthread_mutex_lock(&mutex1);
    for(i=0;i<locTotal;i++){
      primes[totalPrimes]=locPrimes[i];
      totalPrimes++;
    }
    if(maxPrime<locPrimes[locTotal-1])
      maxPrime=locPrimes[locTotal-1];
    pthread_mutex_unlock(&mutex1);
    
        pthread_mutex_lock(&mutex2);
    progress++;
    if (progress==chunkNo)
    {
      unsearchedScope_low_bound=unsearchedScope_up_bound+2;
      PmaxPrime=maxPrime;
      PtotalPrimes=totalPrimes;
      progress=0;
      if(search_scope_up_bound/maxPrime<maxPrime)
	unsearchedScope_up_bound=search_scope_up_bound;
      else
	unsearchedScope_up_bound=maxPrime*maxPrime;
      chunkNo=(unsearchedScope_up_bound-unsearchedScope_low_bound+chunkSize-1)/chunkSize;
      pthread_cond_broadcast(&cond);
    }
    pthread_mutex_unlock(&mutex2); 
}
  //结束
  return((void*)NULL);
}
