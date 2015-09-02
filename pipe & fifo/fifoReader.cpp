#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <sys/stat.h>
#include "fifoDef.h"
#include <sys/errno.h>
using namespace std;

const int bufSize = 1024;

int main() {
	char buf[bufSize];
	int readerFd;
	int n;

	if (mkfifo(fifoFile, S_IRUSR | S_IWUSR) < 0 && errno != EEXIST) {
		cout << "create file error" << endl;
	}

	readerFd = open(fifoFile, O_CREAT | O_RDONLY);
	while((n = read(readerFd, buf, bufSize)) > 0) {
		buf[n] = '\0';
		cout << "fifo Reader: " << buf << endl;
		if (strcmp(buf, "end") == 0) {
			break;
		}
	}
	return 0;
}
