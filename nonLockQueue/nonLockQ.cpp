#include "msgQ.h"
using namespace std;

msgQueue *myQueue;
void *enQueue(void *arg)
{
	for(int i = 0;i < 10;i++)
	{
		myQueue->enQueue(i + 1);
		printf("enQueue %d\n", i + 1);
	}
}

void *deQueue(void *arg)
{
	int val = 1;
	for(int i = 0;i < 10;i++)
	{
		val = -1;
		while(myQueue->deQueue(val) != 0);
		printf("deQueue %d\n", val);
	}
}

int main()
{
	myQueue = new msgQueue();
	pthread_t enT1, enT2, deT1, deT2;

	pthread_create(&enT1, NULL, enQueue, NULL);
	pthread_create(&enT2, NULL, enQueue, NULL);
	pthread_create(&deT1, NULL, deQueue, NULL);
	pthread_create(&deT2, NULL, deQueue, NULL);

	pthread_join(enT1, NULL);
	pthread_join(enT2, NULL);
	pthread_join(deT1, NULL);
	pthread_join(deT2, NULL);


	delete myQueue;
	return 0;
}
