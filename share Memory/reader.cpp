/*****************************************************
 * Author: Ai Hongwei 
 * Email: ufo008ahw@163.com
 * Last modified: 2015-08-25 21:57
 * Filename: a.cpp
 * Description: share memory clinet
 ****************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sys/shm.h>
#include <unistd.h>
#include "define.h"
using namespace std;

extern int shm_key;

void reader() {
	int shm_id;
	shared_buffer *buffer_ptr;
	
	// get shared memory
	shm_id = shmget(shm_key, sizeof(shared_buffer), 0400 | IPC_CREAT);
	if (shm_id < 0) {
		perror("get shared memory error!\n");
		return;
	}
	buffer_ptr = (shared_buffer *)shmat(shm_id, (void *)0, 0);


	// message pass
	while(1) {
		while(buffer_ptr->bufferState == state_write) {
			sleep(1);
		}
		
		if (strcmp(buffer_ptr->buffer, "end") == 0) {
			break;
		} else 
			cout << "read: " << buffer_ptr->buffer << endl;
		
		buffer_ptr->bufferState = state_write;
	}
	buffer_ptr->bufferState = state_idle;
	
	// detach shared memory
	shmdt(buffer_ptr);
}
