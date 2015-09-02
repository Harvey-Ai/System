#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sys/wait.h>
using namespace std;

const int bufSize = 1024;
int main() {

	int fd[2];
	pipe(fd);

	cout << "########Usage: Input end to stop process.#########" << endl;

	int pid;
	if((pid = fork()) < 0) {
		perror("fork error");
	} else if (pid == 0) {
		
		char buf[bufSize];
		int n;
		while((n = read(fd[0], buf, bufSize)) > 0) {
			buf[n] = '\0';
			cout << "read pipe: " << buf << endl;

			if (strcmp(buf, "end") == 0) {
				break;
			}
		}
	} else {
		char buf[bufSize];
		cout << "write pipe: ";
		while(cin >> buf) {
			write(fd[1], buf, strlen(buf));
			sleep(1);
			if (strcmp(buf, "end") != 0) {
				cout << "write pipe: ";
			} else {
				break;
			}
		}
	}

	int status;
	waitpid(pid, &status, 0);
	close(fd[0]);
	close(fd[1]);
	return 0;
}
