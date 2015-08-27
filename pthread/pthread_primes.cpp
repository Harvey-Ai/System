#include <stdio.h>
#include <string.h>
#include <time.h>

#define maximumPrimes 1200000

int n =15000000;     // 小于N的素数个数；
int primes[maximumPrimes];   //从primes[0] – primes[number-1] 中存放生成的素数；

int main()
{
	int  i, j, k; 
	int usedPrime = 2;
	FILE *fResult;
	timeval start, end;
	double tcost;

	fResult=fopen("result_ser_prime.txt", "rw");

	gettimeofday(&start, NULL);
	primes[0] = 2;
	for(i=3, j=1; i<n; i++){  //从整数3开始检查 i 是否为素数
		for(k=0; primes[k]*primes[k]<i; k++)  // 依次检查 i 是否可以被前面的素数整除
			if( i % primes[k] == 0) break;
		if(primes[k]*primes[k]>i){ // 如果 i 不能被前面的素数整除，则将它作为新素数存入数组
			primes[j] = i;
			if(primes[k]>usedPrime) usedPrime = primes[k];
			j++;
		}
	}
	gettimeofday(&end, NULL);

	printf("writing found primes into result_ser_prime.txt\n");
	char result[20];
	tcost = end.tv_sec - start.tv_sec + (double)(end.tv_usec-start.tv_usec)/1000000.0;
	printf("\n serial time is %f seconds, found %d primes, the last used primes is %d\n", tcost, j, usedPrime);

	return(0);
}



