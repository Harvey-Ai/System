/*****************************************************
 * Author: Ai Hongwei 
 * Email: ufo008ahw@163.com
 * Last modified: 2015-08-25 22:00
 * Filename: define.h
 * Description: 
 ****************************************************/

extern int shm_key;

const int bufferSize = 1024;
enum state {state_write, state_read, state_idle};

struct  shared_buffer {
	state bufferState;
	char buffer[bufferSize];
};

