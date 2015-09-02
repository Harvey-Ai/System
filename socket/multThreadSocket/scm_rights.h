#ifndef SCM_RIGHTS_H_
#define  SCM_RIGHTS_H_

// fd: unix socket, sendfd: file desc
// return: sendmsg()
int write_fd(int fd, int sendfd);

// fd: unix socket
// return file desc
int read_fd(int fd);

#endif
