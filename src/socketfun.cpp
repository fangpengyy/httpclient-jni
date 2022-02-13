#include "socketfun.h"

#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>


int Socket::SetNonBlock(int sockfd, bool nonblock)
{
   int flags = ::fcntl(sockfd, F_GETFL, 0);
   nonblock ? flags |= O_NONBLOCK : flags & ~O_NONBLOCK;
   if (fcntl(sockfd, F_SETFL, flags) == -1)
       return -1;  

#if 0
   flags = ::fcntl(sockfd, F_GETFD, 0);
   flags |= FD_CLOEXEC;
   if (fcntl(sockfd, F_SETFD, flags) == -1)
       return -1;
#endif  
   return 0;
}

void Socket::SetRecvBufSize(int sockFd, uint32_t aiSize)
{
   socklen_t loLen = sizeof(aiSize);
   if (::setsockopt(sockFd, SOL_SOCKET, SO_RCVBUF, &aiSize, loLen) == -1) {
       int size = GetRecvBufSize(sockFd);
       printf("%s error=%s old bufsize=%d\n",__func__, strerror(errno), size);
   }
}

void Socket::SetSendBufSize(int sockFd, uint32_t aiSize)
{
   socklen_t loLen = sizeof(aiSize);
   if (::setsockopt(sockFd, SOL_SOCKET, SO_SNDBUF, &aiSize, loLen) ==-1) {
       int size = GetSendBufSize(sockFd);
       printf("%s error=%s old bufsize=%d\n",__func__, strerror(errno), size);
   }
}


int Socket::GetSendBufSize(int sockFd)
{
   int size = 0;
   socklen_t len = sizeof(size);
   if (getsockopt(sockFd, SOL_SOCKET, SO_SNDBUF, (void*)&size, &len) == -1)
       printf("%s error=%s bufsize=%d\n",__func__, strerror(errno), size);
   return size;
}


int Socket::GetRecvBufSize(int sockFd)
{
   int size = 0;
   socklen_t len = sizeof(size);
   if (getsockopt(sockFd, SOL_SOCKET, SO_RCVBUF, (void*)&size, &len) == -1)
       printf("%s error=%s bufsize=%d\n",__func__, strerror(errno), size);
   return size;
}



void Socket::SetTcpNoDelay(int sockFd, bool on)
{
   int liVal = on ? 1 : 0;
   ::setsockopt(sockFd, IPPROTO_TCP, TCP_NODELAY, &liVal, static_cast<socklen_t>(sizeof liVal));
}

int Socket::SetNoDelay(int sockFd)
{
    const char lchOpt = 1;
    int liRet = setsockopt(sockFd, IPPROTO_TCP, TCP_NODELAY,  &lchOpt,  sizeof(char));
    if(liRet == -1)
        return -1;
    return 0;
}

int Socket::Name2Addr(const char*name, in_addr_t* pAddr)
{
    struct hostent * hp;
    if (isdigit((int)(*name)))
        *pAddr = inet_addr(name);
    else
    {
       hp = gethostbyname(name);
       if (hp == nullptr)
          return -1;
       memcpy((char*)pAddr, hp->h_addr_list[0], hp->h_length);
    }
    return 0;
}

int Socket::SetSendTimeout(int sockFd, int timeout_s)
{
    struct timeval loSndTimeo = {timeout_s, 0};
    if (setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, (char*)&loSndTimeo, sizeof(loSndTimeo)) == -1)
        return -1;
    return 0;
}

int Socket::SetRecvTimeout(int sockFd, int timeout_s)
{
    struct timeval loRcvTimeo = {timeout_s, 0};
    if (setsockopt(sockFd, SOL_SOCKET, SO_RCVTIMEO, &loRcvTimeo, sizeof(loRcvTimeo)) == -1)
        return -1;
    return 0;
}

void Socket::MaskPipe()
{
    sigset_t signal_mask;
    sigemptyset (&signal_mask);
    sigaddset (&signal_mask, SIGPIPE);
    int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
    if (rc != 0)
        printf("block sigpipe error/n");
}


int Socket::NewPairSocket(int socket_pair[2])
{
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, socket_pair) == -1 ) { 
        printf("Error, socketpair create failed, errno(%d): %s\n", errno, strerror(errno));
        return -1; 
    } 
    return 0;
}

void Socket::CloseFd(int& fd)
{
    close(fd);
    fd = -1;
}

