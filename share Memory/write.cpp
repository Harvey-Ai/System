/*****************************************************
 * Author: Ai Hongwei 
 * Email: ufo008ahw@163.com
 * Last modified: 2015-08-25 22:18
 * Filename: write.cpp
 * Description: 
 ****************************************************/

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sys/shm.h>
#include <unistd.h>
#include "define.h"
using namespace std;

extern int shm_key;

void writer() {

	int shm_id;
	shared_buffer *buffer_ptr;

	// get shared memory
	shm_id = shmget(shm_key, sizeof(shared_buffer), 0600 | IPC_CREAT);
	if (shm_id < 0) {
		perror("get shared memory error!\n");
		return; 
	}
	buffer_ptr = (shared_buffer *)shmat(shm_id, (void *)0, 0);
	if(buffer_ptr == (void *)-1)  
	{
		printf("strerror: %s\n", strerror(errno));
		exit(EXIT_FAILURE);  
	}  
	buffer_ptr->bufferState = state_write;	
	// message pass
	while(1) {
		while(buffer_ptr->bufferState == state_read) {
			sleep(1);
		}
		cout << "write: ";
		cin >> buffer_ptr->buffer;
		
		buffer_ptr->bufferState = state_read;
		if (strcmp(buffer_ptr->buffer, "end") == 0) {
			break;
		}
	}
	
	// detach shared memory
	shmdt(buffer_ptr);
	while(buffer_ptr->bufferState != state_idle) {
		printf("wait read thread\n");
		sleep(1000);
	}

	shmctl(shm_id, IPC_RMID, NULL);
}

