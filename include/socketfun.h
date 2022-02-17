
#ifndef __SOCKET_FUN_H__
#define __SOCKET_FUN_H__

#include <stdint.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>


class Socket
{
public:
    static int SetTOS(int sockfd, unsigned char val);	
    static int SetMaxSeg(int sockfd, int mss);
    static int SetNonBlock(int sockfd, bool nonblock);
    static void SetRecvBufSize(int sockFd, uint32_t aiSize);
    static void SetSendBufSize(int sockFd, uint32_t aiSize);
    static void SetTcpNoDelay(int sockFd, bool on);
    static int SetNoDelay(int sockFd);
    static int Name2Addr(const char*name, in_addr_t* pAddr);
    static int SetSendTimeout(int sockFd, int timeout_s);
    static int SetRecvTimeout(int sockFd, int timeout_s);
    static int GetSendBufSize(int sockFd);
    static int GetRecvBufSize(int sockFd);
    static void MaskPipe();
    static int NewPairSocket(int socket_pair[2]);
    static void CloseFd(int& fd);
};





#endif
