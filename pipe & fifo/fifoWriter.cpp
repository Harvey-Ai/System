#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include "fifoDef.h"
#include <sys/errno.h>
using namespace std;

const int bufSize = 1024;

int main() {
	char buf[bufSize];
	int writerFd;
	int n;

	if (mkfifo(fifoFile, S_IWUSR | S_IRUSR) < 0 && errno != EEXIST) {
		cout << "writer mkfifo error" << endl;
	}
	writerFd = open(fifoFile, O_CREAT | O_WRONLY);
	
	cout << "fifo writer: ";
	cin >> buf;
	while((n = write(writerFd, buf, strlen(buf))) > 0) {
		cout << "fifo writer: ";
		cin >> buf;
		if (strcmp(buf, "end") == 0) {
			break;
		}
	}

	return 0;
}
