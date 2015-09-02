/*****************************************************
 * Author: Ai Hongwei 
 * Email: ufo008ahw@163.com
 * Last modified: 2015-08-31 01:34
 * Filename: scm_rights_writefd.cpp
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
#include <stdlib.h>
#include <sys/wait.h> 

#define CONTROLLEN CMSG_LEN(sizeof(int))

int
write_fd(int fd, int sendfd)
{
	// auxiliary data
    struct msghdr msg;
	// data
    struct iovec iov[1];
	char buffer[2] = "1";
// #ifdef  HAVE_MSGHDR_MSG_CONTROL
	 union {
		struct cmsghdr cm;
		char data[CMSG_SPACE(sizeof(int))];
	} cmsg;


	/*
	cmsghdr *ptr = (cmsghdr *) malloc(CONTROLLEN);
	msg.msg_control = ptr;
    msg.msg_controllen = CONTROLLEN;

    ptr->cmsg_len = CMSG_LEN(sizeof(int));
    ptr->cmsg_level = SOL_SOCKET;
    ptr->cmsg_type = SCM_RIGHTS;
    *((int *) CMSG_DATA(ptr)) = sendfd;
	*/


    msg.msg_control = (caddr_t) &cmsg;
    msg.msg_controllen = CONTROLLEN;

    cmsg.cm.cmsg_len = CMSG_LEN(sizeof(int));
    cmsg.cm.cmsg_level = SOL_SOCKET;
    cmsg.cm.cmsg_type = SCM_RIGHTS;
    *((int *) CMSG_DATA(&(cmsg.cm))) = sendfd;
/*#else
    msg.msg_accrights = (caddr_t) &sendfd;
    msg.msg_accrightslen = sizeof(int);
#endif
*/
    msg.msg_name = NULL;
    msg.msg_namelen = 0;

    iov[0].iov_base = buffer;
    iov[0].iov_len = sizeof(buffer);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;

	perror("sending socket");
    int temp = sendmsg(fd, &msg, MSG_DONTWAIT);
	perror("sended socket");

	return temp;
}
