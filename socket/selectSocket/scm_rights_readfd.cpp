/*****************************************************
 * Author: Ai Hongwei 
 * Email: ufo008ahw@163.com
 * Last modified: 2015-08-31 02:54
 * Filename: scm_rights_readfd.cpp
 * Description: 
 ****************************************************/

#include <sys/types.h>  
#include <sys/socket.h>  
#include <fcntl.h>  
#include <linux/un.h>  
#include <string.h>  
#include <signal.h>  
#include <stdio.h>  
#include <errno.h>  
#include <unistd.h>  
#include <sys/wait.h>  

#define CONTROLLEN CMSG_LEN(sizeof(int))

int read_fd(int fd)
{
	struct msghdr   msg;
	struct iovec    iov[1];
	ssize_t         n;
	int             newfd;
	char			buffer[2];
	int				recvfd;

// #ifdef  HAVE_MSGHDR_MSG_CONTROL
	struct cmsghdr  *cmptr = NULL;

	msg.msg_control = cmptr;
	msg.msg_controllen = CONTROLLEN;
	/*
#else
	msg.msg_accrights = (caddr_t) &newfd;
	msg.msg_accrightslen = sizeof(int);
#endif
	 */

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	iov[0].iov_base = buffer;
	iov[0].iov_len = sizeof(buffer);
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	if ( (n = recvmsg(fd, &msg, 0)) <= 0)
		return -1;

	// #ifdef  HAVE_MSGHDR_MSG_CONTROL
	if ( (cmptr = CMSG_FIRSTHDR(&msg)) != NULL &&
			cmptr->cmsg_len == CMSG_LEN(sizeof(int))) {
		if (cmptr->cmsg_level != SOL_SOCKET)
			perror("control level != SOL_SOCKET");
		if (cmptr->cmsg_type != SCM_RIGHTS)
			perror("control type != SCM_RIGHTS");
		recvfd = *((int *) CMSG_DATA(cmptr));
	} else
		recvfd = -1;
	/*
#else
	if (msg.msg_accrightslen == sizeof(int))
	recvfd = newfd;
	else
	recvfd = -1;
#endif
	 */

	return recvfd;
}
