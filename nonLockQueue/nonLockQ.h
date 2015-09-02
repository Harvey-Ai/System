#include <iostream>
#include <cstdio>
#include <map>
#include <cstdlib>
#include <pthread.h>
using namespace std;

struct listNode
{
	int val;
	listNode *next;

	listNode(int v = 0, listNode *n = NULL)
	{
		val = v, next = n;
	}
};

class msgQueue
{
	private:
		listNode *head, *tail;

	public:
		msgQueue();
		~msgQueue();

		int enQueue(int val);
		int deQueue(int &val);
};

msgQueue::msgQueue()
{
	listNode *dummy = new listNode(-1);
	head = tail = dummy;
}

msgQueue::~msgQueue()
{
	while(head)
	{
		free(head);
		head = head->next;
	}
}

int msgQueue::enQueue(int val)
{
	listNode *newNode = new listNode(val, NULL);
	listNode *tmp = tail;
	//	if (__sync_bool_compare_and_swap(&head, NULL, newNode)
	//		&& __sync_bool_compare_and_swap(&tail, NULL, newNode))
	//		return 0;
	//	if (tmp == NULL)
	//		return -1;

	while(!__sync_bool_compare_and_swap(&(tmp->next), NULL, newNode))
		tmp = tail;

	//	cout << tmp->val << "next:" << val << endl;
	__sync_bool_compare_and_swap(&tail, tmp, newNode);

	return 0;
}

int msgQueue::deQueue(int &val)
{
	//	if (head == NULL)
	//		return -1;

	//	__sync_bool_compare_and_swap(&tail, head, NULL);

	listNode *tmp = head;
	do{
		tmp = head;
		if (tmp->next == NULL)
			return -1;
	}while(!__sync_bool_compare_and_swap(&head, tmp, tmp->next));

	val = tmp->next->val;
	delete tmp;
	return 0;
}
