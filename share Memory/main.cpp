/*****************************************************
 * Author: Ai Hongwei 
 * Email: ufo008ahw@163.com
 * Last modified: 2015-08-25 22:23
 * Filename: main.cpp
 * Description: 
 ****************************************************/

#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
using namespace std;



extern void reader();
extern void writer();

int shm_key;
int main() {

	pid_t reader_pid = 0, writer_pid = 0;
	shm_key = 12323145;
	//shm_key = ftok("./a", 10);

	if ((writer_pid = fork()) < 0) {
		perror("wirte pid fork error\n");
	} else if (writer_pid == 0) {
		writer();
	}


	if (writer_pid > 0) {
		reader_pid = fork();
		if (reader_pid == 0) {
			reader();
		}
	}

	if (writer_pid > 0 && reader_pid > 0) {
		int pNum = 2;
		while(pNum--) {
			int status;
			pid_t p = wait(&status);
			cout << (p == writer_pid ? "writer" : "reader") << " finished "<< endl;
		}
		cout << "finished !" << endl;
	}

	return 0;
}
