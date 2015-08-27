#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>


/*
 * Learning Tips
 *
 * 1. child process will inheriate all stack from father process.
 *
 */
int main()
{
	int i;
	printf("%d\n", i);
	for(int i = 0;i < 2;i++)
	{
		fork();
		printf("%d\n", i);
		printf("-");
	}
	return 0;
}
